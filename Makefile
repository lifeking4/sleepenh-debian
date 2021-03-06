Q ?= @
ifneq ($(Q),@)
Q :=
endif

CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -O2

targets := sleepenh sleepenh.1.gz

all: $(targets)

.clean = $(wildcard $(targets))
distclean clean:
ifneq ($(.clean),)
	rm -f $(.clean)
endif

sleepenh: CFLAGS += -DVCSVERSION=$(call vcsversion)

%.1.gz: %.1
	gzip -9 < $< > $@

install: $(targets)
	install -D -m 0755 -o root -g root sleepenh ${DESTDIR}/usr/bin/sleepenh
	install -D -m 0644 -o root -g root sleepenh.1.gz ${DESTDIR}/usr/share/man/man1/sleepenh.1.gz

define vcsversion
'$(shell git describe --dirty | sed -e 's,^\(.*\)/\([^/]\+\),\2/\1,;s,^.*$$,\"&\",')'
endef
