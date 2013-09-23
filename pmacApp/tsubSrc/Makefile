TOP = ../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
USR_CFLAGS += -DDEBUG

### build a support library
LIBRARY_IOC += tsub
LIB_LIBS += $(EPICS_BASE_IOC_LIBS)

### tsub record
OBJLIB_SRCS  += tsubRecord.c

### tsubRecord.h will be created from tsubRecord.dbd
DBDINC += tsubRecord

### install <name>.dbd into <top>/dbd
DBD += tsubInclude.dbd

### Tsub functions sources
OBJLIB_SRCS  += tsubSeq.c
OBJLIB_SRCS  += tsubBD.c
OBJLIB_SRCS  += tsubBD3.c
OBJLIB_SRCS  += tsubCCD2009.c
# OBJLIB_SRCS  += tsubCCD.c
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
# OBJLIB_SRCS  += tsubA.c
OBJLIB_SRCS  += tsubX.c
# OBJLIB_SRCS  += tsubX_bm_extern_offset.c
OBJLIB_SRCS  += tsubXY.c
OBJLIB_SRCS  += tsubXYZ.c
OBJLIB_SRCS  += tsubXYZ_frac.c
OBJLIB_SRCS  += tsubAcc65e.c
OBJLIB_SRCS  += tsubCollimator.c

tsub_SRCS += $(OBJLIB_SRCS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
tsubRecord$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubSeq$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubBD$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubBD3$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubCCD2009$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubCS$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubHS$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubMD$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubMI$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubMO_2mtr_cry2down$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubSH$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubTB$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubX$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubXY$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubXYZ$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubXYZ_frac$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubAcc65e$(OBJ): $(COMMON_DIR)/tsubRecord.h
tsubCollimator$(OBJ): $(COMMON_DIR)/tsubRecord.h
