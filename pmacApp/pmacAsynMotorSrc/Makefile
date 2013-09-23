TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------

DBD = pmacAsynMotor.dbd

LIBRARY = pmacAsynMotor
LIB_LIBS += asyn
LIB_LIBS += motor
LIB_LIBS += $(EPICS_BASE_IOC_LIBS)

pmacAsynMotor_SRCS = pmacAsynMotor.c pmacAsynMotorRegister.cc

include $(TOP)/configure/RULES
