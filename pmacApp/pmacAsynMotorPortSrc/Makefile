TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------

DBD += pmacAsynMotorPort.dbd
DBD += pmacAsynMotorPortVxWorks.dbd

LIBRARY_IOC = pmacAsynMotorPort
LIB_LIBS += motor
LIB_LIBS += asyn
LIB_LIBS += $(EPICS_BASE_IOC_LIBS)

pmacAsynMotorPort_SRCS = pmacController.cpp pmacAxis.cpp pmacCsGroups.cpp

include $(TOP)/configure/RULES
