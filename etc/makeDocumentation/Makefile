TOP = ../..
include $(TOP)/configure/CONFIG

# set to -d to get debugging
DEBUG :=

# this is the doxygen output dir
DOCDIR := $(TOP)/documentation/doxygen

# add the documentation files to the install target
install: $(DOCDIR)/build_instructions $(TOP)/documentation/index.html

# rule for documentation
$(TOP)/documentation/index.html: config.src manual.src $(DOCDIR)/build_instructions
	mkdir -p $(DOCDIR)
	/dls_sw/work/common/python/documentation/src/documentation.py -o $(DOCDIR) config.src manual.src
	cd $(TOP)/documentation; ln -s doxygen/index.html

# rule for generating build instructions from example.xml file
$(DOCDIR)/build_instructions: $(TOP)/etc/makeIocs/example.xml
	mkdir -p $(DOCDIR)
	/dls_sw/work/common/python/xmlbuilder/src/xmlbuilder.py --doc=$@ $(DEBUG) $^

# Remove entire documentation/doxygen dir on clean
clean:
	rm -rf $(DOCDIR) $(TOP)/documentation/index.html
