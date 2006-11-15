### Configure PMAC-VME Hardware
###	args:	(1) EPICS VME Card #
###		(2) PMAC Base Address
###		(3) PMAC DPRAM Address (0=none)
###		(4) PMAC IRQ Vectors (v must be odd) (uses v-1,v,v+1)
###		(5) IRQ Level
pmacVmeDebug
devPmacMbxDebug
devPmacRamDebug
drvPmacDebug
# pmacSerialDebug

### For 32-bit PMAC settings:
### DPRAM address range is A32 0xB0700000 - 0xB0703FFF
### MBOX  address range is A32 0xB07FA000 - 0xB07FA1FF
# pmacVmeConfig (0, 0xB07FA000, 0xB0700000, 0xa1, 2)
# pmacVmeConfig (1, 0xB07FA200, 0xB0704000, 0xa5, 2)
# pmacVmeConfig (2, 0xB07FA400, 0xB0708000, 0xa9, 2)


### For 24-bit PMAC settings:
### DPRAM address range is A24 0xF0700000 - 0xF0703FFF
### MBOX  address range is A24 0xF07FA000 - 0xF07FA1FF
# pmacVmeConfig (0, 0x6FA000, 0x600000, 0xa5, 6)
  pmacVmeConfig (1, 0x7FA000, 0x700000, 0xa1, 1)

### Configure PMAC-VME Driver
### 	args:	(1) EPICS VME Card #
###		(2) DPRAM Servo Fixed Buffer Scan Rate
### 			(ticks, 0=default, -1=disabled, -2=interrupt)
### 		(3) DPRAM Background Fixed Buffer Scan Rate
###			(ticks, 0=default, -1=disabled, -2=interrupt)
###		(4) DPRAM Background Variable Buffer Scan Rate
###			(ticks, 0=default, -1=disabled, -2=interrupt)
### 		(5) Mailbox Registers Disable
###			(0=default, 0=enable/-1=disable)
# pmacDrvConfig (0, 0, 0, 0, 0)
  pmacDrvConfig (1, 0, 0, 0, 0)
# pmacDrvConfig (2, 0, 0, 0, 0)

### PMAC Templates for pmac21
#============================

### PMAC Databases
  dbLoadTemplate ("dbLoad_14xy-2hs/pmac.Ascii")

### Motor Databases
  dbLoadTemplate ("dbLoad_14xy-2hs/mtr.mtrdat")
  dbLoadTemplate ("dbLoad_14xy-2hs/mtr.Ascii")
  dbLoadTemplate ("dbLoad_14xy-2hs/mtr.RqsTwkMem")
  dbLoadTemplate ("dbLoad_14xy-2hs/mtr.LimTransfer")

### Coordinate System Databases
  dbLoadTemplate ("dbLoad_14xy-2hs/pcs.bkgfix1pcs")
  dbLoadTemplate ("dbLoad_14xy-2hs/pcs.CtlPnlPcs")
  dbLoadTemplate ("dbLoad_14xy-2hs/pcs.AsciiPcs")

### Assembly Databases:

### Common assembly:
  dbLoadTemplate ("dbLoad_14xy-2hs/assy.AssyGeneric")
  dbLoadTemplate ("dbLoad_14xy-2hs/assy.MotionPrg")
  dbLoadTemplate ("dbLoad_14xy-2hs/assy.RamMap")
  dbLoadTemplate ("dbLoad_14xy-2hs/assy.Load")

#### Different Assembly Types:
  dbLoadTemplate ("dbLoad_14xy-2hs/hs.Assy")
  dbLoadTemplate ("dbLoad_14xy-2hs/xy.Assy")

