BUILDDIR:=build

MKDIR?=mkdir
CMAKE?=cmake

PWD:=$(CURDIR)

all:
	$(MKDIR) -p $(BUILDDIR) ; \
	cd $(BUILDDIR) && $(CMAKE) -D LLVM_CONFIG=$(LLVM_CONFIG) $(PWD) && $(MAKE)

clean:
	$(RM) -r $(BUILDDIR)

install:
	$(INSTALL) -m 0755 -d $(DESTDIR)/usr/lib/picon/ ; \
	$(INSTALL) -m 0644 $(BUILDDIR)/CFI/libCFI.so $(DESTDIR)/usr/lib/picon/


.PHONY: all clean install
