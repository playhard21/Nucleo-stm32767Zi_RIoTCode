#include <stdio.h>
#include <string.h>

#include "led.h"
#include "PLiFi.h"
#include "thread.h"
#include "periph/adc.h"

static const uint32_t DEVICE_ID_POS = 0;
uint8_t device_id = 0001;
uint8_t control_info []={0x7e};
uint8_t sink1[]={0xd4};
static char tx_stack1[THREAD_STACKSIZE_MAIN];
static char rx_stack1[THREAD_STACKSIZE_MAIN];
static void on_rx(PLiFi *lifi, const uint8_t *msg, uint8_t len);
static void control_message(const uint8_t *msg, uint8_t len);
static void on_message_for_me(const uint8_t *msg, uint8_t len);
//pin declaration
gpio_t pinD8 = GPIO_PIN(PORT_D,8);
PLiFi lifi1(pinD8, 0, on_rx, NULL);
//transmit loop from module
void * tx_loop(void *arg) {
    PLiFi *lifi = (PLiFi *)arg;

    lifi->tx_loop();
    return NULL;
}
//receive loop from module
void * rx_loop(void *arg) {
    PLiFi *lifi = (PLiFi *)arg;
    lifi->rx_loop();
    return NULL;
}
//on receive
static void on_rx(PLiFi *lifi, const uint8_t *msg, uint8_t len)
{
    if (len >= 2) {
        if (msg[0] == device_id) {
            on_message_for_me(msg + 2, len - 2);
        }
        else if (msg[0]==control_info[0])
        {
            control_message(msg+1,len-1);
        }
        else {
            if (lifi != &lifi1) {
                lifi1.send_data(msg, len);
            }
            if (lifi != &lifi2) {
                lifi2.send_data(msg, len);
            }
        }
    }
}

static void control_message(const uint8_t *msg, uint8_t len)
{
    if (len > 0) {
        Serial.print("Got: 0x");
        for (uint8_t i = 0; i < len - 1; i++) {
            Serial.print(msg[i], HEX);
            Serial.print(", 0x");
        }
        Serial.println(msg[len-1], HEX);
        control_data[0]=msg[len-1];
        if (control_data[0]==sink1[0])
        {
            state =1;
        }
        if (control_data[0]==sink2[0])
        {
            state =2;
        }
        if (control_data[0]==sink3[0])
        {
            state =3;
        }
    }
    else {
        Serial.println("Got: Empty message");
    }
}

static void on_message_for_me(const uint8_t *msg, uint8_t len)
{
    if (len > 0) {
        Serial.print("Got: 0x");
        for (uint8_t i = 0; i < len - 1; i++) {
            Serial.print(msg[i], HEX);
            Serial.print(", 0x");
        }
        Serial.println(msg[len-1], HEX);
    }
    else {
        Serial.println("Got: Empty message");
    }
}

int main(void)
{
    device_id = eeprom_read_byte(DEVICE_ID_POS);
    Serial.print("Device ID: 0x");
    Serial.println(device_id, HEX);

    thread_create(tx_stack1, sizeof(tx_stack1), THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, tx_loop, &lifi1, "tx1");
    thread_create(rx_stack1, sizeof(rx_stack1), THREAD_PRIORITY_MAIN - 2,
                  THREAD_CREATE_STACKTEST, rx_loop, &lifi1, "rx1");
    delay(1);

    thread_create(tx_stack2, sizeof(tx_stack2), THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST, tx_loop, &lifi2, "tx2");
    thread_create(rx_stack2, sizeof(rx_stack2), THREAD_PRIORITY_MAIN - 2,
                  THREAD_CREATE_STACKTEST, rx_loop, &lifi2, "rx2");
    delay(1);
    pinMode(SEN1, INPUT_PULLUP);
    pinMode(SEN2,INPUT_PULLUP);
    pinMode(forward,OUTPUT);
    pinMode(reverse,OUTPUT);
    digitalWrite(forward,LOW);
    digitalWrite(reverse,LOW);

    Serial.println("setup() done");

    while(1){
        uint8_t msg[2];
        if(state==1)
        {
            digitalWrite(forward,HIGH);
            sensorValue=digitalRead(SEN1);
            Serial.println(sensorValue);
            if(sensorValue==1)
            {
                msg[0]=control_info[0];
                msg[1]=sink1[0];
                if(lifi2.send_data(msg,sizeof(msg))){
                    delay(10);
                }
                delay(5000);
                digitalWrite(forward,LOW);
                state=0;
            }
        }
        if(state==2)
        {
            digitalWrite(forward,HIGH);
            sensorValue=digitalRead(SEN1);
            if(sensorValue==1)
            {
                msg[0]=control_info[0];
                msg[1]=sink2[0];
                if(lifi2.send_data(msg,sizeof(msg))){
                    delay(10);}
                delay(5000);
                digitalWrite(forward,LOW);
                state=0;
            }
        }
        if(state==3)
        {
            digitalWrite(forward,HIGH);
            sensorValue=digitalRead(SEN1);
            if(sensorValue==1)
            {
                msg[0]=control_info[0];
                msg[1]=sink3[0];
                if(lifi2.send_data(msg,sizeof(msg))){
                    delay(10);}
                delay(5000);
                digitalWrite(forward,LOW);
                state=0;
            }
        }
        else
        {
            //Serial.println("");
        }
    }
    return 0;
}
