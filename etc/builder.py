from iocbuilder import Device, records, RecordFactory, SetSimulation
from iocbuilder.arginfo import *
from iocbuilder.modules.asyn import AsynPort, Asyn
from iocbuilder.modules.motor import MotorLib

__all__ = ['GeoBrick']


class Tpmac(Device):
    Dependencies = (Asyn, MotorLib)
    AutoInstantiate = True

class DeltaTauCommsPort(AsynPort):
    "AsynPort that communicates with a delta tau motor controller"
    pass


class DeltaTau(AsynPort):
    "Asyn Motor Port that we can attach motor records to"
    pass


class pmacAsynIPPort(DeltaTauCommsPort):
    """This will create an AsynPort connecting to a PMAC or GeoBrick over IP"""
    LibFileList = ['pmacAsynIPPort']
    DbdFileList = ['pmacAsynIPPort']
    _Cards = []

    def __init__(self, name, IP, simulation=None):
        if ':' not in IP:
            IP = IP + ':1025'
        self.IP = IP
        self.name = name
        # init the AsynPort superclass
        self.__super.__init__(name)

    def Initialise(self):
        print '# Create IP Port (PortName, IPAddr)'
        print 'pmacAsynIPConfigure("%(name)s", "%(IP)s")' % \
            self.__dict__

    ArgInfo = makeArgInfo(__init__,
        name   = Simple('Port Name, normally something like BRICK1port', str),
        IP     = Simple('IP address of the geobrick', str),
        simulation   = Simple('IP port to connect to if in simulation mode', str))

# Sim requires python simulator running on simulation port
def pmacAsynIPPort_sim(name, IP, simulation=None, pmacAsynIPPort=pmacAsynIPPort):
    if simulation:
        return pmacAsynIPPort(name, simulation)
SetSimulation(pmacAsynIPPort, pmacAsynIPPort_sim)


class pmacVmeConfig(DeltaTauCommsPort):
    """This will create an AsynPort connecting to a PMAC or GeoBrick over VME"""
    LibFileList = ['pmacIoc']
    DbdFileList = ['pmacInclude']
    _Cards = []

    def __init__(self, Prefix = "PMAC_S", simulation=None, priority=0):
        # Now add self to list of cards
        self.Card = len(self._Cards)
        self._Cards.append(self)
        self.Prefix = Prefix
        self.priority = priority
        self.vector = self.AllocateIntVector(3)
        assert self.vector == 192 + self.Card * 3, "PMAC should be instantiated first to avoid interrupt clashes, vector = %d"% self.vector
        # init the AsynPort superclass
        self.__super.__init__(Prefix + str(self.Card))

    def Initialise(self):
        if self.Card == 0:
            print 'pmacVmeDebug=1'
            print 'drvPmacDebug=1'
            print '# Configure %d PMAC cards' % len(self._Cards)
        print 'pmacVmeConfig(%d, 0x%dfa000, 0x%d00000, 0xC%d, %d)' % (self.Card, self.Card+7, self.Card+7, 3*self.Card+1, self.Card+3)
        if self.Card == len(self._Cards) - 1:
            print '# Startup driver for DPRAM ASCII buffer'
            print 'pmacDrv()'
            print 'pmacVmeDebug=0'
            print 'drvPmacDebug=0'
            print 'pmacAsynConfig(0, "%s", %d)' % (self.Prefix, self.priority)

    ArgInfo = makeArgInfo(__init__,
        Prefix = Simple('Prefix for asyn port name, Default of PMAC_S will give PMAC_S0, PMAC_S1, etc.', str),
        priority = Simple('Priority to give the asyn serial ports', int),
        simulation    = Simple('IP port to connect to if in simulation mode', str))

# Sim requires python simulator running on simulation port
def pmacVmeConfig_sim(Prefix = "PMAC_S", simulation=None, pmacAsynIPPort=pmacAsynIPPort, pmacVmeConfig=pmacVmeConfig):
    # Now add self to list of cards
    name = Prefix + str(len(pmacVmeConfig._Cards))
    pmacVmeConfig._Cards.append(name)
    if simulation:
        return pmacAsynIPPort(name, simulation)
SetSimulation(pmacVmeConfig, pmacVmeConfig_sim)


