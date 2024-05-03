#include <WiFi.h>
#include <PubSubClient.h>

#define GREEN 5
#define RED 4
#define BLUE 3
#define YELLOW 2
#define NONE 0

WiFiClient espClient;
PubSubClient client(espClient);

char *ssid = "NOS-E914"; // Does not work on 5Ghz networks
char *wifiPassword = "FUACPFMP";
char *server = "192.168.1.125";
short port = 1883;
char *topic = "Lab 1"; // Change it to your student ID

bool stop = false;

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
  setupLEDs();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  loopLEDs();
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

void setupLEDs()
{
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);
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

  String message;
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.println(message);
  stop = message.toInt();
}

void loopLEDs()
{
  int LEDs[] = {RED, GREEN, BLUE, YELLOW, NONE};

  for (int i = 0; i <= 4; i++)
  {
    if (LEDs[i] != NONE)
    {
      digitalWrite(LEDs[i], HIGH);
      delay(1000);
    }
    else
    {
      delay(1000);
    }

    client.loop();
    while (stop)
    {
      client.loop();
      delay(100);
    }

    digitalWrite(LEDs[i], LOW);
  }
}