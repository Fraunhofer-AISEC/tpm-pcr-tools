CC ?= gcc
DEBUG_BUILD ?= n

LOCAL_CFLAGS := -g -std=gnu99 -Icommon/ -ftrapv -O0
LOCAL_CFLAGS += -Wall -Wextra -Wcast-align -Wformat -Wformat-security

ifeq ($(DEBUG_BUILD),y)
    # if requested, we enable sanitization for easier debugging
    # this requires libasan libasan-static libubsan libubsan-static
    # to be installed on the build host
    LOCAL_CFLAGS += -fsanitize=address -static-libasan -fsanitize=undefined -fsanitize-recover=address
endif

DEFINES := -DTPM_POSIX -DPRINT=printf
ifeq ($(DEBUG_BUILD),y)
	# If requested, we enable the DEBUG_BUILD define which causes increased logging output
	DEFINES += -DDEBUG_BUILD
endif

LFLAGS := -lcrypto

ISRCS := \
	./ \
	../common/

CSRS := \
	main.c \
	../common/common.c \
	../common/hash.c \

IFLAGS := $(ISRCS:%=-I%)

.PHONY: all
all: calculate-ima-pcr

.PHONY: calculate-ima-pcr
calculate-ima-pcr:
	$(CC) $(DEFINES) $(LOCAL_CFLAGS) $(IFLAGS) -o calculate-ima-pcr $(CSRS) $(LFLAGS)

.PHONY: install
install:
	install -v -m 0755 ./calculate-ima-pcr /usr/bin/

.PHONY: clean
clean:
	rm -f calculate-ima-pcr *.o
