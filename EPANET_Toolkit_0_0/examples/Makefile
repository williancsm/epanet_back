PROGRAMS = evalpattern evalleveltriggers getcoords
.PHONY: all FORCE

all: $(PROGRAMS)

$(PROGRAMS): FORCE
	@echo "--> Building program '$@' <--"
	$(MAKE) -C $@

FORCE: ;