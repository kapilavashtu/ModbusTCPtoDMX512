#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>

#include "lib_dmx.h"
#include "defines.h"

#define    DMX512     (0)    // (250 kbaud - 2 от 512 каналов) Стандарт USITT DMX-512
#define    DMX1024    (1)    // (500 kbaud - 2 от 1024 каналов) Не стандарт, проверка.
#define    DMX2048    (2)    // (1000 kbaud - 2 от 2048 каналов) called by manufacturers DMX1000K

#define DMX_CHANNEL_COUNT   48
#define HOLDING_REGS_COUNT  64
#define RESET_REGISTER      50
#define START_PULSE_REG     51

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 1, 177);

EthernetServer ethServer(502);

ModbusTCPServer modbusTCPServer;

const int ledPin = LED_BUILTIN;

uint8_t holdingRegisters[HOLDING_REGS_COUNT];

uint8_t pulse1ChannelPull[DMX_CHANNEL_COUNT];
uint8_t pulse2ChannelPull[DMX_CHANNEL_COUNT];
uint8_t pulse3ChannelPull[DMX_CHANNEL_COUNT];
uint8_t pulse4ChannelPull[DMX_CHANNEL_COUNT];
uint8_t pulse5ChannelPull[DMX_CHANNEL_COUNT];

void setup() 
{
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  ArduinoDmx3.set_tx_address(0);  // put here start address
  ArduinoDmx3.set_tx_channels(DMX_CHANNEL_COUNT);  // put here the number of channels to transmmit
  ArduinoDmx3.init_tx(DMX512);  // starts universe 0 as TX, standard mode DMX512

  memset(holdingRegisters, 0, sizeof(holdingRegisters));

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  Serial.begin(19200);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) 
  {
    while (true) 
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  if (Ethernet.linkStatus() == LinkOFF) 
  {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  ethServer.begin();
  
  // start the Modbus TCP server
  if (!modbusTCPServer.begin(10)) 
  {
    Serial.println("Failed to start Modbus TCP Server!");
    while (1);
  }

  // configure the LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // configure a 100 holding regs from 0 address
  modbusTCPServer.configureHoldingRegisters(0x00, HOLDING_REGS_COUNT);
}

//timers values
uint32_t current_millis = 0;
uint32_t previous_millis = 0;
uint32_t interval_delay = 1000;
uint8_t click1 = 0;

//===================================================================================
void loop() 
{
  // listen for incoming clients
  EthernetClient client = ethServer.available();
  
  if (client) 
  {
    Serial.println("new client");
    // let the Modbus TCP accept the connection 
    modbusTCPServer.accept(client);

    while (client.connected()) 
    {
      // poll for Modbus TCP requests, while client connected
      modbusTCPServer.poll();
      updateLED();
    }

    Serial.println("client disconnected");
  }
}
//===================================================================================
void updateLED() 
{
  //step 1 - save from regs to dmx slots
  for(int i = 0; i < DMX_CHANNEL_COUNT; i++)
  {
    uint8_t rx_val = (uint8_t) modbusTCPServer.holdingRegisterRead(i);
    holdingRegisters[i] = rx_val;
    ArduinoDmx3.TxBuffer[i] = rx_val;
    //String message = "channel " + String(i) +" value = " + String(rx_val);
    //Serial.println(message);
  }
  //step 2
  parsePulse();

}

void setChannel(int channel, int value)
{
  ArduinoDmx3.TxBuffer[channel] = value;
}

void parsePulse(void)
{
  uint16_t bits = (uint16_t) modbusTCPServer.holdingRegisterRead(START_PULSE_REG);
  print16(bits);

  uint16_t pulseState = 0;

  //get type of command(pulseOn or pulseOff)
  if(convertPulseState(bits) & SET_PULSE_ON)
  {
    pulseState = 1;
  }else if(convertPulseState(bits) & SET_PULSE_OFF)
  {
    pulseState = 0;
  }

  //get pulse num
  uint16_t channel = convertPulseChannel(bits);
  uint8_t pulse_num = convertPulseNum(bits);

  Serial.println("pulse state = " + String(pulseState));
  Serial.println("pulse num = " + String(pulse_num));
  Serial.println("pulse channel num = " + String(channel));
  print16(bits);
}

void print16(uint16_t val)
{
  String message = "";
  for(int i = 15; i >= 0; i--)
  {
    if(BIT_IS_SET(val, i, uint16_t))
    {
      message += "1";
    } else
    {
        message += "0";
    } 
    
  }

  Serial.println(message);
}

//pulse functions
void pulseAddChannelToPull(uint8_t channel_num, uint8_t pulse_type)
{
   switch(pulse_type)
  {
    case 1:
    pulse1ChannelPull[channel_num] = 1;
    break;

    case 2:
    pulse2ChannelPull[channel_num] = 1;
    break;

    case 3:
    pulse3ChannelPull[channel_num] = 1;
    break;

    case 4:
    pulse4ChannelPull[channel_num] = 1;
    break;

    case 5:
    pulse5ChannelPull[channel_num] = 1;
    break;
  };
  
}

void pulseRemoveChannelToPull(uint8_t channel_num, uint8_t pulse_type)
{
  switch(pulse_type)
  {
    case 1:
    pulse1ChannelPull[channel_num] = 0;
    break;

    case 2:
    pulse2ChannelPull[channel_num] = 0;
    break;

    case 3:
    pulse3ChannelPull[channel_num] = 0;
    break;

    case 4:
    pulse4ChannelPull[channel_num] = 0;
    break;

    case 5:
    pulse5ChannelPull[channel_num] = 0;
    break;
  };
}

void setPulse(uint8_t channel, uint8_t pulse_type)
{
  switch(pulse_type)
  {
    case 1:
    break;

    case 2:
    break;

    case 3:
    break;

    case 4:
    break;

    case 5:
    break;
  };
}

void pulse1()
{
  current_millis = millis();
  if((current_millis - previous_millis) >= interval_delay)
  {
    previous_millis = current_millis;
    if(click1)
    {
      click1 = !click1;
      //do something
    }
  }
}

uint16_t convertPulseState(uint16_t val)
{
    return (val >> 12) & 0b000000000001111;
}

uint16_t convertPulseNum(uint16_t val)
{
    return (val >> 8) & 0b000000000001111;
}

uint16_t convertPulseChannel(uint16_t val)
{
  return val & 0b000000011111111;
}











