### vxWorks startup script
  loginUserAdd "gmca", "RceeQSdRSb"
# loginUserDelete "vw5"
  shellPromptSet "23i:ioc1> "

### This is routeAdd, hostAdd, hostShow, nfsMount...
  nfsAuthUnixSet "bl1dl380lower", 500, 100
  nfsMount "bl1dl380lower", "/home/gmca/epics_synApps/synApps_5_2", "/ioc"

### Add gateway to be visible from lab network:
### routeAdd   "destaddr",      "gateaddr"
  routeAdd  "164.54.210.0",    "164.54.210.65"
  routeAdd  "164.54.210.128",  "164.54.210.65"
# routeShow

  < iocEnv

  putenv ("EPICS_TS_NTP_INET=164.54.210.2")

### This is needed for Derek's feedback:
  putenv ("EPICS_CA_ADDR_LIST=164.54.210.2")
  printf "EPICS_CA_ADDR_LIST=%s\n",getenv("EPICS_CA_ADDR_LIST")
################################################################################

### If the VxWorks kernel was built using the project facility, the following must
### be added before any C++ code is loaded (see SPR #28980).
  sysCplusEnable=1

### Load EPICS software
  cd appbin
  ld < xxx.munch

### Increase size of buffer for error logging from default 1256
  errlogInit(5000)
################################################################################
### Register all support components
  cd startup
  dbLoadDatabase("../../dbd/iocxxxVX.dbd")
  iocxxxVX_registerRecordDeviceDriver(pdbbase)

  iocLogDisable=1

### override address, interrupt vector, etc. information in module_types.h
#  module_types()

################################################################################
### IP stuff: 
  < st_ip.cmd

##############################################################################
### save_restore setup
### status-PV prefix
  save_restoreSet_status_prefix("23i:ioc1")
### Debug-output level
  save_restoreSet_Debug(0)

### Ok to save/restore save sets with missing values (no CA connection to PV)?
  save_restoreSet_IncompleteSetsOk(1)
### Save dated backup files?
  save_restoreSet_DatedBackupFiles(0)

### Number of sequenced backup files to write
  save_restoreSet_NumSeqFiles(3)
### Time interval between sequenced backups
  save_restoreSet_SeqPeriodInSeconds(600)

### If you want save_restore to manage its own NFS mount, specify the name and
### IP address of the file server to which save files should be written.
### This currently is supported only on vxWorks.
### save_restoreSet_NFSHost("oxygen", "164.54.52.4")

### specify where save files should be
  set_savefile_path(startup, "autosave")

### Specify what save files should be restored.  Note these files must be
### in the directory specified in set_savefile_path(), or, if that function
### has not been called, from the directory current when iocInit is invoked.
### Specify which save files are to be restored before record initialization (pass 0)
### and which are to be restored after record initialization (pass 1)
### This is the default: restore positios before record init and settings both before and after:
# set_pass0_restoreFile("auto_positions.sav")
  set_pass0_restoreFile("auto_settings.sav")
  set_pass1_restoreFile("auto_settings.sav")
# set_pass0_restoreFile("triggered_settings.sav")

### specify directories in which to to search for included request files
  set_requestfile_path(startup, "dbSave")

### Save_restore status database:
  dbLoadRecords("$(AUTOSAVE)/save_restoreStatus.db", "P=23i:ioc1")
  dbLoadRecords("$(GMCA)/generic_bo.db", "BL=23i,NAME=ioc1:saveTrigger")

###############################################################################
### Scan-support software
### crate-resident scan.  This executes 1D, 2D, 3D, and 4D scans, and caches
### 1D data, but it doesn`t store anything to disk.  (See 'saveData' below
### for that.)
  dbLoadRecords("$(SSCAN)/scan.db","P=23i:1:,MAXPTS1=2000,MAXPTS2=20,MAXPTS3=1,MAXPTS4=1,MAXPTSH=2000")

###############################################################################
### Acromag AVME9440 setup parameters:
### devAvem9440Config (ncards,a16base,intvecbase)
### uses 8 vectors
  devAvme9440Config(1, 0x2800, 0x78)

### Acromag 9440 digital IO records:
### - Outpot Channels 0--15 can provide digital output
### - Input  Channels 0--7 are scanned in I/O interrupt mode
### - Input  Channels 8--15 are scanned in passive mode
### - Input  Channels 16--31 provide digital output readback
### ATTENTION: to use interrupts, set interrupt level to 5 (jumper J4: 1&2, 5&6 IN))

  dbLoadTemplate("digital_IO.substitutions")

# devAvme9440Debug=5

#vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
### Load Keithley-428 Current Amplifier databases:
# < st_keithley.cmd
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
### Load PMAC databases:
  < st_pmac.cmd
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

###############################################################################
  dbLoadRecords("$(VXSTATS)/vxStats-template.db", "IOCNAME=23i:1")

###############################################################################
# reboot_restoreDebug=1
# reboot_restoreDebug=5
# save_restoreDebug=1
# save_restoreDebug=5

  iocInit

  taskDelay 240
# create_monitor_set("auto_positions.req", 5, "P=23i:1")
# create_monitor_set("auto_settings.req", 30, "P=23i:1")
# create_periodic_set("auto_settings.req", 600, "P=23i:ioc1")
# create_triggered_set("triggered_settings.req", "23i:ioc1:saveTrigger.PROC", "P=23i:ioc1")
  create_monitor_set("auto_positions.req", 1800, "P=23i:ioc1")
  create_monitor_set("auto_settings.req", 18000, "P=23i:ioc1")
  create_triggered_set("auto_settings.req", "23i:ioc1:saveTrigger.PROC", "P=23i:ioc1")

### Clear Macro (now moved to home script based on I39 status):
# taskDelay 10
# dbpf ("23i:pmac10:StrCmd","ms$$$0")
# taskDelay 10
# dbpf ("23i:pmac10:StrCmd","ms$$$32")
# taskDelay 10

### Clear Macro Faults:
  taskDelay 30
  dbpf ("23i:pmac10:StrCmd","msclrf0")
  taskDelay 30
  dbpf ("23i:pmac10:StrCmd","msclrf32")
  taskDelay 30

### Increase precision for Mono rotary:
# dbpf ("23i:MO:mr:ActPos.PREC","2")
# taskDelay 10
# dbpf ("23i:MO:mr:RqsPos.PREC","2")
### Change slope for mono rotary to 1/(32*500):
### (the default is 1/(32*96)=3.2552E-04 -- see mtrdat.db)
# taskDelay 10
# dbpf ("23i:MO:mr:ActPos.ASLO","0.0000625")
# taskDelay 10
# dbpf ("23i:MO:mr:ActVelSvo.ASLO","0.0000625")
# taskDelay 10
# dbpf ("23i:MO:mr:FolErr.ASLO","0.0000625")

### Disable Telnet:
# ts tTelnetd
############################# END 23i:ioc3 ##########################
### ALL DONE!
### Enter ^X or "reboot" to reboot!
### Debugging: dbcar(0,5), i, dbgrep
### Also use: memShow, bootChange, lkup, dbgrep, i, ts, tr
###



