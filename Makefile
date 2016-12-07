
MFT = mft
MAP = mft1_sorted

all: _teste

ifeq ($(DEBUG),1)
CFLAGS := $(CFLAGS) -DDEBUG
endif
ifeq ($(WRITE_LIST),1)
CFLAGS := $(CFLAGS) -DWRITE_LIST
endif

LDFLAGS := $(LDFLAGS) -lreadline
a: a.o

b: b.o

e: e.o

testa: a
	./$< < mbr

testb: b
	./$< < $(MFT)

_testb: b
	./$< < $(MFT) | more

clean:
	rm -f *.o a b e

_teste: e
	./$< < $(MAP) | more
