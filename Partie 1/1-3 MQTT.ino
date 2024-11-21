#include <ArduinoMqttClient.h>
#include <WiFi.h>

// Informations de connexion WiFi
char ssid[] = ""; // Nom du réseau WiFi
char pass[] = ""; // Mot de passe du réseau WiFi

// Déclaration des objets WiFi et MQTT
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Informations sur le broker MQTT
const char broker[] = "test.mosquitto.org"; // Adresse du broker MQTT
int port = 1883;                            // Port du broker MQTT
const char topic[] = "tp_popo/alban";       // Topic utilisé pour la communication

void setup()
{
    // Initialisation de la communication série
    Serial.begin(9600);
    while (!Serial)
    {
        ; // Attente de l'ouverture du port série (nécessaire pour certains ports USB natifs)
    }

    // Configuration et connexion au réseau WiFi
    WiFi.mode(WIFI_STA); // Mode station (connexion à un réseau existant)
    WiFi.disconnect();   // Déconnexion préalable
    delay(100);          // Pause pour la stabilité
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass); // Démarrage de la connexion WiFi
    while (WiFi.status() != WL_CONNECTED)
    {
        // En cas d'échec, réessayer
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
            ; // Boucle infinie en cas d'échec de connexion
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();

    // Configuration du callback pour la réception des messages MQTT
    mqttClient.onMessage(onMqttMessage);

    // Souscription au topic défini
    Serial.print("Subscribing to topic: ");
    Serial.println(topic);
    Serial.println();
    mqttClient.subscribe(topic); // Souscription

    // Envoi d'un message au topic défini
    mqttClient.beginMessage(topic); // Démarrage de l'envoi du message
    mqttClient.print("coucou !");   // Contenu du message
    mqttClient.endMessage();        // Fin de l'envoi du message

    // Exemple pour désabonnement d'un topic :
    // mqttClient.unsubscribe(topic);

    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.println();
}

void loop()
{
    // Appel régulier à poll() pour gérer les messages MQTT et maintenir la connexion
    mqttClient.poll();
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

    // Lecture et affichage du contenu du message
    while (mqttClient.available())
    {
        Serial.print((char)mqttClient.read()); // Conversion et affichage des caractères reçus
    }
    Serial.println();
    Serial.println();
}
