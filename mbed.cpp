#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"
#include <string>
#include <list>
#include <mpr121.h>

RawSerial  pc(USBTX, USBRX);
RawSerial  dev(p13,p14);
 
uLCD_4DGL uLCD(p9,p10,p11); // serial tx, serial rx, reset pin;

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
Thread thread;
Thread thread2;
Thread threadu1;
Thread threadu2;

SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
AnalogOut DACout(p18);
wave_player waver(&DACout);

//InterruptIn interrupt(p26);

//Mutex stdio_mutex;
Mutex lederp;

Semaphore two_slots(1);
//Semaphore three_slots(3);

PwmOut R(p21);
PwmOut G(p22);
PwmOut B(p23);

bool Go = true;
bool Bo = false;
char mode = '0';

void eRequest()
{
    //G = 1;
    while(1) {
        if(dev.getc() == '9')
            if(dev.getc() == '1')
                if(dev.getc() == '1'){
                    G = 1;
                    Bo = true;
                    dev.printf("EMP enroute\n");
                    return;}
                else 
                  continue;
            else
              continue;
        else
          continue;
                
    }
}   
    
void red_Circle()
{
    while(Go){
        //stdio_mutex.lock();
        //uLCD.cls();
        two_slots.wait();
        if(mode%2 == 0){
            uLCD.filled_rectangle(18,32,64,64,RED);
            uLCD.filled_rectangle(64,32,110,64,BLACK); 
            uLCD.rectangle(64,32,110,64,BLUE); }
        else{
            uLCD.filled_rectangle(18,32,64,64,GREEN);
            uLCD.filled_rectangle(64,32,110,64,BLACK);
            uLCD.rectangle(64,32,110,64,0xffff00); }
         if(mode == '5'){
            uLCD.cls();
            uLCD.printf("ALL EMERGENCY RESPONSE UNITS ARE IN ROUTE\n");   }
        if(mode == '6'){
            uLCD.cls();
            uLCD.printf("BACKUP REQUESTED\n");}
        if(mode == '7'){
            uLCD.cls();
            uLCD.printf("MY LORD WHAT DID YOU BURN??\n");}
        //stdio_mutex.unlock();
        Thread::wait(900);
        two_slots.release();
        }
        
}

void blue_Circle()
{
    while(Go){
        //stdio_mutex.lock();
        //uLCD.cls();
        two_slots.wait();
        if(mode%2 == 0){
            uLCD.filled_rectangle(18,32,64,64,BLACK);
            uLCD.rectangle(18,32,64,64,RED);
            uLCD.filled_rectangle(64,32,110,64,BLUE);}
        else{
            uLCD.filled_rectangle(18,32,64,64,BLACK);
            uLCD.rectangle(18,32,64,64,GREEN);
            uLCD.filled_rectangle(64,32,110,64,0xffff00);}
            
        if(mode == '5'){
            uLCD.cls();
            uLCD.printf("ALL EMERGENCY RESPONSE UNITS ARE IN ROUTE\n");}
        if(mode == '6'){
            uLCD.cls();
            uLCD.printf("BACKUP REQUESTED\n");}
        if(mode == '7'){
            uLCD.cls();
            uLCD.printf("MY LORD WHAT DID YOU BURN??\n");}
        //stdio_mutex.unlock();
        Thread::wait(1800);
        two_slots.release();
        }
}   
 
void led2_thread() {
   while (Go) {
      if(dev.readable()){
        lederp.lock();   
        if (dev.getc()=='!') 
            if (dev.getc()=='B') //button data
                mode = dev.getc(); //button number to set mode
        lederp.unlock(); }
    switch (mode){
        case '1':
            led1 = !led1;
            led2 = 0;
            led3 = 0;
            led4 = !led4;
            break;
        case '2':
            led1 = 0;
            led2 = !led2;
            led3 = !led3;
            led4 = 0;
            break;
        case '3':
            led1 = 0;
            led2 = !led2;
            led3 = 0;
            led4 = !led4;
            break;
        case '4':
            led1 = !led1;
            led2 = 0;
            led3 = !led3;
            led4 = 0;
            break;
    }
        
    Thread::wait(50);
  }
}

void kRGB() {
   while (Go){
       R = !R;
       G = 0;
       B = !B;
       
    switch (mode){
       case '2':
          G = !B;
          B = !G;
          R = 0;
          break;
       case '3':
          G = !R;
          R = !G;
          B = 0;
          break;
       case '4':
          R = !R;
          G = !G;
          B = !B;
          break;
       }
       Thread::wait(1000);
    }
}

//Block of code

void fallInterrupt() {
    int key_code=0;
    int i=0;
    int value=mpr121.read(0x00);
    value +=mpr121.read(0x01)<<8;
    // LED demo mod by J. Hamblen
    //pc.printf("MPR value: %x \r\n", value);
    i=0;
    // puts key number out to LEDs for demo
    for (i=0; i<12; i++) {
        if (((value>>i)&0x01)==1) key_code=i+1;
        }
    led4=key_code & 0x01;
    led3=(key_code>>1) & 0x01;
    led2=(key_code>>2) & 0x01;
    led1=(key_code>>3) & 0x01;
    
    if (led2 && led3){
        mode = '5';} 
    if (led1 && led4){
        mode = '6';}
    if (led2 && led4){
        mode = '7';}
}

 
int main() {
    uLCD.cls();
    pc.baud(9600);
    dev.baud(9600);
    while(!Bo){
       eRequest();
       G = 0;
    }
    R = 0;
    B = 1;
    srand(time(NULL));
    thread.start(led2_thread);
    thread2.start(kRGB);
    threadu1.start(red_Circle);
    threadu2.start(blue_Circle);
    FILE *wave_file;
    
    // Block of code. lol gone
  while (Go) {
    wave_file=fopen("/sd/siren.wav","r");
    waver.play(wave_file);
    Thread::wait(50);
    fclose(wave_file);
        
   } 
}
