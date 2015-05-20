TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#==================================================
USR_CFLAGS += -DSTATUS_RECORD
# build a support library
OBJLIB_vxWorks += pmac

# statusRecord.h will be created from statusRecord.dbd
DBDINC += statusRecord

# install <name>.dbd into <top>/dbd
DBD += add_pmac.dbd
#DBD += add_pmac_nodpram.dbd
DBD += statusDevice.dbd
DBD += pmacInclude.dbd

# The following are compiled and added to the Support library
OBJLIB_SRCS += devPmacMbx.c
OBJLIB_SRCS += devPmacRam.c
OBJLIB_SRCS += drvPmac.c
OBJLIB_SRCS += pmacVme.c
OBJLIB_SRCS += pmacSerial.c
OBJLIB_SRCS += pmacError.c
OBJLIB_SRCS += pmacDriver.c
OBJLIB_SRCS += asynUploadFile.c
OBJLIB_SRCS += statusRecord.c
OBJLIB_SRCS += devStatusSoft.c

LIBRARY_vxWorks += pmacIoc

pmacIoc_SRCS += $(OBJLIB_SRCS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
