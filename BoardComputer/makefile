# --- Configuration -----------------------------------------------------------
PYTHON_VER := $(shell python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")

# Directories
IDIR := src/
TDIR := test/
BIN_DIR := bin/

# Interface directories
X86_STANDALONE_IDIR := $(IDIR)Interface/x86_Standalone/
X86_TEST_IDIR := $(IDIR)Interface/x86_Test/
HW_1_AVR_IDIR := $(IDIR)Interface/HW_1_AVR/

# Flags
TESTDEFINES 	:= -DTESTUSE=
STANDALONEFLAGS	:= -O1 -g $(TESTDEFINES)
TESTFLAGS   	:= -O1 -fpic -g -D__DEBUG__ $(TESTDEFINES)
AVRFLAGS    	:= -O1 -mmcu=atmega324pb -DF_CPU=8000000 -fshort-enums $(TESTDEFINES)

# Common sources
COMMON_SRCS := USART.c input.c nextion.c timer.c sensorsfeed.c countersfeed.c utils.c \
               programdata.c average.c system.c config.c
UI_SRCS     := UI/init.c UI/board.c UI/numpad.c UI/config.c

# Tools
CC_X86 := gcc
CC_AVR := avr-gcc

# Colors and printing
GREEN := \033[0;32m
RESET := \033[0m
PRINTF = @printf "$(GREEN)CC:$(RESET) %s\n" "$1"

# --- Targets -----------------------------------------------------------------

.PHONY: all clean flash x86_test HW_1 prepare_dirs

all: prepare_dirs HW_1 flash

# Create required directories once
prepare_dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BIN_DIR)UI
	@mkdir -p $(BIN_DIR)Interface

# -------------------------- x86 Test Build -----------------------------------

x86_test: clean prepare_dirs
	cp -R $(IDIR) $(TDIR)
	mkdir $(TDIR)/generatedDefinitions
	python3 testsuite/fileswrapper.py $(TDIR) test/generatedDefinitions
	@echo "Using Python $(PYTHON_VER)"
	$(MAKE) $(BIN_DIR)testmodule.so

# List all .o objects for x86 Test
X86_SRCS := $(addprefix $(TDIR), $(COMMON_SRCS) $(UI_SRCS)) \
            $(TDIR)main.c
X86_IFACE_SRCS := $(X86_TEST_IDIR)persistent_memory.c $(X86_TEST_IDIR)system_interface.c $(X86_TEST_IDIR)serial.c

X86_OBJS := $(patsubst $(TDIR)%.c,$(BIN_DIR)%_.o,$(X86_SRCS)) \
             $(patsubst $(X86_TEST_IDIR)%.c,$(BIN_DIR)%_.o,$(X86_IFACE_SRCS))


$(BIN_DIR)testmodule.so: $(X86_OBJS)
	$(call PRINTF,"Linking $@")
	$(CC_X86) -shared $^ -lpython$(PYTHON_VER) -o $@

$(BIN_DIR)%_.o: $(TDIR)%.c
	$(call PRINTF,"Compiling $<")
	$(CC_X86) $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -I/usr/include/python$(PYTHON_VER) -c $< -o $@

$(BIN_DIR)%_.o: $(X86_TEST_IDIR)%.c
	$(call PRINTF,"Compiling $<")
	$(CC_X86) $(TESTFLAGS) -I$(IDIR) -I$(X86_TEST_IDIR) -c $< -o $@

# -------------------------- X86 Standalone Build ----------------------------------------

x86_standalone: prepare_dirs
	$(MAKE) $(BIN_DIR)BoardComputer

# List all .o objects for x86 Standalone
X86_SRCS := $(addprefix src/, $(COMMON_SRCS) $(UI_SRCS)) \
            src/main.c

X86_IFACE_SRCS := $(X86_STANDALONE_IDIR)persistent_memory.c $(X86_STANDALONE_IDIR)system_interface.c $(X86_STANDALONE_IDIR)serial.c
X86_OBJS := $(patsubst src/%.c,$(BIN_DIR)%_x86.o,$(X86_SRCS)) \
             $(patsubst $(X86_STANDALONE_IDIR)%.c,$(BIN_DIR)%_x86.o,$(X86_IFACE_SRCS))

$(BIN_DIR)BoardComputer: $(X86_OBJS)
	$(call PRINTF,"Linking $@")
	$(CC_X86) $^ -o $@

$(BIN_DIR)%_x86.o: src/%.c
	$(call PRINTF,"Compiling $<")
	$(CC_X86) $(STANDALONEFLAGS) -I$(IDIR) -I$(X86_STANDALONE_IDIR) -c $< -o $@

$(BIN_DIR)%_x86.o: $(X86_STANDALONE_IDIR)%.c
	$(call PRINTF,"Compiling $<")
	$(CC_X86) $(STANDALONEFLAGS) -I$(IDIR) -I$(X86_STANDALONE_IDIR) -c $< -o $@

# -------------------------- AVR Build ----------------------------------------

HW_1: prepare_dirs $(BIN_DIR)BoardComputer.hex

# List all .o objects for AVR
AVR_SRCS := $(addprefix src/, $(COMMON_SRCS) $(UI_SRCS)) \
            src/main.c
AVR_IFACE_SRCS := $(HW_1_AVR_IDIR)persistent_memory.c $(HW_1_AVR_IDIR)system_interface.c $(HW_1_AVR_IDIR)serial.c

AVR_OBJS := $(patsubst src/%.c,$(BIN_DIR)%.o,$(AVR_SRCS)) \
             $(patsubst $(HW_1_AVR_IDIR)%.c,$(BIN_DIR)%.o,$(AVR_IFACE_SRCS))

$(BIN_DIR)BoardComputer.hex: $(AVR_OBJS)
	$(call PRINTF,"Linking $@")
	$(CC_AVR) -mmcu=atmega324pb $^ -o $(BIN_DIR)BoardComputer.elf
	avr-objcopy -j .text -j .data -O ihex $(BIN_DIR)BoardComputer.elf $@
	avr-size --mcu=atmega324pa -C $(BIN_DIR)BoardComputer.elf

$(BIN_DIR)%.o: src/%.c
	$(call PRINTF,"Compiling $<")
	$(CC_AVR) $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c $< -o $@

$(BIN_DIR)%.o: $(HW_1_AVR_IDIR)%.c
	$(call PRINTF,"Compiling $<")
	$(CC_AVR) $(AVRFLAGS) -I$(IDIR) -I$(HW_1_AVR_IDIR) -c $< -o $@


# -------------------------- Flash --------------------------------------------

flash:
	$(call PRINTF,"Flashing BoardComputer.hex")
	avrdude -p m324pb -c usbasp -C +"utils/324pb.conf" -B 0.5 -U flash:w:"$(BIN_DIR)BoardComputer.hex":a


# -------------------------- Cleanup ------------------------------------------

clean:
	$(call PRINTF,"Cleaning build files")
	rm -f $(BIN_DIR)*.o $(BIN_DIR)*.elf $(BIN_DIR)*.hex $(BIN_DIR)*.so
	rm -rf $(BIN_DIR)UI
	rm -rf $(BIN_DIR)Interface
	rm -rf $(TDIR)
