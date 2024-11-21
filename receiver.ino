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


char ssid[] = "";  // SSID
char pass[] = ""; // password

// Gestion des valeurs reçues
struct packet
{
    unsigned short freq : 8;
    unsigned int sb : 12;
    unsigned char sf : 4;
} packet_data;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Informations MQTT
const char broker[] = "test.mosquitto.org";
int port = 1883;
const char topic[] = "CR7";

void setup()
{
    Serial.begin(9600);

    // Connexion au réseau WiFi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        // failed, retry
        Serial.print(".");
        delay(2000);
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
            ;
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();

    // Callback pour la réception des messages
    mqttClient.onMessage(onMqttMessage);

    // Souscription à un topic
    Serial.print("Subscribing to topic: ");
    Serial.println(topic);
    Serial.println();

    mqttClient.subscribe(topic);
}

void loop()
{
    // call poll() regularly to allow the library to receive MQTT messages and
    // send MQTT keep alive which avoids being disconnected by the broker
    mqttClient.poll();
}

// Gestion de la réception des messages LoRa
void onReceive(int packetSize)
{
    Serial.println("Packet LoRa received");
    int packetLen;
    packetLen = LoRa.parsePacket();
    char rdp[2];
    int i = 0;
    if (packetLen == 16)
    {
        while (LoRa.available())
        {
            rdp[i] = LoRa.read();
            i++;
        }
        Serial.println("Value on LoRa received !");
        Serial.println(rdp[0]);
        Serial.println(rdp[1]);
        Serial.println();
    }
}

// Gestion de la réception des messages MQTT
void onMqttMessage(int messageSize)
{
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    while (mqttClient.available())
    {
        byte serializedData[3];

        for (int i = 0; i < 3; i++)
        {
            serializedData[i] = mqttClient.read();
        }

        // Désérialisation des données dans la structure packet_data
        packet_data.freq = serializedData[0];                                          // Premier octet contient freq
        packet_data.sb = (serializedData[1] << 4) | ((serializedData[2] >> 4) & 0x0F); // Combinaison des bits pour sb
        packet_data.sf = serializedData[2] & 0x0F;                                     // Les 4 derniers bits de serializedData[2] contiennent sf

        Serial.println(packet_data.sb);
        Serial.println(packet_data.freq);
        Serial.println(packet_data.sf);

        // Configuration des paramètres LoRa
        Serial.println("LoRa");
        pinMode(DI0, INPUT);
        SPI.begin(SCK, MISO, MOSI, SS);
        LoRa.setPins(SS, RST, DI0);
        if (!LoRa.begin(((packet_data.freq + 868) * 1E6)))
        {
            Serial.println("KO");
            while (1)
                ;
        }
        else
        {
            Serial.println("OK");
        }
        LoRa.setSpreadingFactor(packet_data.sf + 7);
        LoRa.setSignalBandwidth((packet_data.sb + 50) * 1E3);
        LoRa.onReceive(onReceive);
        LoRa.receive();
    }
    Serial.println();
    Serial.println();
}
