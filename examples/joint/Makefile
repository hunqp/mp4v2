CROSS_COMPILE   :=
CC              := $(CROSS_COMPILE)gcc
CXX             := $(CROSS_COMPILE)g++
LD              := $(CROSS_COMPILE)ld
AR              := $(CROSS_COMPILE)ar
RANLIB          := $(CROSS_COMPILE)ranlib
STRIP           := $(CROSS_COMPILE)strip

BIN     = joint
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard inc/*.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
IFLAGS  = -I./inc
CFLAGS  = $(IFLAGS) -g
LFLAGS  = -L./lib

$(BIN) : $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LFLAGS) -o $@
	$(STRIP) $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean :
	rm -f src/*.o  $(BIN)



