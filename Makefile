PREFIX=		/usr/cust
INSTALL=	/usr/bin/install
CFLAGS=		-O2 -pipe
CC=		clang

all: hid_sppd artbt

hid_sppd:
	$(CC) $(CFLAGS) -std=gnu99 -fstack-protector -Wsystem-headers -Werror -Wall -Wno-format-y2k -Wno-uninitialized -Wno-pointer-sign -Wno-empty-body -Wno-string-plus-int -Wno-unused-const-variable -Wno-tautological-compare -Wno-unused-value -Wno-parentheses-equality -Wno-unused-function -Wno-enum-conversion -Wno-switch -Wno-switch-enum -Wno-knr-promoted-parameter -Qunused-arguments -lsdp -lbluetooth -lutil -o hid_sppd hid_sppd.c

artbt:
	$(CC) $(CFLAGS) -std=gnu99 -fstack-protector -Wsystem-headers -Werror -Wall -Wno-format-y2k -Wno-uninitialized -Wno-pointer-sign -Wno-empty-body -Wno-string-plus-int -Wno-unused-const-variable -Wno-tautological-compare -Wno-unused-value -Wno-parentheses-equality -Wno-unused-function -Wno-enum-conversion -Wno-switch -Wno-switch-enum -Wno-knr-promoted-parameter -Qunused-arguments -lsdp -lbluetooth -lutil -o artbt artbt.c

clean:
	rm -f *.o
	rm -f hid_sppd 
	rm -f artbt

install:
	mkdir -p $(PREFIX)/sbin
	mkdir -p $(PREFIX)/bin
	@(echo "Installing hid_sppd")
	$(INSTALL) -s -m 0755 -o root -g wheel hid_sppd $(PREFIX)/sbin/

deinstall:
	@(echo "Deinstalling hid_sppd")
	rm -f $(PREFIX)/sbin/hid_sppd
