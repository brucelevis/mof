# GNU Make solution makefile autogenerated by Premake
# Type "make help" for usage help

ifndef config
  config=debug
endif
export config

PROJECTS := MergeDB

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

MergeDB: 
	@echo "==== Building MergeDB ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f MergeDB.make

clean:
	@${MAKE} --no-print-directory -C . -f MergeDB.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debug"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   MergeDB"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
