TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# In a Diamond Support Module, install
# templates like this
#
#DB += motor.template
#DB += basic_motor.template

#----------------------------------------------------
# In a Diamond IOC Application, build db files from
# template files like this
#
DB += pmacTest.db

USES_TEMPLATE += $(MOTOR)/db/basic_asyn_motor.template

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
