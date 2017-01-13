TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------

DBD = pmacAsynMotor.dbd

LIBRARY = pmacAsynMotor
pmacAsynMotor_SRCS = pmacAsynMotor.c pmacAsynMotorRegister.cc
LIB_LIBS += asyn motor $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
