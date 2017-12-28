#if compile x86 demo on unbuntu 64.

DEVICE_NAME:= gm8136

ifeq ($(DEVICE_NAME),mt7688)
CC:= /opt/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-g++  
else ifeq ($(DEVICE_NAME),ar9331) 
CC:= /opt/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.16/bin/mips-openwrt-linux-g++  
else ifeq ($(DEVICE_NAME),gm8136) 
CC:= ../../workspace/GM8136SDKreleasev2.0/Software/Embedded_Linux/source/arm-linux-3.3/toolchain_gnueabi-4.4.0_ARMv5TE/usr/bin/arm-linux-g++
endif

export STAGING_DIR:=/staging_dir



#all: app0
#	@echo build complete


#clean:
#	-rm airkiss

#app0:airkiss.c
#	$(CC) airkiss.c -o airkiss -O0 -g3 -I"./include" -L"./lib" -lairkiss_log -lpthread -ldl -lstdc++ -lm -lrt


COBJS += main.o airkisstask.o json.o wireless.o

CFLAGS += -O3 -Wall -g3
INCDIRS	+= -I./ -I./inc

LDFLAGS	+=
LIBPATH  = -L./lib/$(DEVICE_NAME)
LIBVAR   = -ldl -lm -lrt -lpthread -lairkiss -lairkiss_log
CROSS_COMPILE	?=

TARGET = airkiss

all:$(TARGET)


$(TARGET):$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBPATH) $(LIBVAR)
 
%.o:%.cpp
#g++ $(CFLAGS) -c main.c $(LDFLAGS)
#g++ $(CFLAGS) -o ./libairkiss.a ./libairkiss_aes.a ./libairkiss_aes_log.a ./libairkiss_log.a
	$(CC) $(CFLAGS) $(INCDIRS) -c $^ $(LDFLAGS) 

.PHONY:clean
clean:
	rm -f $(COBJS)
	rm -f $(TARGET)
