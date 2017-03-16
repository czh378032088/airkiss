
COBJS += main.o wireless.o

#CFLAGS += -O2 -Wall -DDEBUG
CFLAGS	+= -I./ -I./inc

LDFLAGS	+=
LIBPATH  = -L./airkisslib
LIBVAR   = -lairkiss_aes -lairkiss_aes_log
CROSS_COMPILE	?=

CC = $(CROSS_COMPILE)g++

TARGET = airkissTest

all:$(TARGET)


$(TARGET):$(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBPATH) $(LIBVAR)
 
%.o:%.cpp
#g++ $(CFLAGS) -c main.c $(LDFLAGS)
#g++ $(CFLAGS) -o ./libairkiss.a ./libairkiss_aes.a ./libairkiss_aes_log.a ./libairkiss_log.a
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS) 

.PHONY:clean
clean:
	rm -f $(COBJS)
	rm -f $(TARGET)
