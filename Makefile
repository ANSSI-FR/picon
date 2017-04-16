
export LLVM_CONFIG:=llvm-config-3.8
export OPT:=opt-3.8


export MKDIR:=mkdir
export CMAKE:=cmake
export INSTALL:=install

##################

export PROJECTDIR:=$(CURDIR)
export CFI_PASS:=$(PROJECTDIR)/llvm_pass/build/CFI/libCFI.so
export CFI_PRELINK:=$(PROJECTDIR)/prelink/picon-prelink
export CFI_INCLUDE:=$(PROJECTDIR)/include


SUBDIRS:=llvm_pass monitor prelink


TARGET?=all

%:
	$(MAKE) recursive TARGET=$@

recursive:
	for DIR in $(SUBDIRS) ; do $(MAKE) -C $${DIR} $(TARGET) || exit 1 ; done

install:
	$(MAKE) recursive TARGET=$@
	$(MAKE) -C examples $@
	$(INSTALL) -m 0755 -d $(DESTDIR)/usr/include/picon ; \
	for i in include/picon/* ; do \
		$(INSTALL) -m 0644 "$${i}" $(DESTDIR)/usr/include/picon/ ; \
	done


.PHONY: install recursive
