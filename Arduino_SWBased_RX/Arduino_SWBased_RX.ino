/*

Radio    Arduino
CE    -> 9
CSN   -> 10 (Hardware SPI SS)
MOSI  -> 11 (Hardware SPI MOSI)
MISO  -> 12 (Hardware SPI MISO)
SCK   -> 13 (Hardware SPI SCK)
IRQ   -> No connection
VCC   -> No more than 3.6 volts
GND   -> GND

*/

#include <SPI.h>
#include <NRFLite.h>

const static uint8_t RADIO_ID = 0;
const static uint8_t DESTINATION_RADIO_ID = 1;
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;
uint32_t _lastSendTime; // Needs to be uint32 to hold millis as it grows larger than uint16.
uint32_t _lastReceivedTime;

enum RadioPacketType
{
    Trigger,
    Response
};

struct RadioPacket // Must be 32 bytes or less.
{
    RadioPacketType PacketType; // 2 bytes
    uint8_t FromRadioId;        // 1 byte
    char Message[29];           // 29 bytes and only a 28 character string can be sent since
                                // the 29th character needs to be a string termination character.
};

NRFLite _radio;

void setup()
{
    Serial.begin(115200);

    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 75)) // 1MBPS did not work consistently.
    {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here forever.
    }
}

void loop()
{
    uint32_t currentMillis = millis(); // Save some time by only calling millis() once.

    // Send Data.
    if (currentMillis - _lastSendTime > 999)
    {
        _lastSendTime = currentMillis;
        sendMessage("Sending Data from RX Software");

        // Now that the send is complete, switch the radio back into RX mode so that it is listening for packets.
        _radio.startRx();
    }

    // Show any received data.
    if (currentMillis - _lastReceivedTime > 999)
    {
        _lastReceivedTime = currentMillis;

        while (_radio.hasData())
        {
            RadioPacket radioData;
            _radio.readData(&radioData);

            if (radioData.PacketType == Trigger)
            {
                String msg = String(radioData.Message);

                Serial.print(currentMillis);
                Serial.print(" Received '");
                Serial.print(msg);
                Serial.print("' from radio ");
                Serial.println(radioData.FromRadioId);
            }
        }
    }
}

void sendMessage(String msg)
{
    RadioPacket messageData;
    messageData.PacketType = Response;
    messageData.FromRadioId = RADIO_ID;

    bool messageTooLargeForPacket = msg.length() > sizeof(messageData.Message) - 1;
    if (messageTooLargeForPacket)
    {
        msg = msg.substring(0, sizeof(messageData.Message) - 1);
    }

    msg.getBytes((unsigned char*)messageData.Message, msg.length() + 1);
    _radio.send(DESTINATION_RADIO_ID, &messageData, sizeof(messageData));
}