/* ATtiny85 as an I2C Slave            BroHogan                           1/12/11
 * Example of ATtiny I2C slave receiving and sending data to an Arduino master.
 * Gets data from master, adds 10 to it and sends it back.
 * SETUP:
 * ATtiny Pin 4 = GND
 * ATtiny Pin 5 = I2C SDA on DS1621  & GPIO        ATtiny Pin 6 = (D1) to LED2
 * ATtiny Pin 7 = I2C SCK on DS1621  & GPIO        ATtiny Pin 8 = VCC (2.7-5.5V)
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 * Current Rx & Tx buffers set at 32 bytes - see usiTwiSlave.h
 * Credit and thanks to Don Blake for his usiTwiSlave code. 
 * More on TinyWireS usage - see TinyWireS.h
 */

#include <TinyDebugSerial.h>
#include "TinyWireS.h"                  // wrapper class for I2C slave routines

#define I2C_SLAVE_ADDR  0x26            // i2c slave address (38)
#define ir_out 4  // chip pin 3
#define TIMER_ENABLE_PWM     (GTCCR |= _BV(COM1B1))
#define TIMER_DISABLE_PWM    (GTCCR &= ~(_BV(COM1B1)))

uint8_t pwmval;
static unsigned long waitMicros;

TinyDebugSerial mySerial = TinyDebugSerial();

void mark() {
  waitMicros = micros();
  TIMER_ENABLE_PWM;
  waitMicros += 1200;
  while ((long)(micros() - waitMicros)<0){}
  TIMER_DISABLE_PWM;
  waitMicros += 600;
  while ((long)(micros() - waitMicros)<0){}
}

void space() {
  waitMicros = micros();
  TIMER_ENABLE_PWM;
  waitMicros += 600;
  while ((long)(micros() - waitMicros)<0){}
  TIMER_DISABLE_PWM;
  waitMicros += 600;
  while ((long)(micros() - waitMicros)<0){}
}

void irsend( uint16_t signal, uint8_t bits ) {
  // start transmission sequence
  uint32_t mask = 1 << bits-1;
  mySerial.print("Mask"); mySerial.println(mask,BIN);
  waitMicros = micros();
  TIMER_ENABLE_PWM;
  waitMicros += 2400;
  while ((long)(micros() - waitMicros)<0){}
  TIMER_DISABLE_PWM;
  waitMicros += 600;
  while ((long)(micros() - waitMicros)<0){}
  for (uint8_t i = 0; i < bits; i++) {
    if (signal & mask) {
      mark();
    }else{
      space();
    }
    signal = signal << 1;
  }
}


void ir_setup() {
// Timer/Counter 1
// PWM Frequency    Clock Slection    CS1[3:0]    OCR1C   RESOLUTION
//    40 kHz            PCK/8           0100       199        7.6

//                     f_TCK1
//        f_PWM = -----------------
//                   (OCR1C + 1
//

  pinMode(ir_out, OUTPUT);
  digitalWrite(ir_out, HIGH);
  GTCCR = 1<<PWM1B;                     //   Pulse Width Modulator B Enable
  GTCCR |= 2<<COM1B0;                   //   OC1B (Pin 4) Cleared on compare match - /OC1B Not connected
  TCCR1 = 1<<CS10;                      //   OC1A (Pin 0 and /OC1A (Pin 1) not connected
                                        //   Set CS1 3:0 to  0b0111 - Synchronous mode = CK/64           
  pwmval  = 200;                                        
  OCR1C = pwmval;                       //   Set frequency

  OCR1B = pwmval/3;                     //   duty cycle
  TIMER_DISABLE_PWM;
  delay(1000);
}

void setup(){
  mySerial.begin( 9600 );
  mySerial.println(F("Start Setup"));

  ir_setup();
  
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  mySerial.println(F("End Setup"));
}

uint8_t byteRcvd;
uint16_t intRcvd;
uint8_t msb;
uint8_t lsb;
boolean top_byte = true;

void loop(){
  if (TinyWireS.available()){             // got 1 bytes I2C input!
    byteRcvd = TinyWireS.receive();       // get the byte from master

    // check for reset byte
    if (byteRcvd == 0xff) {
      // clear the receive buffer
      mySerial.println("Reset command received"); 
      while (TinyWireS.available()) {
        byteRcvd = TinyWireS.receive();
        mySerial.print("cleared: "); mySerial.println(byteRcvd,HEX); 
      }
      top_byte = true;
      
    } else {

      if (top_byte) {
        msb = byteRcvd;
        top_byte = false;
        mySerial.print("msb: "); mySerial.println(msb,HEX); 
      } else {
        lsb = byteRcvd;
        top_byte = true;
        mySerial.print("lsb: "); mySerial.println(lsb,HEX); 
      }
      
      if (top_byte) {
        intRcvd = (msb << 8) | lsb;
        mySerial.print("Received: "); mySerial.println(intRcvd,HEX); 
        for (int i = 0; i < 2; i++) {       // send the ir code twice
          irsend(intRcvd, 12);
          delay(40);
        }
      }
          
      TinyWireS.send(msb);                // send msb back to master  
      TinyWireS.send(lsb);                // send lsb back to master  
      mySerial.println("Sent: ");
    }
  }
}