class GeoBrick(DeltaTau):
    """This will create an asyn motor port for a GeoBrick that we can attach
    motor records to"""
    LibFileList = ['pmacAsynMotor']
    DbdFileList = ['pmacAsynMotor']
    Dependencies = (Tpmac,)
    _Cards = []

    def __init__(self, Port, name = None, NAxes = 8, IdlePoll = 500,
            MovingPoll = 50):
        # First create an asyn IP port to connect to
        self.PortName = Port.DeviceName()
        # Now add self to list of cards
        self.Card = len(self._Cards)
        self._Cards.append(self)
        if name is None:
            name = "BRICK%d" % (self.Card + 1)
        # Store other attributes
        self.NAxes = NAxes
        self.IdlePoll = IdlePoll
        self.MovingPoll = MovingPoll
        # init the AsynPort superclass
        self.__super.__init__(name)

    # __init__ arguments
    ArgInfo = makeArgInfo(__init__,
        name = Simple('Name to use for the asyn port', str),
        Port       = Ident('pmacAsynIPPort/pmacVmeConfig to connect to', DeltaTauCommsPort),
        NAxes      = Simple('Number of axes', int),
        IdlePoll   = Simple('Idle Poll Period in ms', int),
        MovingPoll = Simple('Moving Poll Period in ms', int))

    def Initialise(self):
        print '# Create asyn motor port (AsynPort, Addr, BrickNum, NAxes)'
        print 'pmacAsynMotorCreate("%(PortName)s", 0, %(Card)d, %(NAxes)d)' % \
            self.__dict__
        print '# Configure GeoBrick (MotorPort, DriverName, BrickNum, NAxes+1)'
        print 'drvAsynMotorConfigure("%s", "pmacAsynMotor", %d, %d)' % (
            self.DeviceName(), self.Card, self.NAxes+1)
        print 'pmacSetIdlePollPeriod(%(Card)d, %(IdlePoll)d)' % self.__dict__
        print 'pmacSetMovingPollPeriod(%(Card)d, %(MovingPoll)d)' % \
            self.__dict__

    def channel(self, channel):
        assert 0 <= channel < self.NAxes, \
               'Channel %d out of range' % channel
        return _GeoBrickChannel(self.DeviceName(), channel)


class _GeoBrickChannel:
    def __init__(self, device, channel):
        self.motor = RecordFactory(
            records.motor, 'asynMotor', 'OUT',
            '@asyn(%s,%d)' % (device, channel + 1))

class GeoBrick3(DeltaTau):
    """This will create an asyn motor port for a GeoBrick that we can attach
    motor records to using the model 3 driver"""
    LibFileList = ['pmacAsynMotorPort']
    DbdFileList = ['pmacAsynMotorPort']
    Dependencies = (Tpmac,)
    _Cards = []

    def __init__(self, Port, name = None, NAxes = 8, IdlePoll = 500,
            MovingPoll = 50):
        # First create an asyn IP port to connect to
        self.PortName = Port.DeviceName()
        # Now add self to list of cards
        self.Card = len(self._Cards)
        self._Cards.append(self)
        if name is None:
            name = "BRICK%d" % (self.Card + 1)
        self.name = name
        # Store other attributes
        self.NAxes = NAxes
        self.IdlePoll = IdlePoll
        self.MovingPoll = MovingPoll
        # init the AsynPort superclass
        self.__super.__init__(name)

    # __init__ arguments
    ArgInfo = makeArgInfo(__init__,
        name = Simple('Name to use for the asyn port', str),
        Port       = Ident('pmacAsynIPPort/pmacVmeConfig to connect to', DeltaTauCommsPort),
        NAxes      = Simple('Number of axes', int),
        IdlePoll   = Simple('Idle Poll Period in ms', int),
        MovingPoll = Simple('Moving Poll Period in ms', int))

    def Initialise(self):
        print '# Configure Model 3 Controller Driver (Controler Port,Asyn Motor Port, ADDR, Axes, MOVE_POLL, IDLE_POLL)'
        print 'pmacCreateController("%(name)s", "%(PortName)s", 0, %(NAxes)d, %(MovingPoll)d, %(IdlePoll)d)' % self.__dict__
        print '# Configure Model 3 Axes Driver (Controler Port, Axis Count)'
        print 'pmacCreateAxes("%(name)s", %(NAxes)d)' % self.__dict__

class PMAC(GeoBrick):
    """This will create an asyn motor port for a PMAC that we can attach
    motor records to"""
    def __init__(self, NAxes = 32, MovingPoll = 200, **args):
        self.__super.__init__(NAxes = NAxes, MovingPoll=MovingPoll, **args)
    ArgInfo = GeoBrick.ArgInfo + makeArgInfo(__init__,
        NAxes      = Simple('Number of axes', int),
        MovingPoll = Simple('Moving Poll Period in ms', int))
    
class PMAC3(GeoBrick3):
    """This will create an asyn motor port for a PMAC that we can attach
    motor records to"""
    def __init__(self, NAxes = 32, MovingPoll = 200, **args):
        self.__super.__init__(NAxes = NAxes, MovingPoll=MovingPoll, **args)
    ArgInfo = GeoBrick3.ArgInfo + makeArgInfo(__init__,
        NAxes      = Simple('Number of axes', int),
        MovingPoll = Simple('Moving Poll Period in ms', int))


