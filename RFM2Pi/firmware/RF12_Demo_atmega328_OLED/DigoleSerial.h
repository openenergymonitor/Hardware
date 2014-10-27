//Digole Digital Solutions: www.digole.com
#ifndef DigoleSerialDisp_h
#define DigoleSerialDisp_h

#include <inttypes.h>
#include "Print.h"
#include "../Wire/Wire.h"
#include "Arduino.h"
#include <SoftwareSerial.h>
  
// Communication set up command
// Text function command
// Graph function command

#define Serial_UART 0;
#define Serial_I2C 1;
#define Serial_SPI 2;
#define _TEXT_ 0
#define _GRAPH_ 1


class DigoleSerialDisp : public Print {
public:
#if defined(_Digole_Serial_UART_)
  
DigoleSerialDisp(SoftwareSerial *s, int baud) //UART set up
    {
        _mySerial = s;
        _Baud = baud;
        _Comdelay=2;
    }


    size_t write(uint8_t value) {
        _mySerial->write(value);
        return 1; // assume sucess
    }

    void begin(void) {
        _mySerial->begin(9600);
        _mySerial->print("SB");
        _mySerial->println(_Baud);
        delay(100);
        _mySerial->begin(_Baud);
    }
#endif
#if defined(_Digole_Serial_I2C_)

void begin(void) {
        _myWire->begin();
    }

    DigoleSerialDisp(TwoWire *s, uint8_t add) //U2C set up
    {
        _myWire = s;
        _I2Caddress = add;
        _Comdelay=6;
    }

    size_t write(uint8_t value) {
        _myWire->beginTransmission(_I2Caddress);
        _myWire->write(value);
        _myWire->endTransmission();
        return 1; // assume sucess
    }
#endif
#if defined(_Digole_Serial_SPI_)

void begin(void) {
    }

    DigoleSerialDisp(uint8_t pin_data, uint8_t pin_clock, uint8_t SS) //spi set up
    {
        _Clockpin = pin_clock;
        _Datapin = pin_data;
        _SSpin = SS;
        pinMode(_Clockpin, OUTPUT);
        pinMode(_Datapin, OUTPUT);
        pinMode(_SSpin, OUTPUT);
        digitalWrite(_SSpin, HIGH);
        digitalWrite(_Clockpin, LOW);
        digitalWrite(_Datapin, LOW);
        _Comdelay=6;
    }

    size_t write(uint8_t value) {
        digitalWrite(_SSpin, LOW);
        digitalWrite(_SSpin, LOW);
        digitalWrite(_SSpin, LOW);
        shiftOut(_Datapin, _Clockpin, MSBFIRST, value);
        digitalWrite(_SSpin, HIGH);
        return 1; // assume sucess
    }
#endif
    //    virtual size_t write(uint8_t);
    //    void begin(void);

        /*---------fucntions for Text and Graphic LCD adapters---------*/
    void disableCursor(void) {
        Print::print("CS0");
    }

    void enableCursor(void) {
        Print::print("CS1");
    }

    void drawStr(uint8_t x, uint8_t y, const char *s) {
        Print::print("TP");
        write(x);
        write(y);
        Print::print("TT");
        Print::println(s);
    }

    void setPrintPos(uint8_t x, uint8_t y, uint8_t graph = _TEXT_) {
        if (graph == _TEXT_) {
            Print::print("TP");
            write(x);
            write(y);
        } else {
            Print::print("GP");
            write(x);
            write(y);
        }
    }

    void clearScreen(void) {
        Print::print("CL");
    }

    void setLCDColRow(uint8_t col, uint8_t row) {
        Print::print("STCR");
        write(col);
        write(row);
        Print::print("\x80\xC0\x94\xD4");
    }

    void setI2CAddress(uint8_t add) {
        Print::print("SI2CA");
        write(add);
        _I2Caddress = add;
    }

    void displayConfig(uint8_t v) {
        Print::print("DC");
        write(v);
    }
    //print function

    size_t println(const __FlashStringHelper *v) {
        preprint();
        Print::println(v);
    }

