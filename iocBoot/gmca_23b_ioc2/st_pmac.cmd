### Configure PMAC-VME Hardware
#	args:	(1) EPICS VME Card #
#		(2) PMAC Base Address
#		(3) PMAC DPRAM Address (0=none)
#		(4) PMAC IRQ Vectors (v must be odd) (uses v-1,v,v+1)
#		(5) IRQ Level
pmacVmeDebug
devPmacMbxDebug
devPmacRamDebug
drvPmacDebug
# pmacSerialDebug

# For 24-bit PMAC settings:
# DPRAM address range is A24 0xF0700000 - 0xF0703FFF
# MBOX  address range is A24 0xF07FA000 - 0xF07FA1FF
  pmacVmeConfig (2, 0x6FA000, 0x600000, 0xF5, 6)
  pmacVmeConfig (3, 0x7FA000, 0x700000, 0xF1, 1)

### Configure PMAC-VME Driver
# 	args:	(1) EPICS VME Card #
#		(2) DPRAM Servo Fixed Buffer Scan Rate
# 			(ticks, 0=default, -1=disabled, -2=interrupt)
# 		(3) DPRAM Background Fixed Buffer Scan Rate
#			(ticks, 0=default, -1=disabled, -2=interrupt)
#		(4) DPRAM Background Variable Buffer Scan Rate
#			(ticks, 0=default, -1=disabled, -2=interrupt)
# 		(5) Mailbox Registers Disable
#			(0=default, 0=enable/-1=disable)
### Configure PMAC-VME Driver
### This uses MAILBOX:
# pmacAsynConfig( "PMAC_MBX_PORT_", 0 )
### This uses DPRAM ASCII:
  pmacAsynConfig( 0, "PMAC_MBX_PORT_" )

# pmacDrvConfig (
#	int		cardNumber,
#	int		scanMtrRate,
#	int		scanBkgRate,
#	int		scanVarRate,
#	char *		asynMbxPort)

  pmacDrvConfig (2, 0, 0, 0, "PMAC_MBX_PORT_2" )
  pmacDrvConfig (3, 0, 0, 0, "PMAC_MBX_PORT_3" )

#asynSetTraceIOMask("PMAC_MBX_PORT_0", -1, 1 )
#asynSetTraceMask("PMAC_MBX_PORT_0",-1,0x9)
#asynSetTraceIOMask("PMAC_MBX_PORT_1", -1, 1 )
#asynSetTraceMask("PMAC_MBX_PORT_1",-1,0x9)

### PMAC Templates for pmac20
#============================

### PMAC Databases
  dbLoadTemplate ("dbLoad_pmac20_23b/pmac.Ascii")
  dbLoadTemplate ("dbLoad_pmac20_23b/pmac.acc65e")

### Motor Databases
  dbLoadTemplate ("dbLoad_pmac20_23b/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_pmac20_23b/mtr.Ascii")
  dbLoadTemplate ("dbLoad_pmac20_23b/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_pmac20_23b/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_pmac20_23b/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_pmac20_23b/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_pmac20_23b/pcs.AsciiPcs")

### Assembly Databases:

### Common assembly:
  dbLoadTemplate ("dbLoad_pmac20_23b/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_pmac20_23b/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_pmac20_23b/assy.RamMap")
  dbLoadTemplate ("dbLoad_pmac20_23b/assy.Load")

#### Different Assembly Types:
  dbLoadTemplate ("dbLoad_pmac20_23b/ccdSt.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/bd.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/hs.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/sh.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/a.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/x.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/xy.Assy")
  dbLoadTemplate ("dbLoad_pmac20_23b/xyz.Assy")


### PMAC Templates for pmac21
#============================

### PMAC Databases
   dbLoadTemplate ("dbLoad_pmac21_23b/pmac.Ascii")
   dbLoadTemplate ("dbLoad_pmac21_23b/pmac.acc65e")
   dbLoadTemplate ("dbLoad_pmac21_23b/pmac.acc59e")

### Motor Databases
   dbLoadTemplate ("dbLoad_pmac21_23b/mtr.mtrdat")
   dbLoadTemplate ("dbLoad_pmac21_23b/mtr.Ascii")
   dbLoadTemplate ("dbLoad_pmac21_23b/mtr.RqsTwkMem")
   dbLoadTemplate ("dbLoad_pmac21_23b/mtr.LimTransfer")

### Coordinate System Databases
   dbLoadTemplate ("dbLoad_pmac21_23b/pcs.bkgfix1pcs")
   dbLoadTemplate ("dbLoad_pmac21_23b/pcs.CtlPnlPcs")
   dbLoadTemplate ("dbLoad_pmac21_23b/pcs.AsciiPcs")

### Assembly Databases:

### Common assembly:
   dbLoadTemplate ("dbLoad_pmac21_23b/assy.AssyGeneric")
   dbLoadTemplate ("dbLoad_pmac21_23b/assy.MotionPrg")
   dbLoadTemplate ("dbLoad_pmac21_23b/assy.RamMap")
   dbLoadTemplate ("dbLoad_pmac21_23b/assy.Load")

#### Different Assembly Types:
   dbLoadTemplate ("dbLoad_pmac21_23b/hs.Assy")
   dbLoadTemplate ("dbLoad_pmac21_23b/x.Assy")
   dbLoadTemplate ("dbLoad_pmac21_23b/xy.Assy")
   dbLoadTemplate ("dbLoad_pmac21_23b/xyz.Assy")

