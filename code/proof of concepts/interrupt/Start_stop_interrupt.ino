/*
  Start/Stop robot met interrupt en debounce
  ------------------------------------------
  - Monostabiele knop op D2 (INT0)
  - Wisselt tussen 'gestopt' en 'actief' bij elke druk
  - Debounce in software
  - Geen polling => timing van restprogramma blijft stabiel
*/

const byte KNOP_PIN = 2;              // Externe interrupt pin
const byte LED_PIN = 13;              // Indicatie van status
volatile bool robotActief = false;    // Huidige status
volatile unsigned long laatsteInterruptTijd = 0; // Voor debounce
const unsigned long DEBOUNCE_TIJD = 200; // 200 ms

void setup() {
  pinMode(KNOP_PIN, INPUT_PULLUP);   // knop naar GND
  pinMode(LED_PIN, OUTPUT);
  
  // Interrupt bij dalende flank (knop ingedrukt)
  attachInterrupt(digitalPinToInterrupt(KNOP_PIN), knopInterrupt, FALLING);

  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  Serial.println("Robot klaar. Druk op de knop om te starten/stoppen.");
}

void loop() {
  // Hier komt je hoofdprogramma voor de robot
  if (robotActief) {
    digitalWrite(LED_PIN, HIGH);
    // ... robot beweegt / voert taken uit ...
  } else {
    digitalWrite(LED_PIN, LOW);
    // ... robot staat stil / wacht op start ...
  }

  // (Voorbeeld: kleine vertraging om CPU te sparen)
  delay(50);
}

// ISR (Interrupt Service Routine)
void knopInterrupt() {
  unsigned long huidigeTijd = millis();
  
  // Debounce: enkel reageren als laatste druk minstens 200 ms geleden is
  if (huidigeTijd - laatsteInterruptTijd > DEBOUNCE_TIJD) {
    robotActief = !robotActief; // toggle status
    laatsteInterruptTijd = huidigeTijd;
  }
}
