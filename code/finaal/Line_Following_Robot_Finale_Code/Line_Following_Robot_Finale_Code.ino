#include "arduino_secrets.h"
#include "thingProperties.h"
#include <Arduino.h>
#include "SerialCommand.h"
#include "EEPROMAnything.h"

#define SerialPort Serial
#define Baudrate 115200
#define EEPROM_SIZE 512

// Motor pinnen
#define MotorLeftForward 5
#define MotorLeftBackward 4
#define MotorRightForward 2
#define MotorRightBackward 3

// Sensor pinnen
const int sensor[] = {A7, A6, A5, A4, A3, A2, A1, A0};

// Gebruik de ingebouwde LED
#define LED_PIN LED_BUILTIN

SerialCommand sCmd(SerialPort);
bool debugMode;
bool run = false;
unsigned long previous, calculationTime;

// Struct met parameters (wordt in EEPROM opgeslagen)
struct param_t {
  unsigned long cycleTime;
  /* andere parameters die in het eeprom geheugen moeten opgeslagen worden voeg je hier toe ... */
  int black[8];
  int white[8];
  int power;
  float diff;
  float kp;
} params;

int normalised[8];
float debugPosition;

void onUnknownCommand(char *command);
void onSet();
void onDebug();
void onCalibrate();
void onRun();

// ======================================================
// ================   SETUP   ===========================
// ======================================================
void setup() {
  SerialPort.begin(Baudrate);
  EEPROM.begin(EEPROM_SIZE);

  // IoT Cloud initialisatie
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(LED_PIN, OUTPUT);
  pinMode(MotorLeftForward, OUTPUT);
  pinMode(MotorLeftBackward, OUTPUT);
  pinMode(MotorRightForward, OUTPUT);
  pinMode(MotorRightBackward, OUTPUT);

  // Commands initialiseren
  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.addCommand("run", onRun);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);

  SerialPort.println("ready");
}

// ======================================================
// ================   MAIN LOOP   =======================
// ======================================================
void loop() {
  ArduinoCloud.update();
  sCmd.readSerial();

  if (startStop) {
    run = true;
    digitalWrite(LED_PIN, HIGH);
    if (!led) led = true;  
  } else {
    run = false;
    digitalWrite(LED_PIN, LOW);
    if (led) led = false;  
    stopMotors();
  }

  unsigned long current = micros();
  if (run && (current - previous >= params.cycleTime)) {
    previous = current;

    // measure & normalize
    for (int i = 0; i < 8; i++) normalised[i] = map(analogRead(sensor[i]), params.black[i], params.white[i], 0, 4096);

    // interpolate
    float position = 0;
    int index = 0;
    for (int i = 1; i < 8; i++) if (normalised[i] < normalised[index]) index = i;

    if (normalised[index] > 3000) {
      run = false;
      startStop = false;   // Cloud switch uitzetten
      led = false;         // Cloud LED uitzetten
      digitalWrite(LED_PIN, LOW);
      stopMotors();
      SerialPort.println("Lijn verloren! Robot automatisch gestopt.");
      return; // verlaat de loop vroegtijdig
    }

    if (index == 0) position = -50;
    else if (index == 7) position = 50;
    else {
      int sNul = normalised[index];
      int sMinEen = normalised[index - 1];
      int sPlusEen = normalised[index + 1];

      float b = sPlusEen - sMinEen;
      b = b / 2;
      float a = sPlusEen - b - sNul;

      position = -b / (2 * a);
      position += index;
      position -= 3.5;
      position *= 15;
    }
    debugPosition = position;

    float error = -position;
    float output = error * params.kp;
    output = constrain(output, -510, 510);

    int powerLeft = 0;
    int powerRight = 0;

    if (output >= 0) {
      powerLeft = constrain(params.power + params.diff * output, -255, 255);
      powerRight = constrain(powerLeft - output, -255, 255);
      powerLeft = powerRight + output;
    } else {
      powerRight = constrain(params.power - params.diff * output, -255, 255);
      powerLeft = constrain(powerRight + output, -255, 255);
      powerRight = powerLeft - output;
    }

    analogWrite(MotorLeftForward, powerLeft > 0 ? powerLeft : 0);
    analogWrite(MotorLeftBackward, powerLeft < 0 ? -powerLeft : 0);
    analogWrite(MotorRightForward, powerRight > 0 ? powerRight : 0);
    analogWrite(MotorRightBackward, powerRight < 0 ? -powerRight : 0);
  }

  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;
}

// ======================================================
// ================   FUNCTIES   ========================
// ======================================================
void stopMotors() {
  analogWrite(MotorLeftForward, 0);
  analogWrite(MotorLeftBackward, 0);
  analogWrite(MotorRightForward, 0);
  analogWrite(MotorRightBackward, 0);
}

void onUnknownCommand(char *command) {
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onSet() {
  char *param = sCmd.next();
  char *value = sCmd.next();

  if (strcmp(param, "cycle") == 0) params.cycleTime = atol(value);
  else if (strcmp(param, "power") == 0) params.power = atol(value);
  else if (strcmp(param, "diff") == 0) params.diff = atof(value);
  else if (strcmp(param, "kp") == 0) params.kp = atof(value);

  /* parameters een nieuwe waarde geven via het set commando doe je hier ... */

  EEPROM_writeAnything(0, params);
  EEPROM.commit();
}

void onDebug() {
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);
  
  /* parameters weergeven met behulp van het debug commando doe je hier ... */
  SerialPort.print("black: ");
  for (int i = 0; i < 8; i++)
  {
    SerialPort.print(params.black[i]);
    SerialPort.print(" ");
  }
  SerialPort.println(" ");

  SerialPort.print("white: ");
  for (int i = 0; i < 8; i++)
  {
    SerialPort.print(params.white[i]);
    SerialPort.print(" ");
  }
  SerialPort.println(" ");

  SerialPort.print("normalised: ");
  for (int i = 0; i < 8; i++)
  {
    SerialPort.print(normalised[i]);
    SerialPort.print(" ");
  }
  SerialPort.println(" ");

  SerialPort.print("position: ");
  SerialPort.println(debugPosition);

  SerialPort.print("power: ");
  SerialPort.println(params.power);
  SerialPort.print("diff: ");
  SerialPort.println(params.diff);  
  SerialPort.print("kp: ");
  SerialPort.println(params.kp);

  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;
}

void onCalibrate() {
  char *param = sCmd.next();

  if (strcmp(param, "black") == 0) {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 8; i++) params.black[i] = analogRead(sensor[i]);
    SerialPort.println("done");
  } else if (strcmp(param, "white") == 0) {
    SerialPort.print("start calibrating white... ");
    for (int i = 0; i < 8; i++) params.white[i] = analogRead(sensor[i]);
    SerialPort.println("done");
  }

  EEPROM_writeAnything(0, params);
  EEPROM.commit();
}

void onRun() {
  run = true;
}

// ======================================================
// ========== IoT Cloud variable callbacks ==============
// ======================================================
void onStartStopChange() {
  if (startStop) {
    run = true;
    led = true;
    digitalWrite(LED_PIN, HIGH);
    SerialPort.println("Robot gestart via IoT Cloud!");
  } else {
    run = false;
    led = false;
    digitalWrite(LED_PIN, LOW);
    stopMotors();
    SerialPort.println("Robot gestopt via IoT Cloud!");
  }
}

void onLedChange() {
  digitalWrite(LED_PIN, led ? HIGH : LOW);
}
