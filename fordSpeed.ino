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
//filament order from back to front and back again.
const uint8_t order[18] = {3, 4, 8, 7, 5, 9, 6, 2, 0, 1, 0, 2, 6, 9, 5, 7, 8, 4};



//Using hardware SPI pins:
static unsigned int HV_ClockPin = 15; //SCLK - yellow wire - HV pin 28 
static unsigned int HV_DataPin = 16;  //MOSI - green wire - HV pin 32
static unsigned int HV_LatchPin = 14; //Blue wire - HV pin 31
static unsigned int HV_BlankPin = 10; //White wire - HV pin 33
//static unsigned int HV_PolarityPin = A0;

static unsigned int DCDC_EnablePin = 9;


void setup()
{
    
    Serial.begin(9600);
    Serial1.begin(9600);
    pinMode(DCDC_EnablePin,OUTPUT);
    digitalWrite(DCDC_EnablePin,LOW);

    SPI.begin();

    pinMode(HV_ClockPin,OUTPUT);
    pinMode(HV_DataPin,OUTPUT);
    pinMode(HV_LatchPin,OUTPUT);
    pinMode(HV_BlankPin,OUTPUT);
//    pinMode(HV_PolarityPin,OUTPUT);


    digitalWrite(HV_ClockPin,LOW);
    digitalWrite(HV_DataPin,LOW);
    digitalWrite(HV_LatchPin,LOW);
    digitalWrite(HV_BlankPin,HIGH); 
 //   digitalWrite(HV_PolarityPin,HIGH); //LOW: enabled pins source current, HIGH: Enabled pins sink current


    SPI.beginTransaction(SPISettings(200000, MSBFIRST, SPI_MODE2));

    delay(3000);
    digitalWrite(DCDC_EnablePin,HIGH); //youtu.be/R-FxmoVM7X4


    prettyAnimation(3);
    
    for(int i=333;i>0;i=i-111)
    {
      shiftOutBitPatternFromSpeed(i);
      delay(500);
    }
}


uint8_t rxByte = 0;
uint8_t rxOld = 250;

void loop()
{
    if(Serial.available()) rxByte = Serial.read();
    if(Serial1.available()) rxByte = Serial1.read();
 
    if(rxByte != rxOld){
      if(rxByte==255)     {
        digitalWrite(HV_BlankPin,LOW);  //Blank the outputs, and turn off DCDC
        digitalWrite(DCDC_EnablePin,LOW);
      }
      else     
      {
        digitalWrite(HV_BlankPin,HIGH); 
        digitalWrite(DCDC_EnablePin,HIGH);
      }
      shiftOutBitPatternFromSpeed(rxByte);
      rxOld=rxByte;        
        
      while(Serial.available()) Serial.read();
      while(Serial1.available()) Serial1.read();
      
    }

}

void shiftOutBitPatternFromSpeed(uint32_t speed){
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

//    Serial.print("Speed in: "); Serial.println(speed,DEC); 
//    Serial.print("Cout: "); Serial.print(Cout,BIN); Serial.print(", Xout: "); Serial.print(Xout,BIN); Serial.print(", Iout: "); Serial.println(Iout,BIN);


    /*
    make 4 bytes from the uint32_t, because SPI transfer won't eat more than one byte,
    so we have to feed it four times...
    also used for testing:
    */
    uint8_t frame[4]={
        0xff&(speedOut>>24), //MSB
        0xff&(speedOut>>16), 
        0xff&(speedOut>>8), 
        0xff&speedOut        //LSB
    };

//    Serial.print("speedOut: "); // Serial.println(speedOut,BIN);

    digitalWrite(HV_LatchPin,LOW);
    SPI.transfer(&frame,4);
/*
    for(int i=0 ; i<4 ; i++){
        SPI.transfer(frame[i]);
        for(int j=8-String(frame[i],BIN).length();j>0;j--) Serial.print("0"); //pad with zeroes up to 8 bits per byte.
        Serial.print(frame[i],BIN); Serial.print(" ");
    }
    Serial.println();
*/
    digitalWrite(HV_LatchPin,HIGH);
   
}


void prettyAnimation(int n) 
{
 
  uint8_t pointer, C, X, I;

  while(n>0){

      for(pointer=0;pointer<18;pointer++){ //Roll up
       
        shiftOutBitPatternFromSpeed(roll(pointer,C,X,I));
              
       delay(75);
      }
      
  n--;
  }


}

uint32_t roll(uint8_t pointer,uint8_t C,uint8_t X,uint8_t I){
    uint32_t out=    order[pointer]*100 + order[(pointer+1)%18]*10 + order[(pointer+2)%18];
    Serial.println(out);
    return out;
}
