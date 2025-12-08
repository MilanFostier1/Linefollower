# Instructable

Een instructable is een stappenplan - zonder verdere uitleg - hoe je vertrekkend van de bill of materials en gebruik makend van de technische tekeningen de robot kan nabouwen. Ook de nodige stappen om de microcontroller te compileren en te uploaden staan beschreven.  

### stap 1

  1. bestel alle componenten uit de bill of materials

### stap 2 — Voorbereiding onderdelen

  1. Controleer ontvangst en aantallen tegen de BOM.
  2. Laad de 18650-batterijen volledig op in de batterijoplader en plaats ze in de batterijhouder (serie = 7,4 V).
  3. Monteer pin-headers waar nodig op de Arduino Nano ESP32 en op de QTR-8A (indien niet vooraf gesoldeerd).

### stap 3 — Mechanische montage chassis

  1. Leg de experimenteerprint (9×15 cm) als chassis.
  2. Plaats motorbeugels op het chassis op positie voor twee wielen.
  3. Monteer de twee 50:1 micro metal gearmotoren in de beugels en zet ze vast.
  4. Druk/wissel de Pololu-wielen op de motorassen.
  5. Monteer de batterijhouder op het chassis (centraal achter/onder) met schroeven of dubbelzijdig tape.
  6.Monteer de QTR-8A aan de voorkant van het chassis (afstand tot grond ~5–12 mm afhankelijk van test).
  7. Monteer de schuifschakelaar op een toegankelijke plek (aan/uit).
  8. Bevestig de Arduino Nano ESP32 op het chassis (stand-off of dubbelzijdig tape).
  9. Monteer de DRV8833 H-brug dicht bij de motoren maar minimaal vibratiebelasting.

### stap 4 — Elektrische onderdelen klaarzetten

  1. Knip en strip draden uit de 40-aderige draadset. Maak korte draadjes voor signaal en langere voor voeding/massa.
  2. Plaats condensator 100µF over de motorvoeding (tussen +7.4V en GND) dicht bij de DRV8833.
  3. Plaats schijfcondensator 100nF ook dicht bij de voedingspinnen van de DRV8833 / Arduino.

### stap 5 — Bekabeling (wiring overzicht)

Gebruik dit schema als uitgangspunt; pas pin-nummers in de code aan als je andere pins kiest.

  1. Voeding

  1.1. Batterijhouder + → VIN (of + van DRV8833 VMOT) (7.4 V)
  
  1.2. Batterijhouder − → GND (alle GND’s met elkaar verbonden)
  
  1.3. Schuifschakelaar in serie met + batterij (aan/uit)

  2. DRV8833 H-brug naar motoren

  2.1. DRV8833 VM → +7.4V (batterij via schakelaar)
  
  2.2. DRV8833 GND → GND
  
  2.3. Motor A outputs → Motor links (M1+, M1−)
  
  2.4. Motor B outputs → Motor rechts (M2+, M2−)
  

  3. DRV8833 signal naar Arduino Nano ESP32 (aanbevolen pin-layout — wijzigbaar)

  3.1. IN1 (Motor A AIN1) → GPIO 14
  
  3.2. IN2 (Motor A AIN2) → GPIO 27
  
  3.3. IN3 (Motor B BIN1) → GPIO 26
  
  3.4. IN4 (Motor B BIN2) → GPIO 25
  
  3.5. (optioneel) PWM inputs als beschikbaar / of gebruik digitale PWM op dezelfde pins

  4. QTR-8A sensor
  
  4.1. VCC → 5V (of 3.3V als QTR-8A daar compatibel mee; controleer je module)
  
  4.2. GND → GND
  
  4.3. OUTs → analoge of digitale ingangen op Nano ESP32 (bijv. A0..A7 → GPIOs 34, 35, 32, 33, 39, 36, 4, 2) — kies beschikbare ADC/GPIO pins
  
  4.4. Als de QTR-8A module I2C of analoog heeft, verbind overeenkomstig.
  
  5. Extra
  
  5.1. USB-C kabeltje → Arduino voor programmeren en debug (zorg dat schakeling losgekoppeld is of gescheiden van motorvoeding tijdens upload).
  
  5.2. Condensatoren parallel aan motorvoeding (zoals hierboven) om ruis en spikes te dempen.

### stap 6 — Solderen & bevestigen

  1. Soldeer motoren aan motorkabels; soldeer kabels aan DRV8833 motoruitgangen.
  2. Soldeer pin-headers / verbindingskabels voor QTR-8A en Arduino.
  3. Controleer alle verbindingen met multimeter op kortsluiting en correcte continuïteit.
  4. Monteer alles definitief op het chassis.

