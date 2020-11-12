# name of your application
APPLICATION = ReadData

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f767zi

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP
CFLAGS=-DAUTO_INIT_LED0

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += saul_default
USEMODULE += xtimer

FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_gpio_irq

include $(RIOTBASE)/Makefile.include
