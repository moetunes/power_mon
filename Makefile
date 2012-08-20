CFLAGS+= -Wall
LDADD+= -lX11 
LDFLAGS=
EXEC=power_mon

PREFIX?= /usr/local
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

power_mon: power_mon.o
	$(CC) $(LDFLAGS) -O2 -ffast-math -fno-unit-at-a-time -o $@ $+ $(LDADD)

install: all
	install -Dm 755 power_mon $(DESTDIR)$(BINDIR)/power_mon

clean:
	rm -fv power_mon *.o
