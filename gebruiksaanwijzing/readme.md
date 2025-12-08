# Gebruiksaanwijzing

### opladen / vervangen batterijen

  1. Zet de robot uit via de schuifschakelaar.
  2. Verwijder de 18650-batterijen uit de batterijhouder.
  3. Plaats de batterijen in de externe 2-slots oplader (let op polariteit + / –).
  4. Wacht tot beide laadindicatoren “vol” tonen.
  5. Plaats de opgeladen batterijen terug in de batterijhouder.
  6. Zet de robot opnieuw aan via de schuifschakelaar.
   
### draadloze communicatie
#### verbinding maken

  1. Zet de robot aan.
  2. De Arduino Nano ESP32 maakt een eigen WiFi-access point aan, of maakt verbinding met een gekende WiFi-netwerkconfiguratie (afhankelijk van firmware).
  3. Zoek het SSID van de robot op je laptop of smartphone en verbind ermee.
  4. Open de robot-interface via het opgegeven IP-adres (meestal 192.168.4.1 bij access-point mode).
  5. Vanaf de webinterface of seriële terminal kunnen alle commando’s worden verstuurd.

#### commando's

Alle commando’s worden als tekst zonder hoofdletters verzonden.
Parameterwaarden worden gescheiden door spaties.

  1. Controle & status

    debug on — toont alle sensorwaarden en berekende correcties.
    debug off — stopt debug-output.

  2. Actie

    start — robot begint te rijden volgens ingestelde PID-parameters.
    stop — robot stopt onmiddellijk alle motoren.

  3. Instellingen

    set cycle [µs] — bepaalt de looptijd van één regelcyclus in microseconden.
    set power [0..255] — basisvermogen waarmee beide motoren draaien.
    set diff [0..1] — maximale snelheidsafwijking tussen linker en rechter motor.
    set kp [waarde] — proportionele versterking voor de regelaar.
    set ki [waarde] — integrale versterking.
    set kd [waarde] — differentiële versterking.

  4. Kalibratie

    calibrate black — registreert min/max-waarden op zwarte lijn.
    calibrate white — registreert min/max-waarden op witte achtergrond.

### kalibratie

  1. Zet de robot op een zwarte lijn en voer calibrate black uit.
  
  Beweeg de robot tijdens de kalibratie lichtjes over de lijn zodat alle sensoren met minimale reflectie gemeten worden.

  3. Zet de robot volledig op witte ondergrond en voer calibrate white uit.

  Wieg de robot zodat alle sensoren maximale reflectie meten.

  5. De robot slaat beide waardes op en gebruikt ze om de lijnpositie nauwkeurig te berekenen.

  Tip: herkalibreer wanneer de ondergrond of belichting verandert.

### settings
De robot rijdt stabiel met volgende parameters:  

### start/stop button
uitleg locatie + werking start/stop button
