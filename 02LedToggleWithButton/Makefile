APPLICATION = press_button

BOARD ?= nucleo-f767zi

RIOTBASE ?= $(CURDIR)/../RIOT

DEVELHELP ?= 1

FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_gpio_irq

CFLAGS += -DAUTO_INIT_LED0

include $(RIOTBASE)/Makefile.include
