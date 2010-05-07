TOP = ../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
USR_CFLAGS += -DDEBUG

# build a support library
OBJLIB_vxWorks += tsub

# Tsub sources
OBJLIB_SRCS  += tsubSeq.c
OBJLIB_SRCS  += tsubBD.c
OBJLIB_SRCS  += tsubCCD.c
# OBJLIB_SRCS  += tsubCCD_bm_no_extern_offset.c
OBJLIB_SRCS  += tsubCS.c
# OBJLIB_SRCS  += tsubDN.c
OBJLIB_SRCS  += tsubHS.c
OBJLIB_SRCS  += tsubMD.c
OBJLIB_SRCS  += tsubMI.c
OBJLIB_SRCS  += tsubMO_2mtr_cry2down.c
# OBJLIB_SRCS  += tsubMO_3mtr_cry2up.c
# OBJLIB_SRCS  += tsubMO_3mtr_cry2down.c
OBJLIB_SRCS  += tsubSH.c
OBJLIB_SRCS  += tsubTB.c
OBJLIB_SRCS  += tsubX.c
# OBJLIB_SRCS  += tsubX_bm_extern_offset.c
OBJLIB_SRCS  += tsubXY.c
OBJLIB_SRCS  += tsubXYZ.c
OBJLIB_SRCS  += tsubAcc65e.c

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
