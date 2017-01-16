TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE


PROD_IOC += testPmacAsynIP

DBD += testPmacAsynIPPort.dbd
DBD += testPmacAsynIPPortVx.dbd


testPmacAsynIPPort_DBD += base.dbd
testPmacAsynIPPort_DBD += asyn.dbd
testPmacAsynIPPort_DBD += motorSupport.dbd
testPmacAsynIPPort_DBD += drvAsynIPPort.dbd
testPmacAsynIPPort_DBD += drvAsynSerialPort.dbd
testPmacAsynIPPort_DBD += pmacAsynMotor.dbd
testPmacAsynIPPort_DBD += pmacAsynIPPort.dbd
testPmacAsynIPPort_DBD += testPmacAsynIP.dbd

testPmacAsynIPPortVx_DBD += $(testPmacAsynIPPort_DBD) pmacInclude.dbd


testPmacAsynIP_SRCS = testPmacAsynIPPort.c 
testPmacAsynIP_SRCS_DEFAULT += testPmacAsynIPPort_registerRecordDeviceDriver.cpp testPmacAsynIPPortMain.cpp
testPmacAsynIP_SRCS_vxWorks  += testPmacAsynIPPort_registerRecordDeviceDriver.cpp

testPmacAsynIPPort_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary



# This line says that this IOC Application depends on the
# xxx Support Module
testPmacAsynIP_LIBS_vxWorks += pmacIoc $(EPICS_BASE_IOC_LIBS)
testPmacAsynIP_LIBS += pmacAsynMotor
testPmacAsynIP_LIBS += pmacAsynIPPort
testPmacAsynIP_LIBS += motor
testPmacAsynIP_LIBS += asyn

# We need to link this IOC Application against the EPICS Base libraries
testPmacAsynIP_LIBS_DEFAULT += $(EPICS_BASE_IOC_LIBS)



include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
