TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

DB += pmacAsynMotor.db

PROD_IOC = pmacAsynMotor

DBD += pmacAsynMotorExample.dbd
DBD += pmacAsynMotorVx.dbd

pmacAsynMotorExample_DBD += base.dbd
pmacAsynMotorExample_DBD += asyn.dbd
pmacAsynMotorExample_DBD += motorSupport.dbd
pmacAsynMotorExample_DBD += drvAsynIPPort.dbd
pmacAsynMotorExample_DBD += drvAsynSerialPort.dbd
pmacAsynMotorExample_DBD += pmacAsynMotor.dbd
pmacAsynMotorExample_DBD += pmacAsynIPPort.dbd

pmacAsynMotorVx_DBD += $(pmacAsynMotorExample_DBD) pmacInclude.dbd

pmacAsynMotor_SRCS_DEFAULT += pmacAsynMotorExample_registerRecordDeviceDriver.cpp pmacAsynMotorMain.cpp
pmacAsynMotor_SRCS_vxWorks += pmacAsynMotorVx_registerRecordDeviceDriver.cpp


pmacAsynMotor_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# This line says that this IOC Application depends on the
# xxx Support Module
pmacAsynMotor_LIBS_vxWorks += pmacIoc $(EPICS_BASE_IOC_LIBS)
pmacAsynMotor_LIBS += pmacAsynMotor
pmacAsynMotor_LIBS += pmacAsynIPPort
pmacAsynMotor_LIBS += motor
pmacAsynMotor_LIBS += asyn
pmacAsynMotor_LIBS += asyn
#pmacAsynMotor_LIBS += stream

# We need to link this IOC Application against the EPICS Base libraries
pmacAsynMotor_LIBS_DEFAULT += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
