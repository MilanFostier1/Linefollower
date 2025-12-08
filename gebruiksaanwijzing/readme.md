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
debug [on/off]  
start  
stop  
set cycle [µs]  
set power [0..255]  
set diff [0..1]  
set kp [0..]  
set ki [0..]  
set kd [0..]  
calibrate black  
calibrate white  

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
