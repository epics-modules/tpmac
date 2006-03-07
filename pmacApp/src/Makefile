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

# tsubRecord.h will be created from tsubRecord.dbd
DBDINC += tsubRecord

# install <name>.dbd into <top>/dbd
DBD += add_pmac.dbd

# The following are compiled and added to the Support library
OBJLIB_SRCS += devPmacMbx.c
OBJLIB_SRCS += devPmacRam.c
OBJLIB_SRCS += drvPmac.c
OBJLIB_SRCS += pmacVme.c
OBJLIB_SRCS += pmacSerial.c
OBJLIB_SRCS += statusRecord.c
OBJLIB_SRCS += tsubRecord.c

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

