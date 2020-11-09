#include <string.h>
//#include <Scheduler.h>
#include "PLiFi.h"
#include "periph/adc.h"
#include "periph_conf.h"
#include "periph/gpio.h"
//#define ENABLE_DEBUG
#include "debugLifi.h"

//Transmitter functions
//declaration of transmitter pins
PLiFi::PLiFi(int sPin, adc_t rPin,
             void (*rx_cb)(PLiFi *, const uint8_t *, uint8_t),
             void (*tx_cb)(PLiFi *lifi)) : lifi(sPin, rPin)
{
    rx_callback = rx_cb;
    tx_callback = tx_cb;
    obuf_len = 0;
    mutex_init(&mutex);
}

int PLiFi::send_data(const uint8_t *data, uint8_t data_len) {
    if (data_len > sizeof(obuf)) {
        DEBUGLN("send_data(): Called with too huge message");
        return -1;
    }

    mutex_lock(&mutex);
    if (obuf_len != 0) {
        mutex_unlock(&mutex);
        DEBUGLN("send_data(): TX is still ongoing, refusing to send next frame");
        return -1;
    }

    memcpy(obuf, data, data_len);
    obuf_len = data_len;
    mutex_unlock(&mutex);
    return 0;
}

void PLiFi::tx_loop() {
    while (1) {
        mutex_lock(&mutex);
        uint8_t len = obuf_len;
        mutex_unlock(&mutex);
        if (len > 0) {
            DEBUGLN("PLiFi::tx_loop(): TX starting");
            lifi.send_data(obuf, len);

            mutex_lock(&mutex);
            obuf_len = 0;
            mutex_unlock(&mutex);
            if (tx_callback) {
                tx_callback(this);
                DEBUGLN("PLiFi::tx_loop(): TX completed");
            }
        }
        else {
            delay(100);
        }
    }
}

void PLiFi::rx_loop() {
    while (1) {
        uint8_t ibuf[64];
        int ibuf_len = lifi.receive(ibuf, sizeof(ibuf));
        if ((ibuf_len >= 0) && (rx_callback)) {
            rx_callback(this, ibuf, ibuf_len);
        }
    }
}
