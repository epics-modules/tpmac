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

# pmacVmeConfig (0, 0x7FA000, 0x700000, 0xa1, 6)
# pmacVmeConfig (1, 0x6FA000, 0x600000, 0xa5, 5)

  pmacVmeConfig (0, 0x6FA000, 0x600000, 0xa5, 6)
  pmacVmeConfig (1, 0x7FA000, 0x700000, 0xa1, 1)

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
  dbLoadTemplate ("dbLoad_pmac10_23o/pmac.Ascii")
# dbLoadTemplate ("dbLoad_pmac10_23o/pmac.acc11e")
# dbLoadTemplate ("dbLoad_pmac10_23o/pmac.acc65e")

### Motor Databases
  dbLoadTemplate ("dbLoad_pmac10_23o/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_pmac10_23o/mtr.Ascii")
  dbLoadTemplate ("dbLoad_pmac10_23o/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_pmac10_23o/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_pmac10_23o/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_pmac10_23o/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_pmac10_23o/pcs.AsciiPcs")

### Assembly Databases

### Common assembly:
  dbLoadTemplate ("dbLoad_pmac10_23o/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_pmac10_23o/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_pmac10_23o/assy.RamMap")
  dbLoadTemplate ("dbLoad_pmac10_23o/assy.Load")

#### Huber-type SLIT Databases (WS:Av:, WS:Ah:)
#### MIRROR Databases (HD:Up:, HD:Dn:)
#### MONO Databases (MO:En:)
#### TABLE Databases (HD:St:)
#### X positioners Databases (MO:Tn1:, BP:Mo:, BP:Hd1:)
#### XY positioners Databases (MO:Ps:, MO:Tn2:)
  dbLoadTemplate ("dbLoad_pmac10_23o/hs.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23o/mi.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23o/mo.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23o/tb.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23o/x.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23o/xy.Assy")

### PMAC Templates: PMAC11
#=========================

### PMAC Databases
  dbLoadTemplate ("dbLoad_pmac11_23o/pmac.Ascii")
# dbLoadTemplate ("dbLoad_pmac11_23o/pmac.acc11e")

### Motor Databases
  dbLoadTemplate ("dbLoad_pmac11_23o/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_pmac11_23o/mtr.Ascii")
  dbLoadTemplate ("dbLoad_pmac11_23o/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_pmac11_23o/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_pmac11_23o/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_pmac11_23o/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_pmac11_23o/pcs.AsciiPcs")

### Assembly Databases

### Common assembly:
  dbLoadTemplate ("dbLoad_pmac11_23o/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_pmac11_23o/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_pmac11_23o/assy.RamMap")
  dbLoadTemplate ("dbLoad_pmac11_23o/assy.Load")

#### Collimator (Piggyback) SLIT Databases (CS:Av:, CS:Ah:)
#### MIRROR Databases (KB:Hy:, KB:Vy:)
#### TABLE Databases (KB:Hz:, KB:Vz:)
#### X positioners Databases (BP:Hd2:, BP:Kb:)
  dbLoadTemplate ("dbLoad_pmac11_23o/cs.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23o/mi.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23o/tb.Assy")
  dbLoadTemplate ("dbLoad_pmac11_23o/x.Assy")

