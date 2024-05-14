#ifndef NODE_H
#define NODE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

const int RF24_PA_LEVEL = RF24_PA_HIGH;
const int RADIO_CE_PIN = 7;
const int RADIO_CSN_PIN = 8;
const int NAME_LENGTH = 7;
const unsigned long INIT_DELAY = 2000;
const unsigned long PAYLOAD_SEND_DELAY = 100;

const char READINGS_REQUEST = 'R';
const char KEEP_ALIVE = 'P';

struct Sensor_Node
{
    char name[NAME_LENGTH];
    int16_t temperature;
    int16_t phototransistor;
};

class Node
{
public:
    /// @brief  Sends a payload to a specific node.
    /// @tparam T The type of the payload.
    /// @param to The node ID to send the payload to.
    /// @param type The type of the payload.
    /// @param payload The payload to send.
    /// @return True if the message is sent successfully, false otherwise.
    template <typename T>
    bool sendPayload(uint16_t to, char type, const T &payload)
    {
        network.update(); // keep the network updated
        RF24NetworkHeader header(to, type);
        delay(PAYLOAD_SEND_DELAY); // ensure reliable connectivity
        bool ok = network.write(header, &payload, sizeof(payload));
        Serial.print(ok ? F(" (status = 1)") : F(" (status = 0)"));
        return ok;
    }

    /// @brief Logs a message to the serial monitor.
    /// @tparam ...Args This is a variadic template that accepts any number of arguments.
    /// @param ...args This is a parameter pack that accepts any number of arguments.
    template <typename... Args>
    void log(Args... args)
    {
        Serial.println();
        Serial.print(millis());
        (Serial.print(args), ...);
    }

protected:
    RF24Network network;
    uint16_t _node;

    Node(int channel, uint16_t node);
    void setupRF24Network();

private:
    RF24 radio;
    int _channel;

    virtual void init() = 0;
    virtual void receivePayload() = 0;
};

#endif