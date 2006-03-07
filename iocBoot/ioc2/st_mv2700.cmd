### vxWorks startup script
### Set shell prompt
  shellPromptSet "ioc2> "

### This is routeAdd, hostAdd, hostShow, nfsMount...
#  < ../nfsCommands
  nfsAuthUnixSet "px0", 501, 100
  nfsMount("px0", "/home/gmca/epics/synApps", "/ioc")

  startup   = "/ioc/xxx2/iocBoot/iocgmca2"
  appbin    = "/ioc/xxx2/bin/mv2700"
  db        = "/ioc/xxx2/db"
  std       = "/ioc/std1-3"
  pmac      = "/ioc/tpmac2-0"
  pmac_pmac = "/ioc/tpmac2-0/pmacApp/Db/pmacDb"
  pmac_soft = "/ioc/tpmac2-0/pmacApp/Db/softDb"
  pmac_cs   = "/ioc/tpmac2-0/pmacApp/Db/csDb"
  pmac_hs   = "/ioc/tpmac2-0/pmacApp/Db/hsDb"
  pmac_md   = "/ioc/tpmac2-0/pmacApp/Db/mdDb"
  pmac_mi   = "/ioc/tpmac2-0/pmacApp/Db/miDb"
  pmac_mo   = "/ioc/tpmac2-0/pmacApp/Db/moDb"
  pmac_tb   = "/ioc/tpmac2-0/pmacApp/Db/tbDb"
  pmac_xy   = "/ioc/tpmac2-0/pmacApp/Db/xyDb"
################################################################################
  cd appbin

### If the VxWorks kernel was built using the project facility, the following must
### be added before any C++ code is loaded (see SPR #28980).
  sysCplusEnable=1

### Load EPICS base software
  ld < iocCore

### Load custom EPICS software from xxxApp and from share
  ld < xxxLib
  ld < tpmacLib
  ld < tsubLib

#  routerInit

##############################################################################
### dbrestore setup
  save_restoreSet_Debug(20)
  save_restoreSet_IncompleteSetsOk(1)
  save_restoreSet_DatedBackupFiles(0)
  save_restoreSet_NumSeqFiles(2)
  save_restoreSet_SeqPeriodInSeconds(60)
  set_savefile_path(startup, "autosave")
  set_requestfile_path(startup, "")
  set_pass0_restoreFile("auto_positions.sav")
  set_pass0_restoreFile("auto_settings.sav")
  set_pass1_restoreFile("auto_settings.sav")

 ld < initHooks.o


### override address, interrupt vector, etc. information in module_types.h
  module_types()

### need more entries in wait/scan-record channel-access queue?
  recDynLinkQsize = 1024

#####################
# Hardware Config   #
#####################

  cd startup
################################################################################
### Tell EPICS all about the record types, device-support modules, drivers,
### etc. in the software we just loaded (xxxLib)
  dbLoadDatabase("../../dbd/xxxApp.dbd")

###############################################################################
# PMAC DPRAM address range A32  0xB0700000 - 0xB0703FFF
# PMAC MBOX  address range A32  0xB07FA000 - 0xB07FA1FF
#vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
### Load PMAC databases:
  < st_pmac.cmd
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


  iocLogDisable=1

  iocInit
  taskDelay 60


### save positions every five seconds
#  create_monitor_set("auto_positions.req", 5, "P=23i:2")

### save other things every thirty seconds
#  create_monitor_set("auto_settings.req", 30, "P=23i:2")

### Clear PMAC Macro:
  taskDelay 10
# dbpf ("23i:pmac40:StrCmd","ms$$$0")
# dbpf ("23i:pmac10:StrCmd","ms$$$0")
# taskDelay 120
# dbpf ("23i:pmac10:StrCmd","ena plc 2")

### ioc2: ALL DONE!
### Enter <^X> or <reboot> to reboot
