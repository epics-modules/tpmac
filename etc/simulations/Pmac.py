
from PmacParser import *
from PmacMotor import *
from PmacUser import *
from PmacUi import *
import os, sys, math

class BreakPoint(object):

    def __init__(self, fileName, lineNumber):
        self.fileName = fileName
        self.lineNumber = lineNumber
        self.name = '%s:%s' % (fileName, lineNumber)

    def getName(self):
        return self.name

class Buffer(object):

    def __init__(self):
        self.tokens = []

    def getTokens(self):
        return self.tokens

    def clear(self):
        self.tokens = []

    def add(self, token):
        self.tokens.append(token)

    def empty(self):
        return len(self.tokens) == 0

    def getString(self):
        result = ''
        for token in self.tokens:
            result += str(token) + ' '
        return result

class CoordinateSystem(PmacParser):

    def __init__(self, pmac, id):
        PmacParser.__init__(self, pmac)
        self.id = id
        self.forward = Buffer()
        self.inverse = Buffer()
        self.axisDef = {}     # Axis definitions, index by motor number
        self.xAxis = 0.0
        self.yAxis = 0.0
        self.zAxis = 0.0
        self.aAxis = 0.0
        self.bAxis = 0.0
        self.cAxis = 0.0
        self.uAxis = 0.0
        self.vAxis = 0.0
        self.wAxis = 0.0
        self.currentProg = None
        self.programRunning = False
        self.pmac = pmac

    def abort(self):
        self.programRunning = False

    def getId(self):
        return self.id

    def getStatus(self):
        status1 = 0
        status2 = 0
        status3 = 0
        if self.isInPosition():
            status2 |= 0x020000
        return "%06X%06X%06X" % (status1, status2, status3)

    def isInPosition(self):
        # Returns true if all motors assigned to this CS are in position
        result = True
        for n,axisDef in self.axisDef.iteritems():
            motor = self.pmac.getMotor(n)
            if motor is not None:
                result = result and motor.isInPosition()
        return result

    def moveAxis(self, moves):
        for move in moves:
            axis = move[0]
            pos = move[1]
            if axis == 'x':
                self.xAxis = pos
            elif axis == 'y':
                self.yAxis = pos
            elif axis == 'z':
                self.zAxis = pos
            elif axis == 'a':
                self.aAxis = pos
            elif axis == 'b':
                self.bAxis = pos
            elif axis == 'c':
                self.cAxis = pos
            elif axis == 'u':
                self.uAxis = pos
            elif axis == 'v':
                self.vAxis = pos
            elif axis == 'w':
                self.wAxis = pos
        self.currentCoordSystem = PmacToken(str(self.id),'',0)
        self.executeInverse()
        self.processAxisDefs()

    def processAxisDefs(self):
        getUi(self.pmac).output("Process axis defs %s\n" % self.axisDef)
        for n,axisDef in self.axisDef.iteritems():
            motor = self.pmac.getMotor(n)
            if motor is not None:
                self.tokens = axisDef
                self.debug = True
                token = self.getToken()
                if token == 'i':
                    offset = 0.0
                    token = self.getToken()
                    if token is not None:
                        if token == '-':
                            token = self.getToken()
                            offset = -self.tokenToFloat(token)
                        else:
                            offset = self.tokenToFloat(token)
                    motor.jog(self.pmac.getPVariable(n) + offset)
                else:
                    pos = 0.0
                    while token is not None:
                        scaleFactor = 1.0
                        if self.tokenIsFloat(token):
                            scaleFactor = self.tokenToFloat(token)
                            token = self.getToken()
                        elif token == '-':
                            value = self.getToken()
                            if self.tokenIsFloat(value):
                                scaleFactor = -self.tokenToFloat(value)
                            else:
                                scaleFactor = -1.0
                                self.putToken(value)
                            token = self.getToken()
                        elif token == '+':
                            value = self.getToken()
                            if self.tokenIsFloat(value):
                                scaleFactor = self.tokenToFloat(value)
                            else:
                                self.putToken(value)
                            token = self.getToken()
                        if token is None:
                            pos += scaleFactor
                        elif token == 'x':
                            pos += scaleFactor * self.xAxis
                        elif token == 'y':
                            pos += scaleFactor * self.yAxis
                        elif token == 'z':
                            pos += scaleFactor * self.zAxis
                        elif token == 'a':
                            pos += scaleFactor * self.aAxis
                        elif token == 'b':
                            pos += scaleFactor * self.bAxis
                        elif token == 'c':
                            pos += scaleFactor * self.cAxis
                        elif token == 'u':
                            pos += scaleFactor * self.uAxis
                        elif token == 'v':
                            pos += scaleFactor * self.vAxis
                        elif token == 'w':
                            pos += scaleFactor * self.wAxis
                        else:
                            self.putToken(token)
                            pos += scaleFactor
                        token = self.getToken()
                    motor.jog(pos) 

    def transferAxesToQVariables(self):
        self.pmac.setCsQVariable(self.id, 1, self.aAxis)
        self.pmac.setCsQVariable(self.id, 2, self.bAxis)
        self.pmac.setCsQVariable(self.id, 3, self.cAxis)
        self.pmac.setCsQVariable(self.id, 4, self.uAxis)
        self.pmac.setCsQVariable(self.id, 5, self.vAxis)
        self.pmac.setCsQVariable(self.id, 6, self.wAxis)
        self.pmac.setCsQVariable(self.id, 7, self.xAxis)
        self.pmac.setCsQVariable(self.id, 8, self.yAxis)
        self.pmac.setCsQVariable(self.id, 9, self.zAxis)

    def transferQVariablesToAxes(self):
        self.aAxis = self.pmac.getCsQVariable(self.id, 1)
        self.bAxis = self.pmac.getCsQVariable(self.id, 2)
        self.cAxis = self.pmac.getCsQVariable(self.id, 3)
        self.uAxis = self.pmac.getCsQVariable(self.id, 4)
        self.vAxis = self.pmac.getCsQVariable(self.id, 5)
        self.wAxis = self.pmac.getCsQVariable(self.id, 6)
        self.xAxis = self.pmac.getCsQVariable(self.id, 7)
        self.yAxis = self.pmac.getCsQVariable(self.id, 8)
        self.zAxis = self.pmac.getCsQVariable(self.id, 9)

    def executeForward(self):
        if not self.forward.empty():
            self.transferAxesToQVariables()
            self.pmac.transferMotorsToPVariables()
            self.tokens = self.forward.getTokens()
            self.processTokens(noPrint=True, programMode=True)
            self.transferQVariablesToAxes()

    def executeInverse(self):
        if not self.inverse.empty():
            self.transferAxesToQVariables()
            self.tokens = self.inverse.getTokens()
            self.processTokens(noPrint=True, programMode=True)

    def clearForward(self):
        self.forward.clear()

    def addForward(self, token):
        self.forward.add(token)

    def clearInverse(self):
        self.inverse.clear()

    def addInverse(self, token):
        self.inverse.add(token)

    def reply(self, data):
        pass

    def setAxisDef(self, motor, axisDef):
        self.axisDef[motor] = axisDef

    def clearAllAxisDef(self):
        self.axisDef = {}

    def clearAxisDef(self, motor):
        if motor in self.axisDef:
            del self.axisDef[motor]

    def getAxisDefString(self, motor):
        result = ''
        if motor in self.axisDef:
            for token in self.axisDef[motor]:
                result += token
        return result

    def motorAssigned(self, motor):
        return motor in self.axisDef

    def getInverseString(self):
        return self.inverse.getString()
 
    def getForwardString(self):
        return self.forward.getString()

    def assignProgram(self, prog):
        # Overrides any program already running
        self.currentProgram = prog
        self.programRunning = False

    def runProgram(self):
        self.programRunning = True
        program = self.pmac.getProgram(self.currentProgram)
        if program is not None:
            program.execute(self.id)

    def dwell(self, period):
        # Any running program is to pause
        # TODO
        pass

