#include <WiFi.h>

char ssid[] = "";
char pass[] = "";
int status = WL_IDLE_STATUS;

void setup()
{
    Serial.begin(9600);

    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true)
            ;
    }

    WiFi.begin(ssid, pass);

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to WEP network, SSID: ");
        Serial.println(ssid);
        status = WiFi.status();
        Serial.println(status);
        delay(2000);
    }

    Serial.print("You're connected to the network");
}

void loop()
{
    delay(10000);
    Serial.println(WiFi.status());
}