### stap 7 — Veiligheidscheck vóór eerste inschakeling

  1. Controleer polariteit van batterijen.
  2. Schakel schakelaar uit, verbind batterijen.
  3. Meet spanningen op DRV8833 VM en Arduino VIN.
  4. Zet schakelaar aan en controleer dat geen rook/oververhitting plaatsvindt.
  5. Schakel direct uit bij verdachte geuren of hitte.

### stap 8 — Firmware voorbereiden (software)

  1. Benodigde software / libraries (kort):
     
  1.1. Arduino IDE (of Arduino CLI) of PlatformIO.
  1.2. ESP32 board-support voor Arduino (esp32).
  1.3. Pololu QTR-sensor library (bijv. QTRSensors van Pololu).
  1.4. (optioneel) AccelStepper of een eenvoudige motor control helper als je PWM / snelheidsregeling wil.

  2. Arduino IDE (GUI) korte instructies:
  
  2.1. Installeer Arduino IDE.
  2.2. Ga naar File > Preferences → voeg https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json toe aan Additional Boards            Manager URLs (als nodig).
  2.3. Ga naar Tools > Board > Boards Manager → zoek esp32 → installeer esp32 by Espressif.
  2.4. Selecteer board: Arduino Nano ESP32 (of het specifieke Nano ESP32 board in de lijst).
  2.5. Selecteer juiste COM-poort onder Tools > Port.
  2.6. Installeer via Library Manager de QTRSensors library.

  3. Arduino CLI (command-line) voorbeeld:
    (als je arduino-cli hebt geïnstalleerd)

          arduino-cli core update-index
          arduino-cli core install esp32:esp32
          arduino-cli lib install "QTRSensors"
          # Compile
          arduino-cli compile --fqbn esp32:esp32:arduino_nano_esp32 path/to/your/sketch
          # Upload (pas de port aan)
          arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:arduino_nano_esp32 path/to/your/sketch


### stap 9 — Voorbeeld minimale sketch (skeleton)

  Pas pin-definities aan aan jouw bedrading en QTR-pin-keuze.

    // Minimal skeleton voor line-following basis
    #include <Arduino.h>
    #include <QTRSensors.h>
    
    // Pin-definities (pas aan)
    const int AIN1 = 14;
    const int AIN2 = 27;
    const int BIN1 = 26;
    const int BIN2 = 25;
    
    // QTR
    QTRSensors qtr;
    const uint8_t qtrPins[8] = {34,35,32,33,39,36,4,2};
    
    void setup() {
      // Motor pins
      pinMode(AIN1, OUTPUT);
      pinMode(AIN2, OUTPUT);
      pinMode(BIN1, OUTPUT);
      pinMode(BIN2, OUTPUT);
    
      // QTR init
      qtr.setTypeRC();
      qtr.setSensorPins(qtrPins, 8);
      qtr.setTimeout(2500);
      qtr.setEmitterPin(21); // indien van toepassing
      Serial.begin(115200);
    }
    
    void driveForward() {
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    }
    
    void stopMotors() {
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
    }
    
    void loop() {
      unsigned int sensorValues[8];
      qtr.read(sensorValues);
      // Basale beslislogica (vervang door echte line-follow algoritme)
      unsigned long sum = 0;
      for (int i=0;i<8;i++) sum += sensorValues[i];
      if (sum > 1000) {
        driveForward();
      } else {
        stopMotors();
      }
      delay(50);
    }

### stap 10 — Compileer & upload

  1. Verbind Arduino via USB-C met je PC.
  2. Selecteer board en poort in Arduino IDE.
  3. Klik Verify (compile).
  4. Klik Upload.
  5. Open Serial Monitor (115200) voor debug-output.

### stap 11 — Testen & afregelen

  1. Leg een testlijn (zwart op wit) voor de QTR-sensor.
  2. Zet robot aan (schakelaar).
  3. Bekijk seriële waarden en pas drempels / PID-logica aan in je code.
  4. Kalibreer QTR-sensor (meerdere metingen in verschillende posities) en update code indien nodig.
  5. Finetune motorsnelheid (PWM) en wrijving.

### stap 12 — Eindcontrole & documentatie

1. Zorg dat alle kabels netjes vastzitten en geen bewegende delen raken.
2. Noteer pinout en eventuele wijzigingen in de BOM/cablerun.
3. Maak backup van de definitieve code en bewaar een kopie van het schema.
