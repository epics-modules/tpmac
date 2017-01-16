TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------

DBD = pmacAsynIPPort.dbd
INC += pmacAsynIPPort.h

LIBRARY = pmacAsynIPPort
pmacAsynIPPort_SRCS = pmacAsynIPPort.c 
LIB_LIBS += asyn $(EPICS_BASE_IOC_LIBS)
LIB_SYS_LIBS_WIN32 += ws2_32

include $(TOP)/configure/RULES
