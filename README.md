# Compte rendu TP LoRA 📡  

![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![Polytech](https://img.shields.io/badge/Polytech-0078D7?style=for-the-badge&logo=Polytech&logoColor=white)
![LoRa](https://img.shields.io/badge/LoRa-00BFFF?style=for-the-badge&logo=LoRa&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-660066?style=for-the-badge&logo=MQTT&logoColor=white)

## Auteurs ✍️  

**Groupe 1**  
- 📧 [romain.pipon@etu.univ-nantes.fr](mailto:romain.pipon@etu.univ-nantes.fr)  
- 📧 [leo.leclerc1@etu.univ-nantes.fr](mailto:leo.leclerc1@etu.univ-nantes.fr)  

**Groupe 2**  
- 📧 [yann-philippe-fokammbopda@etu.univ-nantes.fr](mailto:yann-philippe.fokam-mbopda@etu.univ-nantes.fr)  
- 📧 [tanguy.hue@etu.univ-nantes.fr](mailto:tanguy.hue@etu.univ-nantes.fr)  

---

## Installation sur Windows 🖥️  

```bash
$ pip3 install pyserial
```  

🔧 **Étape importante :** Installer le driver fourni dans le fichier ZIP du dossier Windows 🗂️ (*"CP210x_Windows_Drivers.zip"*) et ouvrir **CP210xVCPInstaller_x64.exe**.  

---

## Partie 1 🔍  

🚀 **Objectif :** Tester et valider le bon fonctionnement de la carte ESP32.  

Les fichiers correspondants sont disponibles dans le dossier `Partie 1`.  

### Test ESP32 🔧  

🛠️ Le script `Test.ino` permet :  
- **D'afficher un message sur l'écran** fourni avec l'ESP32.  

### Test WiFi 🌐  

📶 Le script `1-2 Wifi.ino` vérifie la connexion au module WiFi et affiche l'état de la connexion.  

### Test MQTT 📨  

🔗 **Protocole MQTT :**  
- Utilisation de la bibliothèque **ArduinoMqttClient**.  
- Le script `1-3 MQTT.ino` permet :  
  - **De se connecter à un serveur MQTT**.  
  - **D'envoyer un message sur un topic**.  
  - **D'afficher les messages reçus sur le port série**.  

---

## Partie 2 📡  

🚀 **Objectif :** Connecter deux cartes ESP32 via LoRa.  

Les fichiers correspondants se trouvent dans le dossier `Partie 2`.  

### Configuration LoRa ⚙️  

🔄 Les informations de connexion LoRa sont :  
- **Envoyées via MQTT**.  
- **Compressées dans une structure de données** pour optimiser leur envoi (3 octets).  

```c
struct packet {
  unsigned short freq : 8;  // Fréquence
  unsigned int sb : 12;     // Bande passante
  unsigned char sf : 4;     // Facteur d'étalement
} packet_data;
```  

> 🌟 Cette optimisation est basée sur les valeurs maximales des paramètres pour minimiser la taille des données. Des calculs sont effectués
pour obtenir les valeurs réelles.  

### Fonctionnement global 🔄  

📋 Le code fusionne **Wi-Fi, MQTT et LoRa** pour transmettre et recevoir des données.  

#### Étapes principales :  
1. **Wi-Fi et MQTT**  
   - Connexion au réseau Wi-Fi avec les identifiants fournis.  
   - Connexion au broker MQTT (test.mosquitto.org) sur le port 1883.  
   - Abonnement au topic `CR7`.  
   - Publication des données sérialisées représentant la structure `packet`.  

2. **Réception MQTT et configuration LoRa**  
   - **Désérialisation des messages MQTT** pour extraire les paramètres LoRa (freq, sb, sf).  
   - **Configuration du module LoRa** avec ces paramètres.  

3. **Transmission LoRa**  
   - Une fonction `loop_lora` envoie périodiquement un paquet LoRa contenant des données flottantes (`sdp.data`).  

### Fichiers associés 📂  

- **`receiver.ino` :**  
  - Reçoit les informations de connexion LoRa via MQTT.  
- **`sender.ino` :**  
  - Envoie les informations de connexion LoRa.  

💡 **Résumé :**  
Ce projet combine la communication **MQTT (réseau)** et **LoRa (radio)** pour piloter dynamiquement des transmissions via LoRa grâce aux messages reçus via MQTT. 🎯  

--- 

✨ Avec cette approche, le système est optimisé pour des échanges rapides et efficaces entre les deux technologies. 🌐📡