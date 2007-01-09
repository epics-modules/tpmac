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
  pmacVmeConfig (0, 0x7FA000, 0x700000, 0xa1, 2)
# pmacVmeConfig (1, 0x7FA200, 0x704000, 0xa5, 2)
# pmacVmeConfig (2, 0x7FA400, 0x708000, 0xa9, 2)

### Configure PMAC-VME Driver
### This uses MAILBOX:
# pmacAsynConfig( "PMAC_MBX_PORT_", 0 )
### This uses DPRAM ASCII:
  pmacAsynConfig( 0, "PMAC_MBX_PORT_" )

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
# pmacDrvConfig (0, 0, 0, 0, 0)
# pmacDrvConfig (1, 0, 0, 0, 0)
# pmacDrvConfig (2, 0, 0, 0, 0)

# pmacDrvConfig (
#	int		cardNumber,
#	int		scanMtrRate,
#	int		scanBkgRate,
#	int		scanVarRate,
#	char *		asynMbxPort)

  pmacDrvConfig (0, 0, 0, 0, "PMAC_MBX_PORT_0" )
# pmacDrvConfig (1, 0, 0, 0, "PMAC_MBX_PORT_1" )


### PMAC Templates
#=================

### PMAC Databases
  dbLoadTemplate ("dbLoad_pmac10_23i/pmac.Ascii")
# dbLoadTemplate ("dbLoad_pmac10_23i/pmac.acc65e")
  dbLoadTemplate ("dbLoad_pmac10_23i/pmac.acc11e")

### Motor Databases
  dbLoadTemplate ("dbLoad_pmac10_23i/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_pmac10_23i/mtr.Ascii")
  dbLoadTemplate ("dbLoad_pmac10_23i/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_pmac10_23i/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_pmac10_23i/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_pmac10_23i/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_pmac10_23i/pcs.AsciiPcs")

### Assembly Databases

### Common assembly:
  dbLoadTemplate ("dbLoad_pmac10_23i/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_pmac10_23i/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_pmac10_23i/assy.RamMap")
  dbLoadTemplate ("dbLoad_pmac10_23i/assy.Load")

#### Collimator (Piggyback) SLIT Databases (CS:Av:, CS:Ah:)
#### Huber-type SLIT Databases (WS:Av:, WS:Ah:)
#### MIRROR Databases (KB:Hy:, KB:Vy:)
#### MONO Databases (MO:En:)
#### TABLE Databases (KB:Hz:, KB:Vz:)
#### X positioners Databases (MO:Tn1:, BP:Mo:, BP:Kb:)
#### XY positioners Databases (MO:Ps:, MO:Tn2:)
  dbLoadTemplate ("dbLoad_pmac10_23i/cs.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23i/hs.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23i/mi.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23i/mo.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23i/tb.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23i/x.Assy")
  dbLoadTemplate ("dbLoad_pmac10_23i/xy.Assy")

