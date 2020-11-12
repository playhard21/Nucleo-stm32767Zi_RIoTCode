#include <stdio.h>
#include <string.h>
/* RIOT APIs */
#include "arduino_board.h"
#include "arduino_pinmap.h"
#include "periph/adc.h"
#include "xtimer.h"
#include "timex.h"
#include "shell.h"
#include "led.h"
#include "periph_conf.h"
#include "periph/gpio.h"


static const shell_command_t commands[] = {
        { NULL, NULL, NULL }
};

int main(void)
{
    //portD8 sending

    //adc port init
    int adcPortinit;
    adcPortinit = adc_init(0);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    printf("this is adcPort return value\n %d",adcPortinit);
    if(adcPortinit == 0){
        puts("\nadc port initilised");
    }else{
        puts("\nadc failed to init");
    }
    for(uint32_t i=0 ; i < 300 ; i++ ){
        int val = adc_sample(0, ADC_RES_10BIT);
        if(val < 0) {
            puts("[!] sampling failure\n");
            return -1;
        }else{
            puts("done sampling");
        }
    }
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
