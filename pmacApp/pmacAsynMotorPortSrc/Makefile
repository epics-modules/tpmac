TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------

DBD += pmacAsynMotorPort.dbd
DBD += pmacAsynMotorPortVxWorks.dbd

LIBRARY_IOC = pmacAsynMotorPort
pmacAsynMotorPort_SRCS = pmacController.cpp pmacAxis.cpp

include $(TOP)/configure/RULES
