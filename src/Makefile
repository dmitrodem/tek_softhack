TARGET = /opt/crosstool/gcc-3.3.6-glibc-2.3.6/powerpc-405-linux-gnu/bin/powerpc-405-linux-gnu-
CC = $(TARGET)gcc
LD = $(TARGET)ld
STRIP = $(TARGET)strip

CFLAGS = -O2 -Wall -fPIC -DPIC
LDFLAGS = -shared -ldl

headers := 
src := override.c
obj := $(patsubst %.c,%.o,$(src))
lib := override.so

clean := 
phony := 

all:
phony += all

$(obj) : %.o: %.c $(headers)
	$(CC) $(CFLAGS) -c $< -o $@
clean += $(obj)

$(lib): $(obj)
	$(LD) $(LDFLAGS) $^ -o $@
	$(STRIP) --strip-unneeded $@
clean += $(lib)

all: $(lib)

clean:
	-rm -rf $(clean)
phony += clean

.PHONY: $(phony)
