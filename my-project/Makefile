PROJECT = usb_check
BUILD_DIR = bin
WRITE_ADDR = 0x8000000

SHARED_DIR = ../my-common-code
CFILES = cdcacm.c led_lib.c timer.c my-project.c
CFILES += 
AFILES +=

# TODO - you will need to edit these two lines!
DEVICE=stm32f407vgt6
OOCD_FILE = board/stm32f4discovery.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
