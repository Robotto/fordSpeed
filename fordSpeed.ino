/*
fordSpeed.ino - a nixie tube speedometer for a Ford Model A.

Interprets bytes from serial as raw unsigned integer values.

Creates the proper binary pattern and shifts it out to a
HV5622 32bit high voltage shift register, which drives three Nixie Tubes

This works because the nixies are connected in a way that allows us to shift
a bit n-times per digit (with value n), to light up the corresponding value of that digit.

Each tube uses 10 bits, the HV5622 has 32.. so we've wasted two bits... sorry.

Copyleft 2022
~Robotto
*/
#include <SPI.h>


//Using hardware SPI pins:
static unsigned int HV_ClockPin = 15; //SCLK
static unsigned int HV_DataPin = 16;  //MOSI
static unsigned int HV_LatchPin = 14;
static unsigned int HV_BlankPin = 10;
static unsigned int HV_PolarityPin = A0;

static unsigned int DCDC_EnablePin = 9;


void setup()
{
    
    pinMode(DCDC_EnablePin,OUTPUT);
    digitalWrite(DCDC_EnablePin,LOW);
    
    Serial.begin(9600);
    Serial1.begin(9600);
    SPI.begin();

    pinMode(HV_ClockPin,OUTPUT);
    pinMode(HV_DataPin,OUTPUT);
    pinMode(HV_LatchPin,OUTPUT);
    pinMode(HV_BlankPin,OUTPUT);
    pinMode(HV_PolarityPin,OUTPUT);


    digitalWrite(HV_ClockPin,LOW);
    digitalWrite(HV_DataPin,LOW);
    digitalWrite(HV_LatchPin,HIGH);
    digitalWrite(HV_BlankPin,HIGH);
    digitalWrite(HV_PolarityPin,LOW); //On means sink current, off means high voltage on.


    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));

    delay(5000);

    digitalWrite(DCDC_EnablePin,HIGH); //youtu.be/R-FxmoVM7X4

}



void loop()
{
 uint8_t rxByte = 0;
 uint8_t rxOld = 255;

    if(Serial.available()) rxByte = Serial.read();
    if(Serial1.available()) rxByte = Serial.read();
 
    if(rxByte != rxOld){
        shiftOutBitPatternFromSpeed(rxByte);
        rxOld=rxByte;        
    }
}

void shiftOutBitPatternFromSpeed(uint8_t speed){
    uint8_t I,X,C; //ones, tens, hundreds
    uint16_t Iout, Xout, Cout; //for the properly patterened bits, ready for concat to uint32_t
    uint32_t speedOut;

    I = speed % 10;
    X = (speed / 10) % 10;
    C = (speed / 100) % 10; //mod10 is redundant here, because integer math..

    //shift a bit n times for each digit:
    Iout = (uint16_t)1<<I;
    Xout = (uint16_t)1<<X;
    Cout = (uint16_t)1<<C;

    speedOut = (uint32_t)Cout<<20 | (uint32_t)Xout<<10 | (uint32_t)Iout;

    Serial.print("Speed in: "); Serial.print(speed,DEC); Serial.print(". Cout: "); Serial.print(Cout,BIN); Serial.print(", Xout: "); Serial.print(Xout,BIN); Serial.print(", Iout: "); Serial.println(Iout,BIN);
    Serial.print("speedOut: "); Serial.println(speedOut,BIN);

/*
    //make 4 bytes from the uint32_t, because shiftout won't eat more than one byte,
    //so we have to feed it four times...
    //also used for testing:
    uint8_t frame[4]={
        0xff&(speedOut>>24), //MSB
        0xff&(speedOut>>16), 
        0xff&(speedOut>>8), 
        0xff&speedOut        //LSB
    };
*/

    digitalWrite(HV_LatchPin,LOW);
    //for(int i=0 ; i<4 ; i++){
        //shiftOut(HV_DataPin,HV_ClockPin,MSBFIRST,frame[i]);
    SPI.transfer(speedOut,4);
    //}
    digitalWrite(HV_LatchPin,HIGH);
}