class pmacSetAxisScale(Device):
    """Apply an integer scale factor to an axis on the PMAC"""
    def __init__(self, Controller, Axis, Scale):
        # init the superclass
        self.__super.__init__()
        # store args
        self.__dict__.update(locals())
    def Initialise(self):                
        if type(self.Controller) is GeoBrick3:
            # model 3 version of pmacSetAxisScale uses port instead of card 
            self.ControllerPort = self.Controller.DeviceName() 
            print 'pmacDisableLimitsCheck("%(ControllerPort)s", %(Axis)d, 1)' % self.__dict__
            print "pmacSetAxisScale(%(ControllerPort)s, %(Axis)d, %(Scale)d)" % self.__dict__
        else: 
            self.Card = Controller.Card
            print "pmacSetAxisScale(%(Card)d, %(Axis)d, %(Scale)d)" % self.__dict__
    # __init__ arguments
    ArgInfo = makeArgInfo(__init__,
        Controller = Ident ('Underlying PMAC or GeoBrick object', DeltaTau),
        Axis       = Simple('Axis number to apply scale to', int),
        Scale      = Simple('Scale factor the cts will be multiplied by before being passed to motor record', int))


class pmacDisableLimitsCheck(Device):
    Dependencies = (Tpmac,)

    def __init__(self, Controller, Axis = None):
        self.__super.__init__()
        self.Controller = Controller
        self.Axis = Axis

    def Initialise(self):
        if self.Axis is None:
            self.Axis = 0
        
        if type(self.Controller) is GeoBrick3:
            # model 3 version of pmacDisableLimitsCheck uses port instead of card 
            self.ControllerPort = self.Controller.DeviceName() 
            print 'pmacDisableLimitsCheck("%(ControllerPort)s", %(Axis)d, 1)' % self.__dict__
        else: 
            self.Card = Controller.Card
            print 'pmacDisableLimitsCheck(%(Card)d, %(Axis)d, 0)' % self.__dict__

    ArgInfo = makeArgInfo(__init__,
        Controller = Ident ('Underlying PMAC or GeoBrick object', DeltaTau),
        Axis       = Simple(
            'Axis number to disable limit check, defaults to all', int))

class pmacCreateCsGroup(Device):
    Dependencies = (Tpmac,)

    def __init__(self, Controller, GroupNumber, GroupName, AxisCount):
        self.__super.__init__()
        self.Controller = Controller
        self.GroupNumber = GroupNumber
        self.AxisCount = AxisCount
        self.GroupName = GroupName

    def Initialise(self):        
        assert type(self.Controller) is GeoBrick3 or type(self.Controller) is PMAC3, \
            "CsGroup functions are only supported by model 3 drivers Geobrick3, PMAC3"
            # model 3 version of pmacDisableLimitsCheck uses port instead of card 
        print 'pmacCreateCsGroup("%(Controller)s", %(GroupNumber)d, "%(GroupName)s", %(AxisCount)d)' % self.__dict__

    ArgInfo = makeArgInfo(__init__,
        Controller = Ident ('Underlying PMAC3 or GeoBrick3 object', DeltaTau),
        GroupNumber = Simple('Unique Group number to describe this group', int),
        GroupName = Simple('Description of the group', str),
        AxisCount = Simple('Number of CS axes in this group', int))

class pmacCsGroupAddAxis(Device):
    Dependencies = (Tpmac,)

    def __init__(self, Controller, GroupNumber, AxisNumber, AxisDef, CoordSysNumber):
        self.__super.__init__()
        self.Controller = Controller
        self.GroupNumber = GroupNumber
        self.AxisNumber = AxisNumber
        self.AxisDef = AxisDef
        self.CoordSysNumber = CoordSysNumber

    def Initialise(self):        
        assert type(self.Controller) is GeoBrick3 or type(self.Controller) is PMAC3, \
            "CsGroup functions are only supported by model 3 drivers Geobrick3, PMAC3"
            # model 3 version of pmacDisableLimitsCheck uses port instead of card 
        print 'pmacCsGroupAddAxis(%(Controller)s, %(GroupNumber)d, %(AxisNumber)d, %(AxisDef)s, %(CoordSysNumber)d)' % self.__dict__

    ArgInfo = makeArgInfo(__init__,
        Controller = Ident ('Underlying PMAC3 or GeoBrick3 object', DeltaTau),
        GroupNumber = Simple('Unique Group number to describe this group', int),
        AxisNumber = Simple('Axis number of axis to add to the group', int),
        AxisDef = Simple('CS Axis definition for this axis i.e. one of I A B C U V W X Y Z (or may inlcude linear equations)', str),
        CoordSysNumber = Simple('Axis number of axis to add to the group', int))
