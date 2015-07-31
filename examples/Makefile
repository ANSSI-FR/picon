SUBDIRS:=unit_tests shared_libs

INSTALL?=install


TARGET?=all
export CC:=clang

%:
	$(MAKE) recursive TARGET=$@

recursive:
	for DIR in $(SUBDIRS) ; do $(MAKE) -C $${DIR} $(TARGET) ; done

install:
	$(INSTALL) -m 0755 -d $(DESTDIR)/usr/share/doc/picon/examples/ ; \
	$(INSTALL) -m 0644 Makefile $(DESTDIR)/usr/share/doc/picon/examples/ ; \
	$(MAKE) recursive TARGET=$@ EXAMPLES_DESTDIR=$(DESTDIR)/usr/share/doc/picon/examples

.PHONY: install recursive