class Plc(PmacParser):

    def __init__(self, pmac, id):
        PmacParser.__init__(self, pmac)
        self.id = id
        self.buffer = Buffer()
        self.enable = False
        self.disable = False
        self.runRequestLock = None
        self.runCompleteLock = None

    def clear(self):
        self.enable = False
        self.buffer.clear()

    def add(self, token):
        self.enable = False
        self.buffer.add(token)

    def reply(self, data):
        pass

    def getString(self):
        return self.buffer.getString()

    def execute(self):
        if self.enable:
            self.runRequestLock.release()   # Allow execute thread to run
            self.runCompleteLock.acquire()  # We acquire the complete lock when execute done

    def run(self):
        # Run program forever
        while not self.disable:
            self.runRequestLock.acquire()   # Wait for next execution opportunity
            self.tokens = self.buffer.getTokens()
            self.processTokens(noPrint=True, programMode=True)
            self.runCompleteLock.release()  # Allow caller to continue execution
        self.disable = False
        self.enable = False

    def setEnable(self):
        self.disable = False
        if not self.enable:
            # Initialise locks
            self.runRequestLock = thread.allocate_lock()
            self.runCompleteLock = thread.allocate_lock()
            self.runRequestLock.acquire()
            self.runCompleteLock.acquire()
            # Now run the thread
            self.enable = True
            self.executeTaskId = thread.start_new_thread(self.run, ())

    def setDisable(self):
        if self.enable:
            self.disable = True

    def executionYield(self):
        # Execution of a program has reached an internal yield point or break point
        self.runCompleteLock.release()  # Allow caller to continue execution
        self.runRequestLock.acquire()   # Wait for our next execution opportunity

