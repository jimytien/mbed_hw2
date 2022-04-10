#include "mbed.h"
#include "uLCD_4DGL.h"

uLCD_4DGL uLCD(D1, D0, D2);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
InterruptIn button_A(PB_4);//pb_4 is D5
InterruptIn button_B(PB_1);//pb_1 is D6
EventQueue queuestore(32 * EVENTS_EVENT_SIZE);
EventQueue queuelcd(32 * EVENTS_EVENT_SIZE);
AnalogOut aout(PA_4);
AnalogIn ain(A0);

Thread transfer;
Thread lcd;
Thread gen_thread;
Thread store_thread;

//
float T = 1000.0/10.0; //ms
float grow = ( 10.0 / 11.0 ) / (0.2 * T); // V/ms
float wither = ( 10.0 / 11.0 ) / (0.1 * T);// V/ms
float current_T = 0.0;
float ADCdata[130];
int i = 1;
//

bool Genwave=false;
 
void gen()
{
    while(1){
        if(Genwave){
            ThisThread::sleep_for(2ms);
            current_T += 2.0;
            if(current_T > T)
                current_T = 0.0;
            
            if(current_T <= 0.2 * T)
                aout = aout + grow * 2;
            else if (current_T <= 0.9 * T) 
                aout = 10.0 / 11.0;
            else
                aout = aout - wither * 2;
        }
        else{
            aout = 0;
            ThisThread::sleep_for(2ms);
        }
    }
}
 
void store()
{
    while(1){
        if(Genwave){
            if(i >= 128)
                i = 1;
            ADCdata[i] = ain;
            i++;
            ThisThread::sleep_for(1000ms/128);
        }
        else{
            ThisThread::sleep_for(1000ms/128);
        }
    }
}
void print_transfer()
{
    uLCD.cls();
    uLCD.printf("Data transfer\n");    
}
void print_start()
{
    uLCD.cls();
    uLCD.printf("Starting program\n");
}
void print_A()
{
    uLCD.cls();
    uLCD.printf("Button A detected\n");
}
void print_B()
{
    uLCD.cls();
    uLCD.printf("Button B detected\n");
}

void output()
{
    ThisThread::sleep_for(2000ms);
    queuelcd.call(print_transfer);
    ThisThread::sleep_for(5000ms);
    for (int i = 0; i < 128; i++){
        printf("%f\r\n", ADCdata[i]);
        ThisThread::sleep_for(100ms);
    }
}
 
void A(void)
{
Genwave=true;
queuelcd.call(print_A);
}
 
void B(void)
{
Genwave=false;
queuestore.call(output);
queuelcd.call(print_B);
}
 
int main()
{
    // Start the event queue
    transfer.start(callback(&queuestore, &EventQueue::dispatch_forever));
    lcd.start(callback(&queuelcd, &EventQueue::dispatch_forever));
    gen_thread.start(gen);
    store_thread.start(store);
    button_A.rise(A);
    button_B.rise(B);
    queuelcd.call(print_start);
}