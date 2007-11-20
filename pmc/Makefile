TOP = ..
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard 1_begin_generic))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard dls))
include $(TOP)/configure/RULES_DIRS

