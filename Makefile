MKDIRS    = mkdir -p $1
BUILDDIR := build/
DISTDIR  := distribution/

BUILD_CC := gcc
NOP      := exit 0

.PHONY : all $(MAKECMDGOALS) $(MAKEFILE_LIST) $(BUILDDIR)Tubsfiles $(BUILDDIR)builddate

TARGETS := $(filter-out $(BUILDDIR)Tubsfiles $(BUILDDIR)tubsfind $(BUILDDIR)builddate, $(MAKECMDGOALS))

ifeq ($(TARGETS),)
    TARGETS := all
endif

$(firstword $(TARGETS)) : $(BUILDDIR)builddate $(BUILDDIR)Tubsfiles
#	touch aweb/version.c
	@$(MAKE) -f Tubsengine --no-print-directory $(MAKEFLAGS) $(TARGETS)

$(wordlist 2, $(words $(TARGETS)), $(TARGETS)) :
	@$(NOP)

$(BUILDDIR)Tubsfiles : $(BUILDDIR)tubsfind
	$(BUILDDIR)tubsfind -c $(BUILDDIR)tubscache -o $@ -e $(BUILDDIR) -e $(DISTDIR) -e %/CVS

$(BUILDDIR)tubsfind : tools/tubsfind/tubsfind.c tools/tubsfind/dir.c tools/tubsfind/ioerr2errno.c
	$(call MKDIRS, $(BUILDDIR))
	$(BUILD_CC) $^ -o  $@ -I tools/tubsfind -Wall

$(BUILDDIR)builddate : tools/builddate.c
	$(call MKDIRS, $(BUILDDIR))
	$(BUILD_CC) $^ -o $@
