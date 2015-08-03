INCLUDEDIR:=include
SRCDIR:=src

PROJECTDIR?=../

INCLUDES:=$(wildcard $(INCLUDEDIR)/*.h)
SRCFILES:=\
	defs.c \
	call_stack.c \
	block_stack.c \
	monitor_dot.c \
	monitor_load.c \
	monitor_run.c \
	main.c

EXE:=picon-monitor

CFLAGS+=-Wall -Wextra -Wtype-limits -fstrict-overflow -Wsign-compare -std=c89
CFLAGS+=-I$(PROJECTDIR)/include
CFLAGS+=-O2
#CFLAGS+=-g

LDFLAGS:=

#CFLAGS+=-DSILENT
#CFLAGS+=-DCFI_DEBUG
#CFLAGS+=-DNB_PRELOADED_MONITOR_OK_ANSWERS=1
#CFLAGS+=-DTIMEOUT
#CFLAGS+=-DTIMEOUT_SEC_MONITOR_NO_SIGNAL=0
#CFLAGS+=-DTIMEOUT_NANOSEC_MONITOR_NO_SIGNAL=500000000



#################
MKDIR?=mkdir
BUILDDIR:=build

_SRCFILES:=$(addprefix $(SRCDIR)/,$(SRCFILES))
_OFILES:=$(addprefix $(BUILDDIR)/,$(patsubst %.c,%.o,$(filter %.c,$(SRCFILES))))

CFLAGS+=-I$(INCLUDEDIR)



all: $(BUILDDIR)/$(EXE)

clean:
	$(RM) -r $(BUILDDIR)

install:
	$(INSTALL) -m 0755 -d $(DESTDIR)/usr/bin/ ; \
	$(INSTALL) -m 0755 $(BUILDDIR)/picon-monitor $(DESTDIR)/usr/bin/

.PHONY: all clean install


$(BUILDDIR)/$(EXE): $(_OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o "$@" $^

$(BUILDDIR):
	$(MKDIR) "$@"

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR) $(INCLUDES)
	$(CC) $(CFLAGS) -c -o "$@" "$<"
