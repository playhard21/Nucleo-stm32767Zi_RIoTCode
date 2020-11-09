#include <stdio.h>
#include <string.h>
#include "board.h"
#include "arduino_board.h"
#include "arduino_pinmap.h"
#include "shell.h"
#include "led.h"
#include "periph_conf.h"
#include "periph/gpio.h"

static int led0_handler(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    LED0_TOGGLE;
    puts("led 0 is on");
    return 0;
}
static int led1_handler(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    LED1_TOGGLE;
    puts("led 1 is on");
    return 0;
}

static int led2_handler(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    LED2_TOGGLE;
    puts("led 2 is on");
    return 0;
}

static const shell_command_t shell_commands[] = {
        { "toggle0", "toggle led", led0_handler},
        { "toggle1", "toggle led", led1_handler},
        { "toggle2", "toggle led", led2_handler},
        { NULL, NULL, NULL }
};


int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
