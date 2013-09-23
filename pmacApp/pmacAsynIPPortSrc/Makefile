TOP=../..

include $(TOP)/configure/CONFIG

# -------------------------------

DBD = pmacAsynIPPort.dbd
INC += pmacAsynIPPort.h

LIBRARY = pmacAsynIPPort
LIB_LIBS += asyn
LIB_LIBS += $(EPICS_BASE_IOC_LIBS)
LIB_SYS_LIBS_WIN32 += Ws2_32

pmacAsynIPPort_SRCS = pmacAsynIPPort.c 

include $(TOP)/configure/RULES
