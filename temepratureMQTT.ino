//Mehrere DHT22 mit Arduino einlesen
//Original von: http://forum.arduino.cc/index.php?topic=256451.0

#include <DHT.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHT2TYPE DHT22  // DHT 22  (AM2302)

//======================================================================
//Pins der Sensoren angeben:
#define DHT2PIN 48   
#define DHT3PIN 46
#define DHT4PIN 44 
#define DHT5PIN 42
#define DHT6PIN 40    
#define DHT7PIN 38
#define DHT8PIN 36


DHT dht[] = {DHT(DHT2PIN, DHT2TYPE), DHT(DHT3PIN, DHT2TYPE), DHT(DHT4PIN, DHT2TYPE), DHT(DHT5PIN, DHT2TYPE), DHT(DHT6PIN, DHT2TYPE), DHT(DHT7PIN, DHT2TYPE), DHT(DHT8PIN, DHT2TYPE)};

//Einfügen Ethernet

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED }; //eine MAC Adresse wählen, darf im eigenen Netz nur 1x vorkommen
IPAddress ip(192, 168, 178, 220); //eine gültige IP Adresse für das eigene Netz
IPAddress server(192, 168, 178, 101); //die Adresse wo der eigene MQTT Broker drauf läuft


EthernetClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("Arduino_1", "Arduino_1", 0, true, "offline")) {
    // Once connected, publish an announcement...
    client.publish("Arduino_1","online", true);
    // ... and resubscribe
    client.subscribe("inTopic");
  }
  //Serial.println("MQTT verbunden");
  return client.connected();
}

//Ende Ethernet

void setup() {
  
  client.setServer(server, 1883);
  Ethernet.begin(mac, ip);

  delay(3500);
  lastReconnectAttempt = 0;
   
  Serial.begin(115200);

  //======================================================================
  //Sensoren starten:
  dht[0].begin(); // Arbeitszimmer
  dht[1].begin(); // Küche
  dht[2].begin(); // Wohnzimmer
  dht[3].begin(); // Schlafzimmer
  dht[4].begin(); // Kinderzimmer
  dht[5].begin(); // Bad
  dht[6].begin(); // Badboden
  
}

void loop() {

unsigned long previousMillis = 0; //Zählervariable, zählt Millisekunden seit dem letzten Funktionsaufruf nach oben
const long interval = 120000; //120000 Millisekunden aka 120 Sekunden, das Interval wie oft der Sensor überhaupt benutzt wird

//Connect herstellen

if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    client.loop();
  }

//----

// Werte auslesen
  for (int i = 0; i < 6; i++) { //Anzahl der angeschlossenen Sensoren eingeben
  
// Variablen Dekleration neu

static char humidity[15]; //Speicherbereich reservieren um die Fechtigkeit zu speichern
static char temperature[15];
float h[2];
float t[2];
//Feld für Topic je Raum
char* Hum[]    = { "Arduino_1/AZ/Hum", "Arduino_1/Kueche/Hum", "Arduino_1/WZ/Hum", "Arduino_1/SZ/Hum", "Arduino_1/Kiz/Hum", "Arduino_1/Bad/Hum","Arduino_1/Badboden/Hum" };
char* Temp[]    = { "Arduino_1/AZ/Temp", "Arduino_1/Kueche/Temp", "Arduino_1/WZ/Temp", "Arduino_1/SZ/Temp", "Arduino_1/Kiz/Temp", "Arduino_1/Bad/Temp", "Arduino_1/Badboden/Temp" };

// ende 
 
    h[i] = dht[i].readHumidity();
    t[i] = dht[i].readTemperature();
    delay(10000); // Zeit bis zum Auslesen des nächsten Sensors 10000 = Alle 10 Sek der nächste Sensor -> alle Sensoren nach 50 Sek gelesen
   // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t[i]) || isnan(h[i])) {
      Serial.print("Failed to read from DHT #"); Serial.println(i);
    client.publish(Hum[i],"Sensorfehler",true);
    client.publish(Temp[i],"Sensorfehler",true);//true sendet die Nachricht retained,

    } 
  else 
  {
    client.publish("Arduino_1","online", true);
    dtostrf(h[i],6, 1, humidity);
    dtostrf(t[i],6, 1, temperature);
    
    client.publish(Hum[i], humidity, true);
    client.publish(Temp[i], temperature, true);
    
    Serial.print("Humidity "); Serial.print(i); Serial.print(": ");
    Serial.print(h[i]);
    Serial.print(" %\t");
    Serial.print("Temperature "); Serial.print(i); Serial.print(": ");
    Serial.print(t[i]);
    Serial.println(" *C");
    
  }
   
  } //ENDE for Schleife
  Serial.println();
}
