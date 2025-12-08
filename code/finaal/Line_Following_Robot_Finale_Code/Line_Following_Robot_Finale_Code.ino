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
float iTerm = 0;
float lastErr;
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
  float ki;
  float kd;
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
  power = params.power;
  diff = params.diff;
  kp = params.kp;

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

    // controle: zijn ALLE sensoren wit? -> motoren uit!
    if (normalised[index] > 3000) startStop = false;

    if (index == 0) position = -30;
    else if (index == 7) position = 30;
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
      position *= 10;      // afstand tussen 2 sensoren 9,52 cm
    }
    debugPosition = position;

    // ALS ALLE SENSOREN ZWART ZIJN → RECHT DOOR
    bool allBlack = true;
    for (int i = 0; i < 8; i++) {
      if (normalised[i] > 500) allBlack = false;   // threshold voor zwart
    }

    if (allBlack) {
      // Rechtdoor rijden met parametersnelheid
      analogWrite(MotorRightForward, params.power);
      analogWrite(MotorRightBackward, 0);
      analogWrite(MotorLeftForward, params.power);
      analogWrite(MotorLeftBackward, 0);

      return;  // PID-sturing overslaan
    }

    /* bereken error = setpoint - positie */
    float error = -position;

    /* proportioneel regelen */
    float output = error * params.kp;

    /* integrerend regelen */
    iTerm += params.ki * error;
    iTerm = constrain(iTerm, -510, 510);
    output += iTerm; 

    /* differentiërend regelen */
    output += params.kd * (error - lastErr);
    lastErr = error; 

    /* output begrenzen tot wat fysiek mogelijk is */
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

    analogWrite(MotorRightForward, powerLeft > 0 ? powerLeft : 0);
    analogWrite(MotorRightBackward, powerLeft < 0 ? -powerLeft : 0);
    analogWrite(MotorLeftForward, powerRight > 0 ? powerRight : 0);
    analogWrite(MotorLeftBackward, powerRight < 0 ? -powerRight : 0);
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

  if (strcmp(param, "cycle") == 0)
  {
    long newCycleTime = atol(value);
    float ratio = ((float) newCycleTime) / ((float) params.cycleTime);
    params.ki *= ratio;
    params.kd /= ratio;
    params.cycleTime = newCycleTime;
  }
  else if (strcmp(param, "power") == 0) params.power = atol(value); // de snelheid waarmee de robot rijd!
  else if (strcmp(param, "diff") == 0) params.diff = atof(value); // trager of sneller rijden als de fout groot of klein is! tussen 0 en 1! (voor in de bochten)!
  else if (strcmp(param, "kp") == 0) params.kp = atof(value); // de grootte waarmee de roobot zijn fout corrigeerd!
  else if (strcmp(param, "ki") == 0)
  {
    float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.ki = atof(value) * cycleTimeInSec;
  }
  else if (strcmp(param, "kd") == 0)
  {
    float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    params.kd = atof(value) / cycleTimeInSec;
  }

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

  float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
  float ki = params.ki / cycleTimeInSec;
  SerialPort.print("Ki: ");
  SerialPort.println(ki);

  float kd = params.kd * cycleTimeInSec;
  SerialPort.print("Kd: ");
  SerialPort.println(kd);
  
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
  iTerm = 0; 
}

// ======================================================
// ========== IoT Cloud variable callbacks ==============
// ======================================================
void onStartStopChange() {
  if (startStop) {
    run = true;
    led = true;
    digitalWrite(LED_PIN, HIGH);
    iTerm = 0; 
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

void onPowerChange()  {
  params.power = power;       // cloud → struct
  EEPROM_writeAnything(0, params);
  EEPROM.commit();
  Serial.println("Power updated from IoT Cloud");
}

void onDiffChange()  {
  params.diff = diff;         // cloud → struct
  EEPROM_writeAnything(0, params);
  EEPROM.commit();
  Serial.println("Diff updated from IoT Cloud");
}

void onKpChange() {
  params.kp = kp;             // cloud → struct
  EEPROM_writeAnything(0, params);
  EEPROM.commit();
  Serial.println("KP updated from IoT Cloud");
}