#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>

// Définitions des broches utilisées par le module LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DI0 26

// Union pour stocker des données dans un format interchangeable
union pack
{
  uint8_t frame[16]; // Tableau brut de 16 octets
  float data[4];     // 4 valeurs float correspondant aux données
} sdp;

// Informations de connexion WiFi
char ssid[] = "Iphone Romain"; // Nom du réseau WiFi
char pass[] = "30141921";      // Mot de passe du réseau WiFi

// Structure des données reçues
struct packet
{
  unsigned short freq : 8; // Fréquence (8 bits)
  unsigned int sb : 12;    // Bande passante (12 bits)
  unsigned char sf : 4;    // Facteur d'étalement (4 bits)
} packet_data;

// Déclaration des objets WiFi et MQTT
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Informations sur le broker MQTT
const char broker[] = "test.mosquitto.org"; // Adresse du broker MQTT
int port = 1883;                            // Port du broker MQTT
const char topic[] = "CR7";                 // Topic utilisé pour la communication

void setup()
{
  // Initialisation de la communication série
  Serial.begin(115200);
  while (!Serial)
  {
    ; // Attente de l'ouverture du port série
  }

  // Configuration et connexion au réseau WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    // Échec, réessai
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // Connexion au broker MQTT
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ; // Boucle infinie en cas d'échec
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  // Configuration du callback pour la réception de messages MQTT
  mqttClient.onMessage(onMqttMessage);

  // Souscription au topic défini
  Serial.print("Subscribing to topic: ");
  Serial.println(topic);
  Serial.println();
  mqttClient.subscribe(topic);

  // Initialisation des données pour un premier envoi via MQTT
  packet_data.freq = 0;
  packet_data.sf = 0;
  packet_data.sb = 125;
  byte serializedData[3];
  serializedData[0] = packet_data.freq;
  serializedData[1] = (packet_data.sb >> 4) & 0xFF;
  serializedData[2] = ((packet_data.sb & 0xF) << 4) | (packet_data.sf & 0xF);

  // Envoi des données via MQTT
  mqttClient.beginMessage(topic);
  mqttClient.write(serializedData, sizeof(serializedData)); // Envoi des données brutes
  mqttClient.endMessage();

  // Affichage du topic utilisé
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.println();
}

void loop()
{
  // Appel régulier à poll() pour gérer les messages MQTT et maintenir la connexion
  mqttClient.poll();
}

// Fonction pour gérer l'envoi de données LoRa
void loop_lora()
{
  // Création et envoi d'un paquet LoRa
  LoRa.beginPacket();
  sdp.data[0] = 12.0;        // Exemple de données (float)
  sdp.data[1] = 2.0;         // Exemple de données supplémentaires
  LoRa.write(sdp.frame, 16); // Envoi des données sous forme brute
  LoRa.endPacket();
  Serial.println("Paquet envoyé");
  delay(2000); // Pause avant le prochain envoi
}

// Callback pour la réception de messages MQTT
void onMqttMessage(int messageSize)
{
  // Message reçu, affichage du topic et de sa longueur
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // Lecture et traitement des données reçues
  while (mqttClient.available())
  {
    byte serializedData[3]; // Tableau pour stocker les données brutes reçues

    for (int i = 0; i < 3; i++)
    {
      serializedData[i] = mqttClient.read(); // Lecture de chaque octet
    }

    // Désérialisation des données reçues dans la structure packet_data
    packet_data.freq = serializedData[0];                                          // Extraction de la fréquence
    packet_data.sb = (serializedData[1] << 4) | ((serializedData[2] >> 4) & 0x0F); // Extraction de la bande passante
    packet_data.sf = serializedData[2] & 0x0F;                                     // Extraction du facteur d'étalement

    // Affichage des paramètres LoRa extraits
    Serial.println(packet_data.sb);
    Serial.println(packet_data.freq);
    Serial.println(packet_data.sf);

    // Configuration des paramètres LoRa
    pinMode(DI0, INPUT);
    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);
    if (!LoRa.begin((packet_data.freq + 868) * 1E6))
    {
      Serial.println("KO");
      while (1)
        ; // Boucle infinie en cas d'échec de configuration
    }
    else
    {
      Serial.println("OK");
    }
    LoRa.setSpreadingFactor(packet_data.sf + 7);
    LoRa.setSignalBandwidth((packet_data.sb + 50) * 1E3);

    // Boucle infinie pour l'envoi continu via LoRa
    while (1)
    {
      loop_lora();
    }
  }
}
