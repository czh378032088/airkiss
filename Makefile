#if compile x86 demo on unbuntu 64.
CC:= /opt/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc
export STAGING_DIR:=/staging_dir


all: app0
	@echo build complete


clean:
	-rm airkiss

app0:airkiss.c
	$(CC) airkiss.c -o airkiss -O0 -g3 -I"./include" -L"./lib" -lairkiss_log -lpthread -ldl -lstdc++ -lm -lrt
