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

  pmacVmeConfig (0, 0xB07FA000, 0xB0700000, 0xa1, 2)
# DPRAM address range is A32 0xB0700000 - 0xB0703FFF
# MBOX  address range is A32 0xB07FA000 - 0xB07FA1FF

# pmacVmeConfig (1, 0x7FA200, 0x704000, 0xa5, 2)
# pmacVmeConfig (2, 0x7FA400, 0x708000, 0xa9, 2)

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
  pmacDrvConfig (0, 0, 0, 0, 0)
# pmacDrvConfig (1, 0, 0, 0, 0)
# pmacDrvConfig (2, 0, 0, 0, 0)

### PMAC Templates
#=================

### PMAC Databases
  dbLoadTemplate ("dbLoad/pmac.acc11e",      pmac_pmac)
  dbLoadTemplate ("dbLoad/pmac.svofix0",     pmac_pmac)
  dbLoadTemplate ("dbLoad/pmac.bkgfix0",     pmac_pmac)
  dbLoadTemplate ("dbLoad/pmac.bkgvar0",     pmac_pmac)
  dbLoadTemplate ("dbLoad/pmac.mbxvar0",     pmac_pmac)
  dbLoadTemplate ("dbLoad/pmac.AsciiPmac",   pmac_pmac)

### Motor Databases
  dbLoadTemplate ("dbLoad/mtr.mtrdat",       pmac_pmac)
  dbLoadTemplate ("dbLoad/mtr.CtlPnlMtr",    pmac_pmac)
  dbLoadTemplate ("dbLoad/mtr.AsciiMtr",     pmac_pmac)
  dbLoadTemplate ("dbLoad/mtr.IxMtrMv",      pmac_pmac)
  dbLoadTemplate ("dbLoad/mtr.OpnLp",        pmac_pmac)
  dbLoadTemplate ("dbLoad/mtr.RqsTwkMem",    pmac_soft)
  dbLoadTemplate ("dbLoad/mtr.LimTransfer",  pmac_soft)

### Coordinate System Databases
  dbLoadTemplate ("dbLoad/pcs.bkgfix1pcs",   pmac_pmac)
  dbLoadTemplate ("dbLoad/pcs.CtlPnlPcs",    pmac_pmac)
  dbLoadTemplate ("dbLoad/pcs.AsciiPcs",     pmac_pmac)

### Assembly Databases

### Common assembly:
  dbLoadTemplate ("dbLoad/assy.AssyGeneric", pmac_soft)
  dbLoadTemplate ("dbLoad/assy.MotionPrg",   pmac_soft)

#### For any 2-motor assemblies:
  dbLoadTemplate ("dbLoad/assy.Ram08XY2",    pmac_pmac)
  dbLoadTemplate ("dbLoad/assy.Load08XY2",   pmac_soft)


#### XY positioners Databases
#### ^^^^^^^^^^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/xy.Calibrate",     pmac_xy)
  dbLoadTemplate ("dbLoad/xy.Tsub",          pmac_xy)
  dbLoadTemplate ("dbLoad/xy.TsubRbk",       pmac_xy)
  dbLoadTemplate ("dbLoad/xy.LimAmp",        pmac_xy)

#### SLIT Databases
#### ^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/hs.Calibrate",     pmac_hs)
  dbLoadTemplate ("dbLoad/hs.Tsub",          pmac_hs)
  dbLoadTemplate ("dbLoad/hs.TsubRbk",       pmac_hs)
  dbLoadTemplate ("dbLoad/hs.LimAmp",        pmac_hs)

