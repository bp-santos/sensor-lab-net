#define GREEN 5
#define RED 4
#define BLUE 3
#define YELLOW 2
#define NONE 0

int LEDs[] = {RED, GREEN, BLUE, YELLOW, NONE};
bool stop = false;

void setup()
{
  Serial.begin(115200);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(YELLOW, OUTPUT);
}

void loop()
{
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

    if (Serial.available())
    {
      String message = Serial.readString();
      stop = message.toInt();
      Serial.println(stop);
    }
    while (stop)
    {
      if (Serial.available())
      {
        String message = Serial.readString();
        stop = message.toInt();
        Serial.println(stop);
      }
      delay(100);
    }

    digitalWrite(LEDs[i], LOW);
  }
}