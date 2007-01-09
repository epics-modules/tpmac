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
  pmacVmeConfig (0, 0x6FA000, 0x600000, 0xa5, 6)
  pmacVmeConfig (1, 0x7FA000, 0x700000, 0xa1, 1)

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

  pmacDrvConfig (0, 0, 0, 0, "PMAC_MBX_PORT_0" )
  pmacDrvConfig (1, 0, 0, 0, "PMAC_MBX_PORT_1" )

#asynSetTraceIOMask("PMAC_MBX_PORT_0", -1, 1 )
#asynSetTraceMask("PMAC_MBX_PORT_0",-1,0x9)
#asynSetTraceIOMask("PMAC_MBX_PORT_1", -1, 1 )
#asynSetTraceMask("PMAC_MBX_PORT_1",-1,0x9)

### PMAC Templates for pmac20
#============================

### PMAC Databases
  dbLoadTemplate ("dbLoad_endstation_pmac20/pmac.Ascii")
  dbLoadTemplate ("dbLoad_endstation_pmac20/pmac.acc65e")

### Motor Databases
  dbLoadTemplate ("dbLoad_endstation_pmac20/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_endstation_pmac20/mtr.Ascii")
  dbLoadTemplate ("dbLoad_endstation_pmac20/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_endstation_pmac20/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_endstation_pmac20/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_endstation_pmac20/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_endstation_pmac20/pcs.AsciiPcs")

### Assembly Databases:

### Common assembly:
  dbLoadTemplate ("dbLoad_endstation_pmac20/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_endstation_pmac20/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_endstation_pmac20/assy.RamMap")
  dbLoadTemplate ("dbLoad_endstation_pmac20/assy.Load")

#### Different Assembly Types:
  dbLoadTemplate ("dbLoad_endstation_pmac20/ccdSt.Assy")
  dbLoadTemplate ("dbLoad_endstation_pmac20/bd.Assy")
  dbLoadTemplate ("dbLoad_endstation_pmac20/hs.Assy")
  dbLoadTemplate ("dbLoad_endstation_pmac20/sh.Assy")
  dbLoadTemplate ("dbLoad_endstation_pmac20/x.Assy")
  dbLoadTemplate ("dbLoad_endstation_pmac20/xy.Assy")
  dbLoadTemplate ("dbLoad_endstation_pmac20/xyz.Assy")


### PMAC Templates for pmac21
#============================

### PMAC Databases
   dbLoadTemplate ("dbLoad_endstation_pmac21/pmac.Ascii")
   dbLoadTemplate ("dbLoad_endstation_pmac21/pmac.acc65e")
   dbLoadTemplate ("dbLoad_endstation_pmac21/pmac.acc59e")

### Motor Databases
   dbLoadTemplate ("dbLoad_endstation_pmac21/mtr.mtrdat")
   dbLoadTemplate ("dbLoad_endstation_pmac21/mtr.Ascii")
   dbLoadTemplate ("dbLoad_endstation_pmac21/mtr.RqsTwkMem")
   dbLoadTemplate ("dbLoad_endstation_pmac21/mtr.LimTransfer")

### Coordinate System Databases
   dbLoadTemplate ("dbLoad_endstation_pmac21/pcs.bkgfix1pcs")
   dbLoadTemplate ("dbLoad_endstation_pmac21/pcs.CtlPnlPcs")
   dbLoadTemplate ("dbLoad_endstation_pmac21/pcs.AsciiPcs")

### Assembly Databases:

### Common assembly:
   dbLoadTemplate ("dbLoad_endstation_pmac21/assy.AssyGeneric")
   dbLoadTemplate ("dbLoad_endstation_pmac21/assy.MotionPrg")
   dbLoadTemplate ("dbLoad_endstation_pmac21/assy.RamMap")
   dbLoadTemplate ("dbLoad_endstation_pmac21/assy.Load")

#### Different Assembly Types:
   dbLoadTemplate ("dbLoad_endstation_pmac21/hs.Assy")
   dbLoadTemplate ("dbLoad_endstation_pmac21/x.Assy")
   dbLoadTemplate ("dbLoad_endstation_pmac21/xy.Assy")
   dbLoadTemplate ("dbLoad_endstation_pmac21/xyz.Assy")

