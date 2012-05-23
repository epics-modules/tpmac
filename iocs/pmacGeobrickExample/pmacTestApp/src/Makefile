TOP=../..

include $(TOP)/configure/CONFIG

# ------------------------
# Build an IOC Application
# ------------------------

PROD_IOC = pmacTest

# pmacTest.dbd will be installed into <top>/dbd
DBD += pmacTest.dbd

# pmacTest.dbd will be created from these files
pmacTest_DBD += base.dbd
pmacTest_DBD += asyn.dbd
pmacTest_DBD += motorSupport.dbd
pmacTest_DBD += pmacAsynCoord.dbd
pmacTest_DBD += drvAsynIPPort.dbd
pmacTest_DBD += pmacAsynMotor.dbd
pmacTest_DBD += pmacAsynIPPort.dbd
pmacTest_DBD += sscanSupport.dbd
pmacTest_DBD += calcSupport.dbd
pmacTest_DBD += asSupport.dbd
pmacTest_DBD += stream.dbd
pmacTest_DBD += genSubRecord.dbd
pmacTest_DBD += pmacUtilSupport.dbd
pmacTest_DBD += stdSupport.dbd
pmacTest_DBD += busySupport.dbd

# pmacTest_registerRecordDeviceDriver.cpp will be created
# pmacTest.dbd
pmacTest_SRCS += pmacTest_registerRecordDeviceDriver.cpp

# These two lines are needed for non-vxWorks builds, such as Linux
pmacTest_SRCS_DEFAULT += pmacTestMain.cpp
pmacTest_SRCS_vxWorks += -nil-

# This line says that this IOC Application depends on the
# xxx Support Module
pmacTest_LIBS += pmacAsynCoord
pmacTest_LIBS += pmacAsynMotor
pmacTest_LIBS += pmacAsynIPPort
pmacTest_LIBS += motor
pmacTest_LIBS += calc
pmacTest_LIBS += autosave
pmacTest_LIBS += genSub
pmacTest_LIBS += pmacUtil
pmacTest_LIBS += sscan
pmacTest_LIBS += seq pv
pmacTest_LIBS += std
pmacTest_LIBS += stream pcre
pmacTest_LIBS += asyn
pmacTest_LIBS += busy


# We need to link this IOC Application against the EPICS Base libraries
pmacTest_LIBS += $(EPICS_BASE_IOC_LIBS)

# The following adds object code from base/src/vxWorks
pmacTest_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

DATA += pmacTestGUI
DATA += pmacGeobrickExample.req
DATA += pmacTest_processed.pmc

include $(TOP)/configure/RULES

%_processed.pmc: ../%.pmc
	dls-pmc-preprocessor.py -c -o $@ -I .:..:$(PMACUTIL)/data:$(PMACCOORD)/data $<

