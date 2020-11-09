#include <stdio.h>
#include <string.h>
/* RIOT APIs */
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
    if(adcPortinit){
        printf("adc port initilised");
    }else{
        printf("adc failed to init");
    }
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
