
all: hid_sppd

hid_sppd:
	cc -O2 -pipe   -std=gnu99 -fstack-protector -Wsystem-headers -Werror -Wall -Wno-format-y2k -Wno-uninitialized -Wno-pointer-sign -Wno-empty-body -Wno-string-plus-int -Wno-unused-const-variable -Wno-tautological-compare -Wno-unused-value -Wno-parentheses-equality -Wno-unused-function -Wno-enum-conversion -Wno-switch -Wno-switch-enum -Wno-knr-promoted-parameter -Qunused-arguments -lsdp -lbluetooth -o hid_sppd hid_sppd.c

clean:
	rm -f *.o
	rm -f hid_sppd 
