MCU = atmega8
TRG = tqw
SRC = $(TRG).c tqw_text.c
ASRC =
LIB =
INC = 
ASFLAGS = -Wa, -gstabs
CPFLAGS = -g -O3 -Wall -Wstrict-prototypes -Wa,-ahlms=$(<:.c=.lst)
LDFLAGS = -Wl,-Map=$(TRG).map,--cref,-mavrmega8

CC = avr-gcc
AS = avr-gcc -x assembler-with-cpp	
RM = rm -f
RN = mv
OUT = coff
ELFCOF = elfcoff
CP = cp
BIN = avr-objcopy
SIZE = avr-size
INCDIR = .
LIBDIR = /usr/local/avr/lib
SHELL = /bin/sh
FORMAT = ihex

OBJ = $(ASRC:.s=.o) $(SRC:.c=.o)

CPFLAGS += -mmcu=$(MCU)
ASFLAGS += -mmcu=$(MCU)
LDFLAGS += -mmcu=$(MCU)

all : tqw.hex

tqw.hex: tqw.elf
	$(BIN) -O $(FORMAT) -R .eeprom $< $@

%.hex : %.elf
	$(BIN) -O $(FORMAT) -R .eeprom $< $@

%.o : %.c 
	$(CC) -c $(CPFLAGS) -I$(INCDIR) $< -o $@

%.s : %.c
	$(CC) -S $(CPFLAGS) -I$(INCDIR) $< -o $@

tqw.elf: $(OBJ)
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@

clean:
	$(RM) *.hex *.elf *.map *.lst *.o
	$(RM) $(TRG).avd
