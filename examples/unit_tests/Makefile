INSTALL?=install

SRCDIR:=src

INSTALL_DATA:=ignored_functions.txt Makefile Makefile.cfi Makefile.common Makefile.no_cfi
INSTALL_DATA_SRC:=$(patsubst $(SRCDIR)/%,%,$(wildcard $(SRCDIR)/*.c))

TARGET?=all

%:
	$(MAKE) both TARGET=$@

both:
	$(MAKE) -f Makefile.no_cfi $(TARGET)
	$(MAKE) -f Makefile.cfi $(TARGET)

install:
	$(INSTALL) -m 0755 -d $(EXAMPLES_DESTDIR)/unit_tests; \
	$(INSTALL) -m 0755 -d $(EXAMPLES_DESTDIR)/unit_tests/src; \
	for file in $(INSTALL_DATA); do \
		$(INSTALL) -m 0644 "$${file}" $(EXAMPLES_DESTDIR)/unit_tests; \
	done ; \
	for file in $(INSTALL_DATA_SRC); do \
		$(INSTALL) -m 0644 "$(SRCDIR)/$${file}" $(EXAMPLES_DESTDIR)/unit_tests/src; \
	done


