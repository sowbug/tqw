MCU = atmega8
TRG = tqw
SRC = $(TRG).c random.c
ASRC =
LIB =
INC = 
ASFLAGS = -Wa, -gstabs
CPFLAGS = -g -O3 -Wall -Wstrict-prototypes -Wa,-ahlms=$(<:.c=.lst)
LDFLAGS = -Wl,-Map=$(TRG).map,--cref,-mavrmega8
#LDFLAGS = -Wl,-Map=$(TRG).map,--cref,-mavr4

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

SRC1 = $(SRC) fire.c sprites.c life.c wave.c
SRC2 = $(SRC) bowl.c chord.c krypton.c zipper.c ringer.c

OBJ1 = $(ASRC:.s=.o) $(SRC1:.c=.o) mem1.o
OBJ2 = $(ASRC:.s=.o) $(SRC2:.c=.o) mem2.o

CPFLAGS += -mmcu=$(MCU)
ASFLAGS += -mmcu=$(MCU)
LDFLAGS += -mmcu=$(MCU)

all : tqw1.hex tqw2.hex

tqw1.hex: tqw1.elf
	$(BIN) -O $(FORMAT) -R .eeprom $< $@

tqw2.hex: tqw2.elf
	$(BIN) -O $(FORMAT) -R .eeprom $< $@

$(SRC1) $(SRC2) : memory.h rand.h tqw.h

%.o : %.c 
	$(CC) -c $(CPFLAGS) -I$(INCDIR) $< -o $@

mem1.o : memory.c
	$(CC) -DMODEL=1 -c $(CPFLAGS) -I$(INCDIR) $< -o $@

mem2.o : memory.c
	$(CC) -DMODEL=2 -c $(CPFLAGS) -I$(INCDIR) $< -o $@

%.s : %.c
	$(CC) -S $(CPFLAGS) -I$(INCDIR) $< -o $@

tqw1.elf: $(OBJ1)
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@

tqw2.elf: $(OBJ2)
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@

%.hex: %.elf
	$(BIN) -O $(FORMAT) -R .eeprom $< $@

clean:
	$(RM) *.hex *.elf *.map *.lst *.o
	$(RM) $(TRG).avd
