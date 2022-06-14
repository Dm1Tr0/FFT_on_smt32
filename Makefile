# This version of rules.mk expects the following to be defined before
# inclusion..
### REQUIRED ###
# OPENCM3_DIR - duh
# PROJECT - will be the basename of the output elf, eg usb-gadget0-stm32f4disco
# CFILES - basenames only, eg main.c blah.c
# CXXFILES - same for C++ files. Must have cxx suffix!
# DEVICE - the full device name, eg stm32f405ret6
#  _or_
# LDSCRIPT - full path, eg ../../examples/stm32/f4/stm32f4-discovery/stm32f4-discovery.ld
# OPENCM3_LIB - the basename, eg: opencm3_stm32f4
# OPENCM3_DEFS - the target define eg: -DSTM32F4
# ARCH_FLAGS - eg, -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
#    (ie, the full set of cpu arch flags, _none_ are defined in this file)
#
### OPTIONAL ###
# INCLUDES - fully formed -I paths, if you want extra, eg -I../shared
# BUILD_DIR - defaults to bin, should set this if you are building multiarch
# OPT - full -O flag, defaults to -Os
# CSTD - defaults -std=c99
# CXXSTD - no default.
# OOCD_INTERFACE - eg stlink-v2
# OOCD_TARGET - eg stm32f4x
#    both only used if you use the "make flash" target.
# OOCD_FILE - eg my.openocd.cfg
#    This overrides interface/target above, and is used as just -f FILE
### TODO/FIXME/notes ###
# No support for stylecheck.
# No support for BMP/texane/random flash methods, no plans either
# No support for magically finding the library.
# C++ hasn't been actually tested with this..... sorry bout that. ;)
# Second expansion/secondary not set, add this if you need them.

PROJECT = high freq_timer
WRITE_ADDR = 0x8000000

PROFILE ?= debug
SRC_DIR ?= ./my-project
PROFILE_DIR ?= $(SRC_DIR)/$(PROFILE)
BUILD_DIR = $(SRC_DIR)/$(PROFILE)/bin

SHARED_DIR = ./my-common-code
CFILES = timer.c led_lib.c my-project.c cdcacm.c adc_lib.c
CFILES += 
AFILES +=

LIBS += -lm
LIBS += -lopencm3

LIB_OBJ = libopencm3.a
LIB_OBJ +=

LIB_LOCATION = $(PROFILE_DIR)/lib_bin

LIBOPENCM3_TARGET ?= stm32/f4

ifeq ($(PROFILE),debug)

ENABLE_SEMIHOSTING ?= 1

ifeq ($(ENABLE_SEMIHOSTING),1)
S_LDFLAGS   += --specs=rdimon.specs -lrdimon
S_DEFS		+= -DENABLE_SEMIHOSTING=1 
else 
S_LDFLAGS += -lnosys
endif
endif

OOCD ?= openocd -f ./discovery_conf.cfg

# TODO - you will need to edit these two lines!
DEVICE=stm32f407vgt6
OOCD_FILE = discovery_conf.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR= ./libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk

ifeq ($(PROFILE),debug)
OPT ?= -Og 
else 
OPT ?= -Os
endif 

CSTD ?= -std=c99

# Be silent per default, but 'make V=1' will show all compiler calls.
# If you're insane, V=99 will print out all sorts of things.
V?=0
ifeq ($(V),0)
Q	:= @
NULL	:= 2>/dev/null
endif

# Tool paths.
PREFIX	?= arm-none-eabi-
CC	= $(PREFIX)gcc
CXX	= $(PREFIX)g++
LD	= $(PREFIX)gcc
GDB = $(PREFIX)gdb
OBJCOPY	= $(PREFIX)objcopy
OBJDUMP	= $(PREFIX)objdump
OOCD	?= openocd

OPENCM3_INC = $(OPENCM3_DIR)/include

# Inclusion of library header files
INCLUDES += $(patsubst %,-I%, . $(OPENCM3_INC) )

OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)
OBJS += $(CXXFILES:%.cxx=$(BUILD_DIR)/%.o)
OBJS += $(AFILES:%.S=$(BUILD_DIR)/%.o)
GENERATED_BINS = $(PROJECT).elf $(PROJECT).bin $(PROJECT).map $(PROJECT).list $(PROJECT).lss

FPU ?= hard
FPU_FLAGS := -mfpu=fpv4-sp-d16 -mfloat-abi=$(FPU)
ARCH_FLAGS := -mcpu=cortex-m4 -mthumb $(FPU_FLAGS)

TGT_CPPFLAGS += -MD
TGT_CPPFLAGS += -Wall -Wundef $(INCLUDES)
TGT_CPPFLAGS += $(INCLUDES) $(OPENCM3_DEFS)

TGT_CFLAGS += $(OPT) $(CSTD) -ggdb3
TGT_CFLAGS += $(ARCH_FLAGS)
TGT_CFLAGS += -fno-common
TGT_CFLAGS += -ffunction-sections -fdata-sections
TGT_CFLAGS += -Wextra -Wshadow -Wno-unused-variable -Wimplicit-function-declaration
TGT_CFLAGS += -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes
TGT_CFLAGS += $(S_DEFS) 

