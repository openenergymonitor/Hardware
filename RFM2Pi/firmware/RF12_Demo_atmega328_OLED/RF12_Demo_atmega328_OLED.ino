// RF12Demo for Attiny84_RFM12b
// Configure some values in EEPROM for easy config of the RF12 later on.
// 2009-05-06 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 6); // RX, TX, RX is set to some dummy pin

//http://www.digole.com/images/file/Tech_Data/Digole_Serial_Display_Adapter-Manual.pdf
#define _Digole_Serial_UART_  //To tell compiler compile the special communication only, 
#include "DigoleSerial.h"
DigoleSerialDisp mydisp(&mySerial, 19200); //UART:Arduino UNO: Pin 1(TX)on arduino to RX on module

#define LCDCol 16
#define LCDRow 2
#define LCDW 127
#define LCDH 63

const unsigned char fonts[] = {6, 10, 18, 51, 120, 123};
const char *fontdir[] = {"0\xb0", "90\xb0", "180\xb0", "270\xb0"};


#include "RF12mods.h"
#include <util/crc16.h>
#include <util/parity.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define LED_PIN     9

#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks

static word bands[4] = { 315, 433, 868, 915 };


static unsigned long now () {
    // FIXME 49-day overflow
    return millis() / 1000;
}

static void activityLed (byte on) {

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, on);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// RF12 configuration setup code

typedef struct {
    byte nodeId;
    byte group;
    byte lock;
//    char msg[30-4];
    word crc;
} RF12Config;

static RF12Config config;

static char cmd;
static byte value, stack[20], top, sendLen, dest, quiet;
static byte testbuf[20], testCounter;


static void saveConfig () {
    // save to EEPROM

    eeprom_write_byte(RF12_EEPROM_ADDR ,config.nodeId);
    eeprom_write_byte(RF12_EEPROM_ADDR +1 ,config.group);
    eeprom_write_byte(RF12_EEPROM_ADDR +2 ,config.lock);
    
    config.crc = ~0;
    config.crc = _crc16_update(config.crc, config.nodeId);     
    config.crc = _crc16_update(config.crc, config.group);        
    config.crc = _crc16_update(config.crc, config.lock);        
        
    for (int i=3; i < RF12_EEPROM_SIZE-2; i++) {
        eeprom_write_byte(RF12_EEPROM_ADDR + i, 0);
        config.crc = _crc16_update(config.crc, 0);        
    }

    eeprom_write_byte(RF12_EEPROM_ADDR + RF12_EEPROM_SIZE-2 ,config.crc);
    eeprom_write_byte(RF12_EEPROM_ADDR + RF12_EEPROM_SIZE-1 ,config.crc>>8);
    
    if (!rf12_config())
        Serial.print(F("config failed"));

}

static void showHelp () {
    Serial.println(F(
        "\n"
    "Available commands:" "\n"
    "  123 x      - Toggle configuration change protection, 1=Unlocked" "\r\n"
    "  <nn> i     - set node ID (standard node ids are 1..26)" "\r\n"
    "  <n> b      - set MHz band (4 = 433, 8 = 868, 9 = 915)" "\r\n"
    "  <nnn> g    - set network group (0 = any)" "\r\n"
    "  <n> c      - set collect mode (advanced, normally 0)" "\r\n"
    "  ...,<nn> a - send data packet to node <nn>, with ack" "\r\n"
    "  ...,<nn> s - send data packet to node <nn>, no ack" "\r\n"
    "  <n> l      - turn activity LED on DIG8 on or off" "\r\n"
    
    ));
    Serial.print(F("Current configuration:\n"));
    config.nodeId = eeprom_read_byte(RF12_EEPROM_ADDR);
    config.group = eeprom_read_byte(RF12_EEPROM_ADDR + 1);
    config.lock = eeprom_read_byte(RF12_EEPROM_ADDR + 2);
    
/*
    Serial.println("EEPROM config: ");        
    uint16_t crc = ~0;
    for (uint8_t i = 0; i < RF12_EEPROM_SIZE; ++i){
        crc = _crc16_update(crc, eeprom_read_byte(RF12_EEPROM_ADDR + i));
       Serial.print(eeprom_read_byte(RF12_EEPROM_ADDR + i),HEX);
    }
    Serial.println("");        
    
    Serial.print("crc:");        
    Serial.print(crc);        
    Serial.print(" ");            
 */
 
    byte id = config.nodeId & 0x1F;
    Serial.print('@' + id,DEC);
    Serial.print(F(" i"));
    Serial.print( id,DEC);
    if (config.nodeId & COLLECT)
         Serial.print("*");
    
    Serial.print(F(" g"));
    Serial.print(config.group,DEC);
    
    Serial.print(F(" @ "));
    word band = config.nodeId >> 6;
    Serial.print(bands[band],DEC);
    Serial.print(F(" MHz "));

    Serial.print(F(" Lock: "));
    Serial.println(config.lock, DEC);

    rf12_config();
}

