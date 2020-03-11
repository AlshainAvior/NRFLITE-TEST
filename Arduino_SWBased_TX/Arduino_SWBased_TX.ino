/*

  Demonstrates two-way communication without using acknowledgement data packets.  This is much slower
  than the hardware-based, ACK packet approach shown in the 'TwoWayCom_HardwareBased' example, but is
  more flexible.

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

const static uint8_t RADIO_ID = 1;
const static uint8_t DESTINATION_RADIO_ID = 0;
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;
uint16_t _lastSendTime;
uint16_t _lastReceiveTime;

// already tried using packet type but not working
//enum RadioPacketType
//{
//  Trigger,
//  Response
//};

struct RadioPacket
{
//  RadioPacketType PacketType;
  uint8_t FromRadioId;
  char Message[31];
};


NRFLite _radio;
//RadioPacket _radioData;

void setup()
{
  Serial.begin(115200);

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN,  NRFLite::BITRATE1MBPS, 75))
  {
    Serial.println("Cannot communicate with radio");
    while (1); // Wait here forever.
  }
}

void loop()
{
  //Send Data;
  if (millis() - _lastSendTime > 999)
  {
    _lastSendTime = millis();
    sendMessage("Sending Data from TX Software");
  }

  // Show any received data.
  if (millis() - _lastReceiveTime > 3999)
  {
    _lastReceiveTime = millis();

    while (_radio.hasData())
    {
      RadioPacket radioData;
      _radio.readData(&radioData);
//      if (radioData.PacketType == Response) { // already try this to both TX ( if equal to "Response") and RX ( if equal to "Trigger") but still not working
      String msg = String(radioData.Message);

      Serial.print("Received '");
      Serial.print(msg);
      Serial.print("' from radio ");
      Serial.println(radioData.FromRadioId);
      delay(10);
//      }
    }
  }

}

void sendMessage(String msg)
{
  RadioPacket messageData;
//  messageData.PacketType = Trigger;
  messageData.FromRadioId = RADIO_ID;

  // Ensure the message is not too large for the MessagePacket.
  if (msg.length() > sizeof(messageData.Message) - 1)
  {
    msg = msg.substring(0, sizeof(messageData.Message) - 1);
  }

  msg.getBytes((unsigned char*)messageData.Message, msg.length() + 1);
  _radio.send(DESTINATION_RADIO_ID, &messageData, sizeof(messageData));
}
