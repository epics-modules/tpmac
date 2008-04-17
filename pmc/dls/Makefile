TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

DATA += $(patsubst ../%, %, $(wildcard ../*.pmc))
DATA += Example_processed.pmc

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

%_processed.pmc: ../%.pmc
	dls-pmc-preprocessor.py -c -o $@ -I ..:$(TPMAC)/data $<
