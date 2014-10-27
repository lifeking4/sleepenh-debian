# just a simple makefile

all: sleepenh manpage

clean:
	rm -fv sleepenh sleepenh.1.gz

distclean: clean

sleepenh:
	gcc -o sleepenh -Wall -O2 sleepenh.c

manpage:
	if [ ! -e "sleepenh.1.gz" ]; then \
	cat sleepenh.1 | gzip -9 > sleepenh.1.gz ; fi

install: sleepenh manpage
	mkdir -p ${DESTDIR}/usr/bin/
	install -m 0755 -o root -g root sleepenh ${DESTDIR}/usr/bin/sleepenh
	mkdir -p ${DESTDIR}/usr/share/man/man1/
	install -m 0644 -o root -g root sleepenh.1.gz ${DESTDIR}/usr/share/man/man1/sleepenh.1.gz