TGT_CXXFLAGS += $(OPT) $(CXXSTD) -ggdb3
TGT_CXXFLAGS += $(ARCH_FLAGS)
TGT_CXXFLAGS += -fno-common
TGT_CXXFLAGS += -ffunction-sections -fdata-sections
TGT_CXXFLAGS += -Wextra -Wshadow -Wredundant-decls  -Weffc++

TGT_ASFLAGS += $(OPT) $(ARCH_FLAGS) -ggdb3

TGT_LDFLAGS += -T$(LDSCRIPT) -L$(LIB_LOCATION) -nostartfiles
TGT_LDFLAGS += $(ARCH_FLAGS)
TGT_LDFLAGS += -specs=nano.specs
TGT_LDFLAGS += -Wl,--gc-sections
TGT_LDFLAGS += -static 
# OPTIONAL
#TGT_LDFLAGS += -Wl,-Map=$(PROJECT).map
ifeq ($(V),99)
TGT_LDFLAGS += -Wl,--print-gc-sections
endif

# Linker script generator fills this in for us.
ifeq (,$(DEVICE))
endif
# nosys is only in newer gcc-arm-embedded...
#LDLIBS += -specs=nosys.specs
LDLIBS += -Wl,--start-group -lc -lgcc $(S_LDFLAGS) -Wl,--end-group
LDLIBS += $(S_LDLIBS)
LDLIBS += $(LIBS)

# Burn in legacy hell fortran modula pascal yacc idontevenwat
.SUFFIXES:
.SUFFIXES: .c .S .h .o .cxx .elf .bin .list .lss

# Bad make, never *ever* try to get a file out of source control by yourself.
%: %,v
%: RCS/%,v
%: RCS/%
%: s.%
%: SCCS/s.%

$(BUILD_DIR):
	mkdir -p $@

$(LIB_LOCATION):
	mkdir -p $@

all: $(PROFILE_DIR)/$(PROJECT).elf $(PROFILE_DIR)/$(PROJECT).bin
#flash: $(PROJECT).flash

# error if not using linker script generator
ifeq (,$(DEVICE))
$(LDSCRIPT):
ifeq (,$(wildcard $(LDSCRIPT)))
    $(error Unable to find specified linker script: $(LDSCRIPT))
endif
else
# if linker script generator was used, make sure it's cleaned.
GENERATED_BINS += $(LDSCRIPT)
endif

$(LIB_LOCATION)/libopencm3.a: | $(LIB_LOCATION) $(OPENCM3_DIR)/Makefile
	@echo Building libopencm3 for $(PROFILE) profile...
	cd $(OPENCM3_DIR) && $(MAKE) $(MAKEFLAGS) TARGETS="$(LIBOPENCM3_TARGET)" FP_FLAGS="$(FPU_FLAGS)" CFLAGS="$(TGT_CFLAGS)" V=1 clean lib
	cp $(OPENCM3_DIR)/lib/libopencm3_$(subst /,,$(LIBOPENCM3_TARGET)).a $@
	@echo libopencm3 for $(PROFILE) profile is built

include $(OPENCM3_DIR)/mk/genlink-rules.mk

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(LIB_LOCATION)/libopencm3.a
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx | $(LIB_LOCATION)/libopencm3.a
	@printf "  CXX\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S | $(LIB_LOCATION)/libopencm3.a
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_ASFLAGS) $(ASFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(PROFILE_DIR)/$(PROJECT).elf: $(OBJS) $(LDSCRIPT) $(LIBDEPS) | $(LIB_LOCATION)/libopencm3.a 
	@printf "  LD\t$@\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.bin: %.elf
	@printf "  OBJCOPY\t$@\n"
	$(Q)$(OBJCOPY) -O binary  $< $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.list: %.elf
	$(OBJDUMP) -S $< > $@

flash: $(PROFILE_DIR)/$(PROJECT).elf
	$(OOCD) -c "program $< verify reset exit"

st_util_server:
	st-util --semihosting &

# to make it work just tipe load and continue
# after continue you may watch the resaults of semihosting prints in :tt
# for some reason can't make the semihosting prints to apeare in the GDB cmd line :(
gdb: $(PROFILE_DIR)/$(PROJECT).elf flash st_util_server
	$(GDB) -ex 'target extended-remote localhost:4242' $<  

st_flash: all
	sudo st-flash --reset write $(PROFILE_DIR)/$(PROJECT).bin $(WRITE_ADDR)

flash_erase:
	sudo st-flash erase 

clean_bins: 
	rm -rf $(GENERATED_BINS)

clean_obj:
	rm -rf $(BUILD_DIR)/*.o

clean: clean_obj clean_bins

mr_proper: clean
	rm -rf  $(LIB_LOCATION)

.PHONY: all clean flash st_flash flash_erase st_util_server
-include $(OBJS:.o=.d)