class Program(PmacParser):

    def __init__(self, pmac, id):
        PmacParser.__init__(self, pmac)
        self.id = id
        self.buffer = Buffer()

    def clear(self):
        self.buffer.clear()

    def add(self, token):
        self.buffer.add(token)

    def reply(self, data):
        pass

    def getString(self):
        return self.buffer.getString()

    def execute(self, cs):
        # Programs are executed in the context of a coordinate system
        if not self.buffer.empty():
            self.tokens = self.buffer.getTokens()
            self.currentCoordSystem = PmacToken(str(cs),'',0)
            self.debug = True
            self.processTokens(noPrint=True, programMode=True)

class MacroStation(object):
    def __init__(self):
        self.iVariables = {}

    def setIVariable(self, var, val):
        self.iVariables[var] = val

    def getIVariable(self, var):
        if not var in self.iVariables:
            self.iVariables[var] = 0.0
        return self.iVariables[var]

# This is the main simulation class that creates a simulation of a PMAC
# based motor controller.
class Pmac(object):
    motorUpdatePeriod = 0.1
    msPerMotorUpdatePeriod = 1000 * motorUpdatePeriod
    qVariableStartAddr = [0x0, 0x1000, 0x800, 0x1800, 0x400, 0xc00, 0x1400, 0x1c00, 
        0x200, 0x600, 0xa00, 0xe00, 0x1200, 0x1600, 0x1a00, 0x1e00]
    msConversionTable = [0, 0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,
        32,33,36,37,40,41,44,45,48,49,52,53,56,57,60,61]
    mnConversionTable = [0, 10, 20, 30, 40, 110, 120, 130, 140]
    countdownTimerDelta = 1000

    def __init__(self, name='none', geobrick=False, numMacroIcs=0, tcpPort=None,
            pmcFile=None, pmcIncludeDirs=[], beamline=None):
        self.beamline = beamline
        self.name = name
        # If we are running as part of a beamline simulation, register ourselves
        if self.beamline is not None:
            self.beamline.addSimulation(self)
        self.geobrick = geobrick
        self.numMacroIcs = numMacroIcs
        self.tcpPort = tcpPort
        self.pmcFile = pmcFile
        self.pmcIncludeDirs = pmcIncludeDirs
        if geobrick:
            self.cardId = '603382'
        else:
            self.cardId = '602413'
        self.numberOfAxes = self.numMacroIcs * 8
        if self.geobrick:
            self.numberOfAxes += 8
        self.iVariables = {}
        self.mVariables = {}
        self.pVariables = {}
        self.qVariables = {}
        self.macroStations = {}
        self.axes = {}
        self.coordinateSystems = {}
        self.plcs = {}
        self.programs = {}
        self.breakPoints = {}
        self.memory = {}
        self.motors = None
        self.setDefaultState()
        self.runMode = 'normal'
        self.pmcFileNames = {}
        for i in range(self.numberOfAxes):
            self.axes[i+1] = PmacMotor(i+1, self, self.geobrick)
        self.moveLoopTaskId = thread.start_new_thread(self.moveAllMotors, ())
        for i in range(16):
            self.coordinateSystems[i+1] = CoordinateSystem(self, i+1)
        for i in range(32):
            self.plcs[i] = Plc(self, i)
        for i in range(64):
            self.programs[i] = Program(self, i)
        self.consoleUser = ConsoleUser(self, self.name)
        if self.tcpPort is not None:
            self.tcpUserServer = TcpUserServer(self, self.tcpPort)
        if self.pmcFile is not None:
            self.readPmcFile(self.pmcFile)
        getUi(self).output("Initialised turbo PMAC simulator %s\n" % self.name)

    def consoleInput(self, text):
        self.consoleUser.process(text.rstrip() + '\n', True)

    def motor(self, motorNum):
        result = None
        if motorNum in self.axes:
            result = self.axes[motorNum]
        return result

    def registerPmacFileName(self, pathName):
        baseName = os.path.basename(pathName)
        self.pmcFileNames[baseName] = pathName

    def expandPmacFileName(self, fileName):
        result = fileName
        if fileName in self.pmcFileNames:
            result = self.pmcFileNames[fileName]
        return result

    def connectMotorSim(self, axis, sim):
        if axis in self.axes:
            self.axes[axis].connectMotorSim(sim)

    def getProgram(self, prog):
        result = None
        if prog in self.programs:
            result = self.programs[prog]
        return result

    def getMotor(self, motor):
        result = None
        if motor in self.axes:
            result = self.axes[motor]
        return result

    def getMotorPosition(self, motor):
        result = 0.0
        if motor in self.axes:
            result = self.axes[motor].currentPosition
        return result

    def transferMotorsToPVariables(self):
        # Transfers the current motor positions to P1-32 ready
        # for the forward kinematic calculations
        for id, motor in self.axes.iteritems():
            self.setPVariable(id, motor.getCurrentPosition())

    def motorInCoordSystem(self, motor):
        # Returns the coordinate system the motor is assigned to
        # or None if not assigned 
        result = None
        for n, cs in self.coordinateSystems.iteritems():
            if result is None and cs.motorAssigned(motor):
                result = n
        return result

    def getMemory(self, addr):
        # Gets the value indicated by the PMAC formatted address from
        # the memory dictionary.  Returns the tuple (supported, value).
        # Note that this function only supports bit fields for integer
        # types (x,y,l).
        val = 0.0
        typeString, address, offset, size, formatString = self.decodeMVarAddress(addr)
        memAddress = self.encodeMemoryAddress(typeString, address)
        supported = False
        if memAddress in self.memory:
            supported = True
            val = self.memory[memAddress]
            if typeString in ['x', 'y', 'l']:
                val = (int(val) >> offset) & PmacMotor.sizeToMask[size]
        return (supported, val)

    def setMemory(self, addr, val, feedback=True):
        # Sets the memory indicated by the PMAC formatted address.  Note
        # that this function only supports bit fields for integer types (x,y,l)
        supported = False
        typeString, address, offset, size, formatString = self.decodeMVarAddress(addr)
        # Inform the module that might be interested
        if feedback:
            if address >= 0x0080 and address < 0x1080:
                # Motor registers
                axisNum = (address >> 7) & 0x1f
                location = address & 0x7f
                if axisNum in self.axes:
                    supported = self.axes[axisNum].setMemory(typeString, location, 
                        offset, size, formatString, val)
                else:
                    supported = True  # Don't complain for non-existant axes
            elif address >= 0x8000 and address < 0xa000:
                # Q variables
                self.setQVariable(address - 0x8000, val)
                supported = True
            elif address >= 0x3440 and address < 0x3480:
                # Macro ring status registers
                supported = True
            elif address in [0x78400, 0x78420]:
                # I/O port
                supported = True
            elif address in PmacMotor.encoderLossMemoryLocations:
                # Encoder loss locations
                supported = True
        # Set the memory dictionary
        memAddress = self.encodeMemoryAddress(typeString, address)
        orig = 0
        if memAddress in self.memory:
            orig = self.memory[memAddress]
        if typeString in ['x', 'y']:
            bitmask = PmacMotor.sizeToMask[size] << offset
            orig = orig & ~bitmask
            orig = orig | ((int(val) << offset) & bitmask)
        else:
            orig = val
        self.memory[memAddress] = orig
        # Update any watch variable
        getUi(self).updateMVar(self, typeString, address)
        return supported

    def setMotorParamMemory(self, formatString, axis, offset, val, feedback=True):
        addr = "%s:$%x" % (formatString, (axis << 7) + offset)
        if formatString in ['x', 'y']:
            addr += ",24"
        return self.setMemory(addr, val, feedback)

    def encodeMemoryAddress(self, typeString, address):
        # Encode a memory address for use as an index in the memory dictionary
        # Returns the address string
        return "%s:$%x" % (typeString, address) 

    def decodeMemoryAddress(self, addr):
        # Decode a memory address used as an index in the memory dictionary.
        # Returns the tuple (typeString, address)
        parts = addr.split(':')
        return (parts[0],int(parts[1][1:], 16))

    def decodeMVarAddress(self, addr):
        # Returns a tuple consisting of the (typeString, address, offset, size, formatString)
        mainBits = addr.split(':')
        typeString = mainBits[0]
        address = 0
        offset = 0
        size = 1
        formatString = 'u'
        if len(mainBits) > 1:
            parts = mainBits[1].split(',')
            address = int(parts[0][1:], 16)
            if len(parts) > 1:
                offset = int(parts[1])
            if offset == 24:
                offset = 0
                size = 24
                if len(parts) > 2:
                    formatString = parts[2]
            else:
                if len(parts) > 2:
                    size = int(parts[2])
                if len(parts) > 3:
                    formatString = parts[3]
        return (typeString, address, offset, size, formatString)

    def setMotors(self, motors):
        self.motors = motors

    def setAxisIVariable(self, axis, var, val):
        self.setIVariable(axis*100 + var, val)

    def getAxisIVariable(self, axis, var):
        return self.getIVariable(axis*100 + var)

    def setIVariable(self, var, val):
        self.iVariables[var] = val
        getUi(self).updateVar(self, ('i%s' % var), val)

    def getIVariable(self, var):
        if not var in self.iVariables:
            #getUi(self).output("Read of undefined I%s\n" % var)
            self.iVariables[var] = 0.0
        return self.iVariables[var]

    def setCsQVariable(self, cs, var, val):
        if cs >= 1 and cs <= 16:
            self.setQVariable(Pmac.qVariableStartAddr[cs]+var, val)
            getUi(self).updateVar(self, ('%sq%s' % (cs, var)), val)
        else:
            getUi(self).output("Illegal coordinate system: %s\n" % cs)

    def setQVariable(self, var, val):
        self.qVariables[var] = val

    def getCsQVariable(self, cs, var):
        result = 0.0
        if cs >= 1 and cs <= 16:
            result = self.getQVariable(Pmac.qVariableStartAddr[cs]+var)
        else:
            getUi(self).output("Illegal coordinate system: %s\n" % cs)
        return result

    def getQVariable(self, var):
        if not var in self.qVariables:
            self.qVariables[var] = 0.0
        return self.qVariables[var]

    def setPVariable(self, var, val):
        self.pVariables[var] = val
        getUi(self).updateVar(self, ('p%s' % var), val)

    def getPVariable(self, var):
        if not var in self.pVariables:
            self.pVariables[var] = 0.0
        return self.pVariables[var]

    def setMotorPosition(self, var, val):
        getUi(self).updateVar(self, ('#%s' % var), val)

    def setMVariableAddress(self, var, val):
        if val == '':
            getUi(self).output("Setting m-var address to blank, m%s\n" % var)
        self.mVariables[var] = val

    def getMVariableAddress(self, var):
        if not var in self.mVariables:
            self.mVariables[var] = '*'
        return self.mVariables[var]

    def setMVariable(self, var, val):
        addr = self.getMVariableAddress(var)
        supported = self.setMemory(addr, val)
        #if not supported:
            # getUi(self).output("!>Write to unsupported memory location '%s=%s', M%s\n" % (addr, val, var))
            #print "!>Write to unsupported memory location '%s=%s', M%s\n" % (addr, val, var)

    def getMVariable(self, var):
        addr = self.getMVariableAddress(var)
        supported,val = self.getMemory(addr)
        #if not supported:
            #getUi(self).output("!>Read from unsupported memory location '%s', M%s\n" % (addr, var))
            #print "!>Read from unsupported memory location '%s', M%s\n" % (addr, var)
        return val

    def getMacroStation(self, ms):
        if not ms in self.macroStations:
            self.macroStations[ms] = MacroStation()
        return self.macroStations[ms]

    def setMsIVariable(self, ms, var, val):
        self.getMacroStation(ms).setIVariable(var, val)

    def getMsIVariable(self, ms, var):
        return self.getMacroStation(ms).getIVariable(var)

    def setAxisMsIVariable(self, axis, var, val):
        if self.geobrick:
            if var >= 910 and var <= 919:
                result = self.setIVariable(self.mnFromAxis(Axis) + 7000 + var - 910, val)
        else:
            self.setMsIVariable(self.msFromAxis(axis), var, val)

    def getAxisMsIVariable(self, axis, var):
        result = None
        if self.geobrick:
            if var >= 910 and var <= 919:
                result = self.getIVariable(self.mnFromAxis(axis) + 7000 + var - 910)
            else:
                print 'Cannot get MS I variable %s for geobrick' % var
        else:
            result = self.getMacroStation(self.msFromAxis(axis)).getIVariable(var)
            #print "[axis=%s, var=%s, val=%s]" % (axis, var, result)
        return result

    def mnFromAxis(self, axis):
        result = -1
        if axis >= 1 and axis <= 8:
            result = Pmac.mnConversionTable[axis]
        return result

    def msFromAxis(self, axis):
        result = -1
        if axis >= 1 and axis <= 32:
           result = Pmac.msConversionTable[axis]
        return result

    def msToAxis(self, ms):
        result = 1
        while Pmac.msConversionTable[result] != ms and result <= 32:
            result += 1
        return result

    def moveAllMotors(self):
        '''Move all the motors if they need moving.'''
        while True:
            time.sleep(Pmac.motorUpdatePeriod)
            if self.runMode  == 'normal' or self.runMode == 'single':
                for number, axis in self.axes.iteritems():
                    axis.move()
                for number, plc in self.plcs.iteritems():
                    plc.execute()
                for timerIvar in [5111, 5112, 5211, 5212, 5311, 5312, 5411, 5412,
                        5511, 5512, 5611, 5612, 5711, 5712, 5811, 5812, 5911, 5912]:
                    tick = int(self.getIVariable(timerIvar))
                    newtick = tick - Pmac.countdownTimerDelta
                    if newtick < -8388608:
                        newtick = -8388608
                    if newtick != tick:
                        self.setIVariable(timerIvar, newtick)

    def readPmcFile(self, fileName):
        '''Reads a PMC file.'''
        FileUser(fileName, self, self.pmcIncludeDirs)

    def setLowerLimitState(self, axis, state):
        if axis in self.axes:
            self.axes[axis].setLowerLimitState(state)

    def setUpperLimitState(self, axis, state):
        if axis in self.axes:
            self.axes[axis].setUpperLimitState(state)

    def setHomeSwitchState(self, axis, state):
        if axis in self.axes:
            self.axes[axis].setHomeSwitchState(state)

    def setEncoderPosition(self, axis, pos):
        if axis in self.axes:
            self.axes[axis].setEncoderPosition(pos)

    def setDefaultState(self):
        self.hexIVariables = [90, 95, 20, 21, 22, 23]
        self.iVariables[1] = 0
        self.iVariables[2] = 1
        self.iVariables[3] = 1
        self.iVariables[4] = 0
        self.iVariables[5] = 0
        self.iVariables[6] = 3
        self.iVariables[10] = 3713991 # pmacMotor.motorUpdatePeriod * 1000 * 8388607 
        self.iVariables[15] = 0    # Degree/radian control
        for macroic in range(4):
            if macroic < self.numMacroIcs:
                self.iVariables[20+macroic] = 0x78400 + macroic*0x1000
            else:
                self.iVariables[20+macroic] = 0
        self.iVariables[56] = 0    # DP RAM ASCII communications interrupt enable
        self.iVariables[58] = 0    # DP RAM ASCII communications enable
        self.iVariables[90] = 0x39   # VME address modifier
        self.iVariables[95] = 2    # VME interrupt level
        self.iVariables[5111] = 0
        self.iVariables[5112] = 0
        self.iVariables[5211] = 0
        self.iVariables[5212] = 0
        self.iVariables[5311] = 0
        self.iVariables[5312] = 0
        self.iVariables[5411] = 0
        self.iVariables[5412] = 0
        self.iVariables[5511] = 0
        self.iVariables[5512] = 0
        self.iVariables[5611] = 0
        self.iVariables[5612] = 0
        self.iVariables[5711] = 0
        self.iVariables[5712] = 0
        self.iVariables[5811] = 0
        self.iVariables[5812] = 0
        self.iVariables[5911] = 0
        self.iVariables[5912] = 0
        for axis in range(32):
            self.setMemory("x:$%x,13" % PmacMotor.encoderLossMemoryLocations[axis], 0)
        self.setMemory("x:$b,24", 0)  # Last real time interrupt execution time
        self.setMemory("x:$37,24", 0)  # Unknown, used by PmacStatus
        self.setMemory("y:$37,24", 0)  # Unknown, used by PmacStatus
        self.setMemory("x:$ffff8c,24", 0)  # Unknown, used by PmacStatus
        self.setMemory("x:$3430,24", 0)  # Total macro ring error count
        for plc in range(32):
            self.setMemory("y:$31%02x,22" % plc, 1)  # PLC disabled
        for cs in range(16):
            self.setMemory("x:$2%01x40,0" % cs, 0)  # CS program running

    def sendMoveToMotors(self, axis, offset):
        '''Sends an axis' move offset.'''
        if self.motors is not None:
            self.motors.sendMoveToMotors(axis, offset)

    def isAtBreakPoint(self, fileName, lineNumber):
        # Returns true if the specified line has a break point set
        bp = BreakPoint(fileName, lineNumber)
        if  bp.getName() in self.breakPoints:
            self.runMode = 'stop'
            getUi(self).output("Stopped on breakpoint %s:%s\n" % (fileName, lineNumber))
        return self.runMode == 'stop'

    def isSingleStep(self, onNewLine):
        result = False
        if self.runMode == 'single' and onNewLine:
            self.runMode = 'stop'
            result = True
        return result

    def cmdDebugBreak(self, user, fileName, lineNumber, on):
        fileName = self.expandPmacFileName(fileName)
        bp = BreakPoint(fileName, lineNumber)
        if on:
            if bp.getName() not in self.breakPoints:
                self.breakPoints[bp.getName()] = bp
            getUi(self).displayLine(fileName, lineNumber)
        else:
            if bp.getName() in self.breakPoints:
                del self.breakPoints[bp.getName()]

    def cmdDebugBreakList(self, user):
        for name, bp in self.breakPoints.iteritems():
            getUi(self).output('Break point: %s\n' % name)

    def cmdDebugBreakClear(self, user):
        self.breakPoints = {}

    def cmdDebugContinue(self, user):
        self.stoppedOnBreakPoint = False
        self.runMode = 'normal'

    def cmdDebugStep(self, user):
        self.runMode = 'single'

    def cmdDebugWatchMotor(self, user, var):
        getUi(self).watchVar(self, ('#%s' % var), self.getMotorPosition(var))

    def cmdDebugWatchIVar(self, user, var):
        getUi(self).watchVar(self, ('i%s' % var), self.getIVariable(var))

    def cmdDebugWatchPVar(self, user, var):
        getUi(self).watchVar(self, ('p%s' % var), self.getPVariable(var))

    def cmdDebugWatchQVar(self, user, cs, var):
        getUi(self).watchVar(self, ('%sq%s' % (cs, var)), self.getQVariable(var))

    def cmdDebugWatchMVar(self, user, var):
        getUi(self).watchVar(self, ('m%s' % var), self.getMVariable(var))

    def cmdDebugUnwatchMotor(self, user, var):
        getUi(self).unwatchVar(self, ('#%s' % var))

    def cmdDebugUnwatchIVar(self, user, var):
        getUi(self).unwatchVar(self, 'i%s' % var)

    def cmdDebugUnwatchPVar(self, user, var):
        getUi(self).unwatchVar(self, 'p%s' % var)

    def cmdDebugUnwatchQVar(self, user, cs, var):
        getUi(self).unwatchVar(self, '%sq%s' % (cs, var))

    def cmdDebugUnwatchMVar(self, user, var):
        getUi(self).unwatchVar('self, m%s' % var)

    def cmdDebugTracePlc(self, user, plc):
        getUi(self).debug('Trace plc%d\n' % plc)
        if plc in self.plcs:
            getUi(self).debug('Trace plc%d found\n' % plc)
            self.plcs[plc].openTraceWindow()

    def cmdDebugTraceProgram(self, user, pgm):
        if pgm in self.programs:
            self.programs[pgm].openTraceWindow()

    def cmdDebugTraceCs(self, user, cs):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].openTraceWindow()

    def cmdDebugTraceOff(self):
        for n,plc in self.plcs.iteritems():
            plc.closeTraceWindow()
        for n,pgm in self.programs.iteritems():
            pgm.closeTraceWindow()
        for n,cs in self.coordinateSystems.iteritems():
            cs.closeTraceWindow()

    def cmdDebugTraceLog(self, user, fileName):
        getUi(self).setTraceLog(fileName)

    def cmdJogRelative(self, user, axis, offset):
        if axis in self.axes:
            self.axes[axis].jogRelative(offset)

    def cmdJogRelativeVariable(self, user, axis):
        if axis in self.axes:
            self.axes[axis].jogRelativeVariable()

    def cmdJogStopTrigger(self, user, axis, after):
        if axis in self.axes:
            self.axes[axis].jogStopTrigger(after)
        
    def cmdHome(self, user, axis):
        if axis in self.axes:
            self.axes[axis].home()

    def cmdCid(self, user):
        user.output("%s " % self.cardId)

    def cmdMoveAxis(self, cs, moves):
        # Handles an axis move command in a coordinate system
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].moveAxis(moves)

    def cmdAbortCoordSystem(self, cs):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].abort()

    def cmdKillMotor(self, axis):
        if axis in self.axes:
            self.axes[axis].kill()

    def cmdEncoderLoss(self, axis, eloss):
        if axis in self.axes:
            self.axes[axis].setEncoderLoss(eloss)

    def cmdDwell(self, user, cs, period):
        getUi(self).output("Dwell for cs=%s\n" % cs)
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].dwell(period)

    def cmdRunProgram(self, user, cs):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].runProgram()

    def cmdAssignProgram(self, user, cs, prog):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].assignProgram(prog)

    def cmdReportCoordSystem(self, user, cs):
        if cs in self.coordinateSystems:
            user.output(self.coordinateSystems[cs].getStatus())

    def cmdEnablePlc(self, user, plc):
        if plc in self.plcs:
            self.plcs[plc].setEnable()

    def cmdDisablePlc(self, user, plc):
        if plc in self.plcs:
            self.plcs[plc].setDisable()

    def cmdListInverse(self, user, cs):
        if cs in self.coordinateSystems:
            user.output(self.coordinateSystems[cs].getInverseString())

    def cmdListForward(self, user, cs):
        if cs in self.coordinateSystems:
            user.output(self.coordinateSystems[cs].getForwardString())

    def cmdListPlc(self, user, plc):
        if plc in self.plcs:
            user.output(self.plcs[plc].getString())

    def cmdListProgram(self, user, prog):
        if prog in self.programs:
            user.output(self.programs[prog].getString())

    def cmdSetCurrentCoordSystem(self, user, number):
        pass

    def cmdReportCurrentCoordSystem(self, user):
        pass

    def cmdClose(self, user):
        pass

    def cmdClearPlc(self, user, plc):
        if plc in self.plcs:
            self.plcs[plc].clear()

    def cmdAddPlc(self, user, plc, token):
        if plc in self.plcs:
            self.plcs[plc].add(token)

    def cmdClearProgram(self, user, prog):
        if prog in self.programs:
            self.programs[prog].clear()

    def cmdAddProg(self, user, prog, token):
        if prog in self.programs:
            self.programs[prog].add(token)

    def cmdClearForward(self, user, cs):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].clearForward()

    def cmdAddForward(self, user, cs, token):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].addForward(token)

    def cmdClearInverse(self, user, cs):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].clearInverse()

    def cmdAddInverse(self, user, cs, token):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].addInverse(token)

    def cmdUndefine(self, user):
        pass

    def cmdUndefineAll(self, user):
        pass

    def cmdVer(self, user):
        user.output('1.934')

    def cmdSetMVariableAddress(self, user, first, number, step, addrType, address, 
            offset, width, format):
        value = ''
        if addrType == '*':
            value = '*'
        elif addrType == 'x' or addrType == 'y':
            if offset == 0 and width == 24:
                if format == 'u':
                    value = '%s:$%x,24' % (addrType, address)
                else:
                    value = '%s:$%x,24,%s' % (addrType, address, format)
            else:
                if format == 'u':
                    if width == 1:
                        value = '%s:$%x,%s' % (addrType, address, offset)
                    else:
                        value = '%s:$%x,%s,%s' % (addrType, address, offset, width)
                else:
                    value = '%s:$%x,%s,%s,%s' % (addrType, address, offset, width, format)
        elif addrType == 'l' or addrType == 'd':
            value = '%s:$%x' % (addrType, address)
        else:
            getUi(self).output('Unsupported M-variable address type "%s\n"' % addrType)
        for i in range(first, first+(number*step), step):
            self.setMVariableAddress(i, value)

    def cmdResetMVariable(self, user, first, number, step):
        for i in range(first, first+(number*step), step):
            self.setMVariableAddress(i, '*')

    def cmdSetMVariable(self, user, first, number, step, assign):
        for i in range(first, first+(number*step), step):
            self.setMVariable(i, assign)

    def cmdReportMVariable(self, user, first, number, step):
        result = ''
        for i in range(first, first+(number*step), step):
            val = self.getMVariable(i)
            if (val - int(val)) == 0:
                val = int(val)
            result += "%s " % val
        user.output(result)

    def cmdReportMVariableAddress(self, user, first, number, step):
        result = ''
        for i in range(first, first+(number*step), step):
            result += '%s ' % self.getMVariableAddress(i)
        user.output(result)

    def cmdSetIVariable(self, user, first, number, step, assign):
        for i in range(first, first+(number*step), step):
            self.setIVariable(i, assign)

    def cmdReportIVariable(self, user, first, number, step):
        result = ''
        for i in range(first, first+(number*step), step):
            val = self.getIVariable(i)
            if (val - int(val)) == 0:
                val = int(val)
            if i in self.hexIVariables:
                result += "$%x " % val
            else:
                result += "%s " % val
        user.output(result)

    def cmdSetQVariable(self, user, cs, first, number, step, assign):
        for i in range(first, first+(number*step), step):
            #if self.getCsQVariable(cs, i) != assign:
            #    print "{{{cs%sq%s = %s}}}" % (cs, i, assign)
            self.setCsQVariable(cs, i, assign)

    def cmdReportQVariable(self, user, cs, first, number, step):
        result = ''
        for i in range(first, first+(number*step), step):
            val = self.getCsQVariable(cs, i)
            if (val - int(val)) == 0:
                val = int(val)
            result += "%s " % val
        user.output(result)

    def cmdSetPVariable(self, user, first, number, step, assign):
        for i in range(first, first+(number*step), step):
            self.setPVariable(i, assign)

    def cmdReportPVariable(self, user, first, number, step):
        result = ''
        for i in range(first, first+(number*step), step):
            val = self.getPVariable(i)
            if (val - int(val)) == 0:
                val = int(val)
            result += "%s " % val
        user.output(result)

    def cmdReportMotorPosition(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            user.output('%s ' % self.axes[axis].motorPosition())
        else:
            user.output('ERR001 ')

    def cmdReportMotorVelocity(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            user.output('%s ' % self.axes[axis].motorVelocity())
        else:
            user.output('ERR001 ')

    def cmdReportMotorStatus(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            user.output('%s ' % self.axes[axis].motorStatus())
        else:
            user.output('ERR001 ')

    def cmdReportGlobalStatus(self, user):
        user.output("000001C00000 ")

    def cmdReportMotorFollowingError(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            user.output('%s ' % self.axes[axis].motorFollowingError())
        else:
            user.output('ERR001 ')

    def cmdSetMsIVariable(self, user, ms, addr, value):
        self.setMsIVariable(ms, addr, value)

    def cmdReportMsIVariable(self, user, ms, addr):
        user.output('%s ' % int(self.getMsIVariable(ms, addr)))

    def cmdJogPositive(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            self.axes[axis].doJogPositive()

    def cmdJogNegative(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            self.axes[axis].doJogNegative()

    def cmdJogStop(self, user):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            self.axes[axis].jogStop()

    def cmdJogTo(self, user, position):
        axis = user.getCurrentAxis()
        if axis in self.axes:
            self.axes[axis].jog(position)

    def cmdQuitProgramAtEndOfMove(self, user):
        pass

    def cmdAssignAxisDef(self, user, cs, motor, axisDef):
        existing = self.motorInCoordSystem(motor)
        if existing is not None and existing != cs:
            user.output('ERR003 ')
        elif cs in self.coordinateSystems:
            self.coordinateSystems[cs].setAxisDef(motor, axisDef)

    def cmdClearAxisDef(self, user, cs, motor):
        if cs in self.coordinateSystems:
            self.coordinateSystems[cs].clearAxisDef(motor)

    def cmdReportCoordSystemAxisDef(self, user, cs, motor):
        if cs in self.coordinateSystems:
            user.output('%s ' % self.coordinateSystems[cs].getAxisDefString(motor))


