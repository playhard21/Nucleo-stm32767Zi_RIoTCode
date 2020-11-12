#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "led.h"
#include "periph_conf.h"
#include "periph/gpio.h"


static const shell_command_t commands[] = {
        { NULL, NULL, NULL }
};

//step1 : init the pin and read data from the pin
//pin PG0

int main(void)
{
    //Initilize th GPIO pin G0
    //gpio_init(GPIO_PIN(G,0),GPIO_IN);
    gpio_t pinG1 = GPIO_PIN(PORT_G,0);
    int isValid = (int) gpio_is_valid	(pinG1);
    if(isValid){
        gpio_init(pinG1,GPIO_IN);
    }
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    while(1){
        uint8_t pinStatus = gpio_read (pinG1);
        printf("This is pin status %u",pinStatus);
        if(pinStatus){
            LED0_TOGGLE;
            LED1_TOGGLE;
            LED2_TOGGLE;
        }
    }
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
