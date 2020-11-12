# name of your application
APPLICATION = LifiSink

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f767zi
#BOARD ?= nucleo-f401re

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP
CFLAGS=-DAUTO_INIT_LED0
CXXEXFLAGS += -std=c++11

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += saul_default
USEMODULE += printf_float
USEMODULE += xtimer
#features

FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_gpio_irq
FEATURES_REQUIRED += cpp
#RIOT base file
include $(RIOTBASE)/Makefile.include
