###########################################################################
#
#   cps1.mak
#
#   Small driver-specific example makefile
#   Use make SUBTARGET=cps1 to build
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit  http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

MAMESRC = $(SRC)/mame
MAMEOBJ = $(OBJ)/mame

AUDIO = $(MAMEOBJ)/audio
DRIVERS = $(MAMEOBJ)/drivers
LAYOUT = $(MAMEOBJ)/layout
MACHINE = $(MAMEOBJ)/machine
VIDEO = $(MAMEOBJ)/video

OBJDIRS += \
	$(AUDIO) \
	$(DRIVERS) \
	$(LAYOUT) \
	$(MACHINE) \
	$(VIDEO) \

#-------------------------------------------------
# Specify all the CPU cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

CPUS += Z80
CPUS += DSP16A
CPUS += M680X0

#-------------------------------------------------
# Specify all the sound cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

SOUNDS += YM2151
SOUNDS += YM2203
SOUNDS += OKIM6295
SOUNDS += MSM5205
SOUNDS += QSOUND

#-------------------------------------------------
# specify available video cores
#-------------------------------------------------

#-------------------------------------------------
# specify available machine cores
#-------------------------------------------------

MACHINES += TIMEKPR
MACHINES += EEPROMDEV
MACHINES += Z80CTC
MACHINES += I8255

#-------------------------------------------------
# specify available bus cores
#-------------------------------------------------

#-------------------------------------------------
# This is the list of files that are necessary
# for building all of the drivers referenced
# in tiny.c
#-------------------------------------------------

DRVLIBS = \
	$(DRIVERS)/fcrash.o \
	$(DRIVERS)/capcom.o $(DRIVERS)/cps1.o $(VIDEO)/cps1.o \
	$(MACHINE)/kabuki.o $(DRIVERS)/kenseim.o

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/kenseim.o:   $(LAYOUT)/kenseim.lh