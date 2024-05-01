#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

char *ssid = "NOS-0856";  // Does not work on 5Ghz networks
char *wifiPassword = "FG94RWP5"; 
char *server = "192.168.1.28";
short port = 1883;
char *topic = "Lab 1";  // Change it to your student ID

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  setupWiFi(ssid, wifiPassword);
  client.setServer(server, port);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}

void setupWiFi(char *ssid, char *wifiPassword)
{
  Serial.println("Attempting WiFi connection...");
  WiFi.begin(ssid, wifiPassword);       // Attempt to connect
  while (WiFi.status() != WL_CONNECTED) // Loop until we're reconnected
  {
    Serial.print(F("WiFi connection failed, rc="));
    Serial.print(WiFi.status());
    Serial.println(F(" try again in 5 seconds"));

    delay(5000); // Wait 5 seconds before retrying
  }
}

void reconnect()
{
  while (!client.connected()) // Loop until we're reconnected
  {
    Serial.println("Attempting MQTT connection...");
    if (!client.connect("arduinoClient")) // Attempt to connect
    {
      Serial.print(F("MQTT connection failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));

      delay(5000); // Wait 5 seconds before retrying
    }
  }
  client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}