static void handleInput (char c) {
    if ('0' <= c && c <= '9')
        value = 10 * value + c - '0';
    else if (c == ',') {
        if (top < sizeof stack)
            stack[top++] = value;
        value = 0;
    } else if ('a' <= c && c <='z') {
        Serial.print(F("> "));
        Serial.print((int) value);
        Serial.println(c);
        switch (c) {
            default:
                showHelp();
                break;

            case 'x': // set node id
                if(value==123) {
                  config.lock = !(config.lock);
                  saveConfig();
                }
                break;             

            case 'i': // set node id
                if(config.lock) {
                  config.nodeId = (config.nodeId & 0xE0) + (value & 0x1F);
                  saveConfig();
                }
                break;             
            case 'b': // set band: 4 = 433, 8 = 868, 9 = 915
                if(config.lock) {
                  value = value == 8 ? RF12_868MHZ :
                          value == 9 ? RF12_915MHZ : RF12_433MHZ;
                  config.nodeId = (value << 6) + (config.nodeId & 0x3F);
                  saveConfig();
                }
                break;             
            case 'g': // set network group
                if(config.lock) {                            
                  config.group = value;
                  saveConfig();
                }
                break;
            case 'c': // set collect mode (off = 0, on = 1)
                if(config.lock) {
                if (value)
                    config.nodeId |= COLLECT;
                else
                    config.nodeId &= ~COLLECT;
                saveConfig();
                }
                break;
            case 'a': // send packet to node ID N, request an ack
            case 's': // send packet to node ID N, no ack
                cmd = c;
                sendLen = top;
                dest = value;
                memcpy(testbuf, stack, top);
                break;
            case 'l': // turn activity LED on or off
                activityLed(value);
                break;
            case 'q': // turn quiet mode on or off (don't report bad packets)
                quiet = value;
                break;
        }
        value = top = 0;
        memset(stack, 0, sizeof stack);
    } else if (c > ' ')
        showHelp();
}

void setup() {

    activityLed(1); // Show some signs of life
    delay(3000); // Wait a bit before initializing the OLED
            
    Serial.begin(9600);
    Serial.print(F("\n[RFM2Pi]\n"));   
    
    if (rf12_config()) {
        config.nodeId = eeprom_read_byte(RF12_EEPROM_ADDR);
        config.group = eeprom_read_byte(RF12_EEPROM_ADDR + 1);
        config.lock = eeprom_read_byte(RF12_EEPROM_ADDR + 2);
    } else {
        config.nodeId = 0x81; // node A1 @ 868 MHz
        config.group = 0xD2;  //210
        config.lock=1;   //Unlocked
        rf12_initialize(config.nodeId&0x1F, config.nodeId >> 6 ,config.group);  
        saveConfig();
    }
    
    rf12_control(0xC049);      

    mydisp.begin();
    
    mydisp.clearScreen(); //CLear screen    
    mydisp.setRot180();  
    
    mydisp.setFont(10); 
    mydisp.print(F("RFM2Pi board config:"));
    mydisp.nextTextLine();
    
    mydisp.print(F("Node ID:"));    mydisp.println(config.nodeId&0x1F); mydisp.print(' ');
    mydisp.nextTextLine();
    
    mydisp.print(F("Group:"));    mydisp.println(config.group); mydisp.print(' ');
    mydisp.nextTextLine();
    
    mydisp.print(F("Band:"));    
    word band = config.nodeId >> 6;
    mydisp.print(bands[band],DEC);
    mydisp.println(F(" MHz "));
    
    showHelp();

    delay(2000);
    
    activityLed(0);
}

void loop() {
    if (Serial.available())
        handleInput(Serial.read());

    if (rf12_recvDone() && (rf12_crc == 0) ) {

        byte n = rf12_len;
        byte acked=0;
        uint8_t rssi = rf12_getRSSI();
        
        activityLed(1);            
                            
        if (RF12_WANTS_ACK && (config.nodeId & COLLECT) == 0) {
           byte i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}  // if ready to send 
           rf12_sendStart(RF12_ACK_REPLY, 0, 0);
           rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
           acked=1;
        }            
        
        if (config.group == 0) {
            Serial.print("G ");
            Serial.print((int) rf12_grp);
        }
        Serial.print(' ');
        Serial.print((int) rf12_hdr & 0x1F);
        for (byte i = 0; i < n; ++i) {
            Serial.print(' ');
            Serial.print((int) rf12_data[i]);
        }
        Serial.println();
        if(acked)  Serial.print(F(" -> ack\n"));
        
        mydisp.setPrintPos(0, 5, _TEXT_);
        mydisp.print(F("From:")); mydisp.print((int) rf12_hdr & 0x1F); 
        
        mydisp.print(" RSSI:");
        if (B1000 & rssi) {
            mydisp.print(B111 & rssi); // 0 (very poor reception), 2, 4, 6 (very good reception)
        } else { // packet was too short
            mydisp.print(rssi); // 1 (bad: 0 or 2), 5 (good: 4 or 6)
          }
          
        mydisp.print(F(" ACK:")); mydisp.print(acked);
        
        activityLed(0);
    }

    if (cmd && rf12_canSend()) {
        activityLed(1);

        Serial.print(F(" -> "));
        Serial.print((int) sendLen);
        Serial.println(" b");
        byte header = cmd == 'a' ? RF12_HDR_ACK : 0;
        if (dest)
            header |= RF12_HDR_DST | dest;
        rf12_sendStart(header, testbuf, sendLen);
        cmd = 0;
        activityLed(0);
    }
}
