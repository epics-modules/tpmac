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

#### For any 1-2-3-motor assemblies:
  dbLoadTemplate ("dbLoad/assy.Ram06X2",     pmac_pmac)
  dbLoadTemplate ("dbLoad/assy.Load06X2",    pmac_soft)
  dbLoadTemplate ("dbLoad/assy.Ram08XY2",    pmac_pmac)
  dbLoadTemplate ("dbLoad/assy.Load08XY2",   pmac_soft)
  dbLoadTemplate ("dbLoad/assy.Ram10XYZ2",   pmac_pmac)
  dbLoadTemplate ("dbLoad/assy.Load10XYZ2",  pmac_soft)

#### Collimator (Piggyback) SLIT Databases (CS:Av:, CS:Ah:)
#### ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/cs.Calibrate",     pmac_cs)
  dbLoadTemplate ("dbLoad/cs.Tsub",          pmac_cs)
  dbLoadTemplate ("dbLoad/cs.TsubRbk",       pmac_cs)
  dbLoadTemplate ("dbLoad/cs.LimAmp",        pmac_cs)

#### Huber-type SLIT Databases (WS:Av:, WS:Ah:)
#### ^^^^^^^^^^^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/hs.Calibrate",     pmac_hs)
  dbLoadTemplate ("dbLoad/hs.Tsub",          pmac_hs)
  dbLoadTemplate ("dbLoad/hs.TsubRbk",       pmac_hs)
  dbLoadTemplate ("dbLoad/hs.LimAmp",        pmac_hs)

#### MODULAR Databases (MD:Tn1:, BP:Mo:, BP:Kb:)
#### ^^^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/md.Calibrate",     pmac_md)
  dbLoadTemplate ("dbLoad/md.Tsub",          pmac_md)
  dbLoadTemplate ("dbLoad/md.TsubRbk",       pmac_md)
  dbLoadTemplate ("dbLoad/md.LimAmp",        pmac_md)

#### MIRROR Databases (KB:Hy:, KB:Vy:)
#### ^^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/mi.Calibrate",     pmac_mi)
  dbLoadTemplate ("dbLoad/mi.Tsub",          pmac_mi)
  dbLoadTemplate ("dbLoad/mi.TsubRbk",       pmac_mi)
  dbLoadTemplate ("dbLoad/mi.LimAmp",        pmac_mi)

#### MONO Databases (MO:En:)
#### ^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/mo.Calibrate",     pmac_mo)
  dbLoadTemplate ("dbLoad/mo.Tsub",          pmac_mo)
  dbLoadTemplate ("dbLoad/mo.TsubRbk",       pmac_mo)
  dbLoadTemplate ("dbLoad/mo.LimAmp",        pmac_mo)

#### TABLE Databases (KB:Hz:, KB:Vz:)
#### ^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/tb.Calibrate",     pmac_tb)
  dbLoadTemplate ("dbLoad/tb.Tsub",          pmac_tb)
  dbLoadTemplate ("dbLoad/tb.TsubRbk",       pmac_tb)
  dbLoadTemplate ("dbLoad/tb.LimAmp",        pmac_tb)

#### XY positioners Databases (MO:Ps:, MO:Tn2:)
#### ^^^^^^^^^^^^^^^^^^^^^^^^
  dbLoadTemplate ("dbLoad/xy.Calibrate",     pmac_xy)
  dbLoadTemplate ("dbLoad/xy.Tsub",          pmac_xy)
  dbLoadTemplate ("dbLoad/xy.TsubRbk",       pmac_xy)
  dbLoadTemplate ("dbLoad/xy.LimAmp",        pmac_xy)

