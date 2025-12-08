// motor A control pins
const int motA_in1 = 5;
const int motA_in2 = 4;
// motor B control pins
const int motB_in1 = 3;
const int motB_in2 = 2;

void setup() {
  pinMode(motA_in1, OUTPUT);
  pinMode(motA_in2, OUTPUT);
  pinMode(motB_in1, OUTPUT);
  pinMode(motB_in2, OUTPUT);
  
  // begin met motoren stil
  digitalWrite(motA_in1, LOW);
  digitalWrite(motA_in2, LOW);
  digitalWrite(motB_in1, LOW);
  digitalWrite(motB_in2, LOW);
  
  Serial.begin(115200);
}

void loop() {
  // Voorbeeld: motor A vooruit met traploze snelheidsopbouw, motor B stil
  Serial.println("Motor A: vooruit opbouw snelheid");
  for (int pwm = 0; pwm <= 255; pwm += 5) {
    analogWrite(motA_in1, pwm);
    digitalWrite(motA_in2, LOW);
    analogWrite(motB_in1, 0);
    digitalWrite(motB_in2, LOW);
    delay(20);
  }
  
  delay(1000);
  
  // Motor A vol snelheid vooruit, Motor B achteruit met halve snelheid
  Serial.println("Motor A: vol vooruit, Motor B: achteruit halve snelheid");
  analogWrite(motA_in1, 255);
  digitalWrite(motA_in2, LOW);
  analogWrite(motB_in1, 0);
  analogWrite(motB_in2, 128); // halve snelheid achteruit
  
  delay(2000);
  
  // Motoren stil
  Serial.println("Motoren stil");
  digitalWrite(motA_in1, LOW);
  digitalWrite(motA_in2, LOW);
  digitalWrite(motB_in1, LOW);
  digitalWrite(motB_in2, LOW);
  
  delay(1000);
  
  // Motor A achteruit met traploze snelheid afbouw, Motor B vooruit vol
  Serial.println("Motor A: achteruit afbouw snelheid, Motor B: vooruit vol");
  analogWrite(motB_in1, 255);
  digitalWrite(motB_in2, LOW);
  for (int pwm = 0; pwm <= 255; pwm += 5) {
    analogWrite(motA_in1, 0);
    analogWrite(motA_in2, 255 - pwm);
    delay(20);
  }
  
  delay(2000);
}
