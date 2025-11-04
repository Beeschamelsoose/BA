include <DHT.h>

#define DHTPIN 12       // Datenpin des DHT11-Sensors
#define DHTTYPE DHT11  // Sensor-Typ DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);  // Serielle Kommunikation starten
  dht.begin();         // DHT-Sensor initialisieren
  delay(1000);         // Sensor ist eher langsam, eine Sekunde warten
  Serial.println("DHT11 Temperatur- und Luftfeuchtigkeitsmessung gestartet.");
}

void loop() {
  // Temperatur und Luftfeuchtigkeit auslesen
  float temperature = dht.readTemperature();  // Temperatur in Celsius
  float humidity = dht.readHumidity();        // Luftfeuchtigkeit in %

  // Überprüfen, ob der Sensor gültige Daten liefert
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Fehler beim Auslesen des Sensors!");
    delay(2000);  // 2 Sekunden warten und erneut versuchen
    return;
  }

  // Temperatur im seriellen Monitor ausgeben
  Serial.print("Temperatur: ");
  Serial.print(temperature);
  Serial.println(" °C");
  // Luftfeuchtigkeit im seriellen Monitor ausgeben
  Serial.print("Luftfeuchtigkeit: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.println("-----------------------------");
  delay(2000);  // 2 Sekunden warten, bevor die Werte erneut ausgelesen werden
}