    size_t println(const String &v) {
        preprint();
        Print::println(v);
    }

    size_t println(const char v[]) {
        preprint();
        Print::println(v);
    }

    size_t println(char v) {
        preprint();
        Print::println(v);
    }

    size_t println(unsigned char v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t println(int v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t println(unsigned int v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t println(long v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t println(unsigned long v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t println(double v, int base = 2) {
        preprint();
        Print::println(v, base);
    }

    size_t println(const Printable& v) {
        preprint();
        Print::println(v);
    }

    size_t println(void) {
    }

    size_t print(const __FlashStringHelper *v) {
        preprint();
        Print::println(v);
    }

    size_t print(const String &v) {
        preprint();
        Print::println(v);
    }

    size_t print(const char v[]) {
        preprint();
        Print::println(v);
    }

    size_t print(char v) {
        preprint();
        Print::println(v);
    }

    size_t print(unsigned char v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t print(int v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t print(unsigned int v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t print(long v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t print(unsigned long v, int base = DEC) {
        preprint();
        Print::println(v, base);
    }

    size_t print(double v, int base = 2) {
        preprint();
        Print::println(v, base);
    }

    size_t print(const Printable& v) {
        preprint();
        Print::println(v);
    }
    void preprint(void);
    /*----------Functions for Graphic LCD/OLED adapters only---------*/
    //the functions in this section compatible with u8glib
    void drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap);
    void setRot90(void);
    void setRot180(void);
    void setRot270(void);
    void undoRotation(void);
    void setRotation(uint8_t);
    void setContrast(uint8_t);
    void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    void drawCircle(uint8_t x, uint8_t y, uint8_t r, uint8_t = 0);
    void drawDisc(uint8_t x, uint8_t y, uint8_t r);
    void drawFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    void drawPixel(uint8_t x, uint8_t y, uint8_t = 1);
    void drawLine(uint8_t x, uint8_t y, uint8_t x1, uint8_t y1);
    void drawLineTo(uint8_t x, uint8_t y);
    void drawHLine(uint8_t x, uint8_t y, uint8_t w);
    void drawVLine(uint8_t x, uint8_t y, uint8_t h);
    //-------------------------------
    //special functions for our adapters
    void setFont(uint8_t font); //set font, availale: 6,10,18,51,120,123, user font: 200-203
    void nextTextLine(void); //got to next text line, depending on the font size
    void setColor(uint8_t); //set color for graphic function
    void backLightOn(void); //Turn on back light
    void backLightOff(void); //Turn off back light
    void directCommand(uint8_t d); //send command to LCD drectly
    void directData(uint8_t d); //send data to LCD drectly
    void moveArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, char xoffset, char yoffset); //move a area of screen to another place

    void displayStartScreen(uint8_t m) {
        Print::print("DSS");
        write(m);
    } //display start screen

    void setMode(uint8_t m) {
        Print::print("DM");
        write(m);
    } //set display mode

    void setTextPosBack(void) {
        Print::print("ETB");
    } //set text position back to previous, only one back allowed

    void setTextPosOffset(char xoffset, char yoffset) {
        Print::print("ETO");
        write(xoffset);
        write(yoffset);
    }

    void setTextPosAbs(uint8_t x, uint8_t y) {
        Print::print("ETP");
        write(x);
        write(y);
    }
    void setLinePattern(uint8_t pattern) {
        Print::print("SLP");
        write(pattern);
     }
    void setLCDChip(uint8_t chip) {      //only for universal LCD adapter
        Print::print("SLCD");
        write(chip);
     }
    void uploadStartScreen(int lon, const unsigned char *data); //upload start screen
    void uploadUserFont(int lon, const unsigned char *data, uint8_t sect); //upload user font
    void digitalOutput(uint8_t x) {Print::print("DOUT");write(x);}
private:
    unsigned long _Baud;
    SoftwareSerial *_mySerial;
    uint8_t _I2Caddress;
    TwoWire *_myWire;
    uint8_t _Clockpin;
    uint8_t _Datapin;
    uint8_t _SSpin;
    uint8_t _Comdelay;
};

#endif
