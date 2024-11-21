# Compte rendu TP LoRA
## Auteurs
- PIPON Romain (romain.pipon@etu.univ-nantes.fr)
- LECLERC Léo (leo.leclerc1@etu.univ-nantes.fr)
## Installation Window

```bash
$ pip3 install pyserial
```

Installer le driver fournit dans le fichier ZIP ("CP210x_Windows_Drivers.zip") et ouvrir **CP210xVCPInstaller_x64.exe**

## Test ESP32
Pour tester que toute fonctionne bien nous avons utiliser le script ci-dessous. Ce Script permet d'afficher un message sur l'écran fournit avec l'esp32 et afficher un message sur le port série.

```c
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15,4,16);
int d1=0,d2=0,d3=0;

void dispData()
{
  char dbuf[16];
  u8x8.clear();
  Serial.println("titre");
  u8x8.drawString(0,1,"titre");
  sprintf(dbuf,"Data1:%d",d1); u8x8.drawString(0,2,dbuf);
  sprintf(dbuf,"Data2:%d",d2); u8x8.drawString(0,3,dbuf);
  sprintf(dbuf,"Data3:%d",d3); u8x8.drawString(0,4,dbuf);
  delay(6000);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
}

void loop() {
  // put your main code here, to run repeatedly:
  d1++;
  d2+=2;
  d3+=4;
  dispData();
}

```
## Test wifi

Pour Tester si le module wifi de l'esp32 fonctionne bien nous avons utiliser le script suivant qui doit nous fournit une ip dans le port série.

```c
#include <WiFi.h>

const char* ssid = "Iphone Romain";
const char* password = "30141921";

void setup(){
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

void loop(){}
```

## Test envoie message MQTT

Voici un petit script pour envoyer un message vers un broker MQTT. Ici on envoie 'Hello, MQTT Leo!'.

Installer la lib **PubSubClient** (Croquis/Inclure une bibliothèque/Gérer les bibliothèques)

```c
#include <WiFi.h>
#include <PubSubClient.h>

// Configuration du réseau WiFi
const char* ssid = "ssid";
const char* password = "password";

// Configuration du Broker MQTT
const char* mqtt_server = "test.mosquitto.org";

// Initialisation des objets WiFi et MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Fonction pour se connecter au WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion au réseau ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP : ");
  Serial.println(WiFi.localIP());
}

// Fonction callback pour gérer les messages reçus (si besoin)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu sur le topic : ");
  Serial.print(topic);
  Serial.print(". Message : ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Fonction pour se connecter au Broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion au Broker MQTT...");
    if (client.connect("Client_Arduino")) {
      Serial.println("connecté");
    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  // Si tu veux recevoir des messages

  // Connexion initiale au Broker
  reconnect();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Exemple d'envoi d'un message MQTT
  String message = "Hello, MQTT Leo!";
  client.publish("tp_popo/alban", message.c_str());

  delay(5000); // Envoie un message toutes les 5 secondes
}
```

## Test LoRa

Installer la lib **LoRa** (Croquis/Inclure une bibliothèque/Gérer les bibliothèques)

### Réception message via LoRa

Afin de réduire la taille du packet transmit, nous avons créé une structure pour notre paquet
```c
struct packet {
  unsigned short freq : 8;
  unsigned int sb : 12;
  unsigned char sf : 4;
} packet_data;
```
Cela est possible car la frequence va varier de 868.1 a 868.5.

Ce code est un programme Arduino pour un ESP32 combinant MQTT, Wi-Fi et LoRa pour transmettre et recevoir des données. Voici un résumé de son fonctionnement :
- Wi-Fi et MQTT : Le programme connecte l'ESP32 au Wi-Fi avec les identifiants fournis. Il établit une connexion avec le broker MQTT (test.mosquitto.org) sur le port 1883 et s'abonne au topic CR7. Des données sérialisées (3 octets) représentant la structure packet sont publiées sur ce topic.
- Réception MQTT et configuration LoRa : Lorsqu'un message est reçu via MQTT, les 3 octets sont désérialisés pour extraire les paramètres LoRa (freq, sb, sf). Ces paramètres sont utilisés pour configurer un module LoRa (fréquence, bande passante, facteur d'étalement).
- Transmission LoRa : Une fonction loop_lora envoie périodiquement un paquet LoRa contenant des données flottantes (sdp.data).

Le code fusionne la communication MQTT (réseau) et LoRa (radio), permettant un transfert de paramètres via MQTT pour piloter des transmissions LoRa dynamiques.

```c
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DI0 26

union pack {
  uint8_t frame[16];
  float data[4];
} sdp;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "Iphone Romain";        // your network SSID
char pass[] = "30141921";    // your network password

struct packet {
  unsigned short freq : 8;
  unsigned int sb : 12;
  unsigned char sf : 4;
} packet_data;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "CR7";

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  Serial.print("Subscribing to topic: ");
  Serial.println(topic);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topic);

  packet_data.freq = 0;
  packet_data.sf = 0;
  packet_data.sb = 125; 
  byte serializedData[3];
  serializedData[0] = packet_data.freq;
  serializedData[1] = (packet_data.sb >> 4) & 0xFF;
  serializedData[2] = ((packet_data.sb & 0xF) << 4) | (packet_data.sf & 0xF);

  // Envoi des données via MQTT
  mqttClient.beginMessage(topic);
  mqttClient.write(serializedData, sizeof(serializedData));  // Utilisation de write pour envoyer les données brutes
  mqttClient.endMessage();



  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.println();
}

void loop() {
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keep alive which avoids being disconnected by the broker
  mqttClient.poll();
}

void loop_lora() {
  LoRa.beginPacket();
  sdp.data[0] = 12.0;
  sdp.data[1] = 2.0;
  LoRa.write(sdp.frame, 16);
  LoRa.endPacket();
  Serial.println("Paquet envoyé");
  delay(2000);
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {  // Lecture des trois octets reçus
    byte serializedData[3];

    for (int i = 0; i < 3; i++) {
      serializedData[i] = mqttClient.read();  // Lire chaque octet successivement
    }

    // Désérialisation des données dans la structure packet_data
    packet_data.freq = serializedData[0];  // Premier octet contient freq
    packet_data.sb = (serializedData[1] << 4) | ((serializedData[2] >> 4) & 0x0F);  // Combinaison des bits pour sb
    packet_data.sf = serializedData[2] & 0x0F;  // Les 4 derniers bits de serializedData[2] contiennent sf

    Serial.println(packet_data.sb);
    Serial.println(packet_data.freq);
    Serial.println(packet_data.sf);

    pinMode(DI0, INPUT);
    SPI.begin(SCK,MISO,MOSI,SS);
    LoRa.setPins(SS,RST,DI0);
    if(!LoRa.begin((packet_data.freq + 868) * 1E6)) {
      Serial.println("KO");
      while(1);
    } else {
      Serial.println("OK");
    }
    LoRa.setSpreadingFactor(packet_data.sf + 7);
    LoRa.setSignalBandwidth((packet_data.sb + 50) * 1E3);

    while(1) {
      loop_lora();
    }
  }
}
```
