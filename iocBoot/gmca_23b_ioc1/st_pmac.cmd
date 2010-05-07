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


# 24-bit PMAC settings:
# DPRAM address range is A24 0xF0700000 - 0xF0703FFF
# MBOX  address range is A24 0xF07FA000 - 0xF07FA1FF

  pmacVmeConfig (0, 0x6FA000, 0x600000, 0xc1, 5)
  pmacVmeConfig (1, 0x7FA000, 0x700000, 0xc5, 6)

### Configure PMAC-VME Driver
### This uses MAILBOX:
# pmacAsynConfig( "PMAC_MBX_PORT_", 0 )
### This uses DPRAM ASCII:
  pmacAsynConfig( 0, "PMAC_MBX_PORT_" )

# 	args:	(1) EPICS VME Card #
#		(2) DPRAM Servo Fixed Buffer Scan Rate
# 			(ticks, 0=default, -1=disabled, -2=interrupt)
# 		(3) DPRAM Background Fixed Buffer Scan Rate
#			(ticks, 0=default, -1=disabled, -2=interrupt)
#		(4) DPRAM Background Variable Buffer Scan Rate
#			(ticks, 0=default, -1=disabled, -2=interrupt)
# 		(5) Mailbox Registers Disable
#			(0=default, 0=enable/-1=disable)
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

### PMAC Templates: PMAC10
#=========================

### PMAC Databases
  dbLoadTemplate ("dbLoad_pmac10_23b/pmac.Ascii")
  dbLoadTemplate ("dbLoad_pmac10_23b/pmac.acc59e")
# dbLoadTemplate ("dbLoad_pmac10_23b/pmac.acc11e")
# dbLoadTemplate ("dbLoad_pmac10_23b/pmac.acc65e")

### Motor Databases
  dbLoadTemplate ("dbLoad_pmac10_23b/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_pmac10_23b/mtr.Ascii")
  dbLoadTemplate ("dbLoad_pmac10_23b/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_pmac10_23b/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_pmac10_23b/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_pmac10_23b/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_pmac10_23b/pcs.AsciiPcs")

### Assembly Databases

### Common assembly:
  dbLoadTemplate ("dbLoad_pmac10_23b/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_pmac10_23b/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_pmac10_23b/assy.RamMap")
  dbLoadTemplate ("dbLoad_pmac10_23b/assy.Load")

#### Huber-type SLIT Databases (WS:Av:, WS:Ah:)
#### MIRROR Databases (VC:Vy:)
#### MONO Databases (MO:En:)
#### TABLE Databases (VC:Vz:)
#### X positioners Databases (VC:Be:, MO:Tn1:)
#### Xbm positioners Databases (MO:Ps:)
#### XY positioners Databases (MO:Be:)
#### XYZ positioners Databases (MO:Tn2:)
  dbLoadTemplate ("dbLoad_pmac10_23b/hs.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23b/mi.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23b/tb.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23b/mo.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23b/x.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23b/xbm.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23b/xyz.Assy")

### PMAC Templates: PMAC11
#=========================

### PMAC Databases
  dbLoadTemplate ("dbLoad_pmac11_23b/pmac.Ascii")
# dbLoadTemplate ("dbLoad_pmac11_23b/pmac.acc11e")

### Motor Databases
  dbLoadTemplate ("dbLoad_pmac11_23b/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_pmac11_23b/mtr.Ascii")
  dbLoadTemplate ("dbLoad_pmac11_23b/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_pmac11_23b/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_pmac11_23b/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_pmac11_23b/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_pmac11_23b/pcs.AsciiPcs")

### Assembly Databases

### Common assembly:
  dbLoadTemplate ("dbLoad_pmac11_23b/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_pmac11_23b/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_pmac11_23b/assy.RamMap")
  dbLoadTemplate ("dbLoad_pmac11_23b/assy.Load")

#### Collimator (Piggyback) SLIT Databases (CS:Av:, CS:Ah:)
#### MIRROR Databases (VF:Vy:)
#### TABLE Databases (VF:Vz:)
#### X positioners Databases (BP:Mo:, VF:Be:, BP:Vf:)
#### XY positioners Databases (AT:Av:)
  dbLoadTemplate ("dbLoad_pmac11_23b/cs.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23b/mi.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23b/tb.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23b/x.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23b/xy.Assy")

