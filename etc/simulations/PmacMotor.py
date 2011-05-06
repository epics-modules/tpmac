from PmacUi import *
import os, sys, subprocess, thread, socket, select, time, math

class PmacMotor(object):
    motorUpdatePeriod = 0.1
    msPerMotorUpdatePeriod = 1000 * motorUpdatePeriod
    encoderLossMemoryLocations = [0x78421, 0x78422, 0x78423, 0x78425, 0x78426, 0x78427,
        0x78429, 0x7842a, 0x7842b, 0x7842d, 0x7842e, 0x7842f, 0x78431, 0x78432, 0x78433, 0x78435,
        0x7a421, 0x7a422, 0x7a423, 0x7a425, 0x7a426, 0x7a427, 0x7a429, 0x7a42a, 0x7a42b,
        0x7a42d, 0x7a42e, 0x7a42f, 0x7a431, 0x7a432, 0x7a433, 0x7a435]
    macroRingStatusMemoryLocations = [0, 0x3440, 0x3441, 0x3444, 0x3445, 0x3448, 0x3449, 0x344c, 0x344d,
        0x3450, 0x3451, 0x3454, 0x3455, 0x3458, 0x3459, 0x345c, 0x345d,
        0x3460, 0x3461, 0x3464, 0x3465, 0x3468, 0x3469, 0x346c, 0x346d,
        0x3460, 0x3461, 0x3464, 0x3465, 0x3468, 0x3469, 0x346c, 0x346d]
    printMove = False
    sizeToMask = [0, 0x000001, 0x000003, 0x000007, 0x00000f, 0x00001f, 0x00003f,
        0x00007f, 0x0000ff, 0x0001ff, 0x0003ff, 0x0007ff, 0x000fff, 0x001fff,
        0x003fff, 0x003fff, 0x007fff, 0x00ffff, 0x01ffff, 0x03ffff, 0x07ffff,
        0x0fffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff]

    def __init__(self, number, pmac, geobrick):
        self.number = number;
        self.pmac = pmac
        self.geobrick = geobrick
        self.defaultIvars()
        self.setCurrentPosition(0.0)
        self.setCommandPosition(0.0)
        self.currentVelocity = 0
        self.followingError = 0
        self.jogPositive = False
        self.jogNegative = False
        self.homing = False
        self.homeComplete = False
        self.atLowerLimit = False
        self.atUpperLimit = False
        self.stoppedOnLimit = False
        self.amplifierEnabled = False
        self.servoLoopEnabled = False
        self.encoderMode = "None"
        self.ignoreEncoder = False
        self.followingErrorOn = False
        self.lock = thread.allocate_lock()
        self.setEncoderLoss(False)
        self.jogPositionDistanceRegister = 0.0
        self.jogStopTriggerActive = False
        self.jogStopTriggerAfter = 0.0
        self.status1 = 0
        self.status2 = 0
        self.positionCaptureInfo = 0
        self.lastTriggered = False
        self.motorSimConnected = False
        self.macroRingStatus = 0
        self.name = 'none'
        self.scaleFactor = 1.0
        self.lowerLimit = 0.0
        self.upperLimit = 0.0
        self.precision = 0.01
        addr = 'x:$%x,24' % PmacMotor.macroRingStatusMemoryLocations[self.number]
        self.pmac.setMemory(addr, self.macroRingStatus, False)

    def configureHardware(self, name='none', scaleFactor=1.0, lowerLimit=0.0,
            upperLimit=0.0, precision=0.01):
        self.name = name
        self.scaleFactor = scaleFactor
        self.lowerLimit = lowerLimit
        self.upperLimit = upperLimit
        self.precision = precision

    def connectMotorSim(self, sim):
        self.motorSimConnected = True
        sim.sendAxisConfiguration(self.number, self.scaleFactor, self.lowerLimit,
            self.upperLimit, self.precision)

    def setEncoderLoss(self, eloss):
        self.encoderLost = eloss
        addr = "x:$%x,13" %  PmacMotor.encoderLossMemoryLocations[self.number-1]
        if self.encoderLost:
            self.pmac.setMemory(addr, 0, False)
        else:
            self.pmac.setMemory(addr, 1, False)

    def setMemory(self, typeString, location, offset, size, formatString, val):
        result = True
        if location == 0x08 and typeString == 'd':
            self.setCommandPosition(val / (self.pmac.getAxisIVariable(self.number, 8) * 32))
        elif location == 0x0b and typeString == 'd':
            self.setCurrentPosition(val / (self.pmac.getAxisIVariable(self.number, 8) * 32))
        elif location == 0x57 and typeString == 'l':
            # The jog distance/offset variable
            self.jogPositionDistanceRegister = val
        else:
            result = None
        return result

    def setCommandPosition(self, val):
        # Sets the command position variable and memory.  No feedback
        # to avoid recursive loop
        self.commandPosition = val
        self.pmac.setMotorParamMemory('d', self.number, 0x08, 
            val * (self.pmac.getAxisIVariable(self.number, 8) * 32), False)

    def setCurrentPosition(self, val):
        # Sets the current position variable and memory.  No feedback
        # to avoid recursive loop
        self.currentPosition = val
        self.pmac.setMotorParamMemory('d', self.number, 0x0b, 
            val * (self.pmac.getAxisIVariable(self.number, 8) * 32), False)
        self.pmac.setMotorPosition(self.number, self.currentPosition)

    def checkForTriggers(self):
        if self.homing and (self.positionCaptureInfo & 0x800) != 0:
            # Home move is complete
            getUi().output("Home complete axis %s\n" % self.number)
            #self.lock.acquire()
            self.jogPositive = False
            self.jogNegative = False
            #self.currentVelocity = 0
            self.homeComplete = True
            self.homing = False
            self.stoppedOnLimit = False
            #self.atLowerLimit = False
            #self.atUpperLimit = False
            self.setCurrentPosition(-int(self.pmac.getAxisIVariable(self.number, 26) / 16))
            self.setCommandPosition(0.0)
            self.calcMotorStatus()
            #self.lock.release()
        elif self.jogStopTriggerActive and (self.positionCaptureInfo & 0x800) != 0:
            # Jog stop trigger has fired
            getUi().output("Jog triggered stop axis %s\n" % self.number)
            #self.lock.acquire()
            self.jogPositive = False
            self.jogNegative = False
            self.stoppedOnLimit = False
            self.currentVelocity = 0
            self.jogStopTriggerActive = False
            self.setCommandPosition(self.currentPosition + self.jogStopTriggerAfter)
            self.calcMotorStatus()
            #self.lock.release()

    def defaultIvars(self):
        self.pmac.setAxisIVariable(self.number, 13, 0)
        self.pmac.setAxisIVariable(self.number, 14, 0)
        self.pmac.setAxisIVariable(self.number, 30, 2000)
        self.pmac.setAxisIVariable(self.number, 31, 1280)
        self.pmac.setAxisIVariable(self.number, 33, 0)
        self.pmac.setAxisIVariable(self.number, 22, 32)
        self.pmac.setAxisIVariable(self.number, 20, 50)
        self.pmac.setAxisIVariable(self.number, 8, 96)
        self.pmac.setAxisIVariable(self.number, 24, 0)
        self.pmac.setAxisIVariable(self.number, 25, 
            self.macroRingStatusMemoryLocations[self.number])

    def initialise(self):
        self.lock.acquire()
        self.setCurrentPosition(0.0)
        self.setCommandPosition(0.0)
        self.lock.release()
    
    def setEncoderMode(self, mode):
        self.lock.acquire()
        self.encoderMode = mode
        self.lock.release()

    def setFollowingErrorOn(self, on):
        self.lock.acquire()
        self.followingErrorOn = on
        getUi().output("Following error on for %s is %s\n" % \
            (self.name, self.followingErrorOn))
        self.lock.release()

    def calcMotorStatus(self):
        '''Calculate the two motor status words.'''
        origStatus1 = self.status1
        origStatus2 = self.status2
        limitsActive = (int(self.pmac.getAxisIVariable(self.number, 24)) & 0x20000) == 0
        self.status1 = 0
        self.status1 |= 0x800000  # Motor activated
        if limitsActive and self.atLowerLimit:
            self.status1 |= 0x400000  # Negative limit activated
        if limitsActive and self.atUpperLimit:
            self.status1 |= 0x200000  # Positive limit activated
        if self.amplifierEnabled:
            self.status1 |= 0x080000  # Amplifier enabled
        if not self.servoLoopEnabled:
            self.status1 |= 0x100000  # Open loop mode
        if self.currentPosition != self.commandPosition:
            self.status1 |= 0x020000  # Running definite time move
        if self.currentVelocity == 0.0:
            self.status1 |= 0x002000  # Desired velocity zero
        self.status2 = 0
        if self.homeComplete:
            self.status2 |= 0x000400  # Home complete
        if limitsActive and self.stoppedOnLimit:
            self.status2 |= 0x000800  # Stopped on limit
        if self.currentPosition == self.commandPosition:
            self.status2 |= 0x000001  # In position
        cs = self.pmac.motorInCoordSystem(self.number)
        if cs is not None:
            self.status2 |= (cs - 1) << 20  # The CS number (-1)
            self.status2 |= 0x008000        # Assigned to a CS
        if self.status1 != origStatus1:
            self.pmac.setMotorParamMemory('x', self.number, 0x30, self.status1, False)
        if self.status2 != origStatus2:
            self.pmac.setMotorParamMemory('y', self.number, 0x40, self.status2, False)

    def motorStatus(self):
        '''Return the two motor status words.'''
        self.lock.acquire()
        text = "%06X%06X" % (self.status1, self.status2)
        self.lock.release()
        return text

    def isInPosition(self):
        return self.currentPosition == self.commandPosition

    def motorFollowingError(self):
        self.lock.acquire()
        result = "%s " % self.followingError
        self.lock.release()
        return result

    def motorPosition(self):
        self.lock.acquire()
        result = "%s " % self.currentPosition
        self.lock.release()
        return result

    def motorVelocity(self):
        self.lock.acquire()
        result = "%s " % self.currentVelocity
        self.lock.release()
        return result

    def jogStop(self):
        self.lock.acquire()
        self.jogPositive = False
        self.jogNegative = False
        self.servoLoopEnabled = True
        self.amplifierEnabled = True
        self.homing = False
        self.setCommandPosition(self.currentPosition)
        self.currentVelocity = 0
        self.calcMotorStatus()
        getUi().output("Stop jog motor %s\n" % self.number)
        self.lock.release()
        return ""

    def kill(self):
        self.lock.acquire()
        self.jogPositive = False
        self.jogNegative = False
        self.homing = False
        self.amplifierEnabled = False
        self.servoLoopEnabled = False
        self.setCommandPosition(self.currentPosition)
        self.currentVelocity = 0
        self.calcMotorStatus()
        getUi().output("Kill motor %s\n" % self.number)
        self.lock.release()

    def home(self):
        self.lock.acquire()
        self.servoLoopEnabled = True
        self.amplifierEnabled = True
        getUi().output("Homing axis %s\n" % self.number)
        self.homeComplete = False
        self.homing = True
        homeVelocity = self.pmac.getAxisIVariable(self.number, 23)
        if homeVelocity < 0:
            self.jogNegative = True
            self.jogPositive = False
        else:
            self.jogNegative = False
            self.jogPositive = True
        self.stoppedOnLimit = False
        self.lock.release()

    def jogStopTrigger(self, after):
        self.jogStopTriggerActive = True
        self.jogStopTriggerAfter = after
        self.lastTriggered = False
        getUi().output("Jog trigger set %s\n" % self.number)

    def jog(self, pos):
        self.lock.acquire()
        self.servoLoopEnabled = True
        self.amplifierEnabled = True
        self.jogPositive = False
        self.jogNegative = False
        self.setCommandPosition(pos)
        self.stoppedOnLimit = False
        getUi().output("Jog motor %s to %s\n" % (self.number, self.commandPosition))
        self.lock.release()
        return ""

    def jogRelative(self, offset):
        # Jog relative to current actual position
        self.lock.acquire()
        self.servoLoopEnabled = True
        self.amplifierEnabled = True
        self.jogPositive = False
        self.jogNegative = False
        self.setCommandPosition(self.currentPosition + offset)
        self.stoppedOnLimit = False
        getUi().output("Jog motor %s to %s\n" % (self.number, self.commandPosition))
        self.lock.release()

    def jogRelativeVariable(self):
        # Jog relative to the current position using the memory
        # variable as the offset
        self.jogRelative(self.jogPositionDistanceRegister)

    def doJogPositive(self):
        self.lock.acquire()
        self.servoLoopEnabled = True
        self.amplifierEnabled = True
        self.jogPositive = True
        self.jogNegative = False
        self.stoppedOnLimit = False
        getUi().output("Jog motor %s positive\n" % self.number)
        self.lock.release()
        return ""

    def doJogNegative(self):
        self.lock.acquire()
        self.servoLoopEnabled = True
        self.amplifierEnabled = True
        self.jogNegative = True
        self.jogPositive = False
        self.stoppedOnLimit = False
        getUi().output("Jog motor %s negative\n" % self.number)
        self.lock.release()
        return ""

    def move(self):
        self.lock.acquire()
        # Does this motor need to move?
        if self.servoLoopEnabled and \
                (self.currentPosition != self.commandPosition or \
                self.jogPositive or self.jogNegative):
            startingPosition = self.currentPosition
            # Get information from i vars
            timeForAccel = self.pmac.getAxisIVariable(self.number, 20)
            if self.homing:
                maxVelocity = abs(self.pmac.getAxisIVariable(self.number, 23))
            else:
                maxVelocity = self.pmac.getAxisIVariable(self.number, 22)
            # Work out the current acceleration (counts/ms/ms)
            accel = maxVelocity / timeForAccel
            # How long to go at current velocity?
            toGo = abs(self.commandPosition - self.currentPosition)
            timeToGo = timeForAccel * 2
            if not self.jogPositive and not self.jogNegative:
                if self.currentVelocity > 0:
                    timeToGo = toGo / self.currentVelocity
            # Accelerate
            if timeToGo > timeForAccel:
                self.currentVelocity += (accel * PmacMotor.msPerMotorUpdatePeriod)
                if self.currentVelocity >= maxVelocity:
                    self.currentVelocity = maxVelocity
            # Make a move
            if self.jogPositive:
                self.setCurrentPosition(self.currentPosition + \
                    (self.currentVelocity * PmacMotor.msPerMotorUpdatePeriod))
            elif self.jogNegative:                
                self.setCurrentPosition(self.currentPosition - \
                    (self.currentVelocity * PmacMotor.msPerMotorUpdatePeriod))
            elif self.currentVelocity <= 0:
                self.setCurrentPosition(self.commandPosition)
            elif self.currentPosition < self.commandPosition:
                self.setCurrentPosition(self.currentPosition + (self.currentVelocity * PmacMotor.msPerMotorUpdatePeriod))
                if self.currentPosition > self.commandPosition:
                    self.setCurrentPosition(self.commandPosition)
            else:
                self.setCurrentPosition(self.currentPosition - (self.currentVelocity * PmacMotor.msPerMotorUpdatePeriod))
                if self.currentPosition < self.commandPosition:
                    self.setCurrentPosition(self.commandPosition)
            # Inform any connected motors
            self.pmac.sendMoveToMotors(self.number, self.currentPosition-startingPosition)
            # Decelerate
            if timeToGo <= timeForAccel:
                self.currentVelocity -= (accel * PmacMotor.msPerMotorUpdatePeriod)
                if self.currentVelocity < 0.0:
                    self.currentVelocity = 0.0
            # Diagnostics
            if PmacMotor.printMove:
                getUi().output("Axis %s, moved to %s, target %s, velocity %s\n" % \
                    (self.number, self.currentPosition, self.commandPosition, 
                    self.currentVelocity))
            getUi().debug("Axis %s, moved to %s, target %s, velocity %s\n" % \
                    (self.number, self.currentPosition, self.commandPosition, 
                    self.currentVelocity))
        else:
            # Not moving, force current velocity to zero
            self.currentVelocity = 0.0
        # Housekeeping
        self.calcMotorStatus()
        self.checkForTrigger()
        self.lock.release()
        self.doMotorSimulation()

    def doMotorSimulation(self):
        # Handles the simulation of the motor if there is no external
        # motor simulation connected
        if not self.motorSimConnected:
            if self.lowerLimit != 0 or self.upperLimit != 0:
                self.setLowerLimitState(self.currentPosition <= self.lowerLimit*self.scaleFactor)
                self.setUpperLimitState(self.currentPosition >= self.upperLimit*self.scaleFactor)
            self.setEncoderPosition(self.currentPosition)

    def setEncoderPosition(self, pos):
        self.lock.acquire()
        if not self.followingErrorOn:
            if self.encoderMode == "Incremental":
                if not self.ignoreEncoder:
                    self.encoderPosition += pos
            elif self.encoderMode == "Absolute":
                if not self.ignoreEncoder:
                    self.encoderPosition = pos
        self.lock.release()

    def setLowerLimitState(self, state):
        if self.atLowerLimit != state:
            self.lock.acquire()
            getUi().output("Lower limit, on=%s, motor=%s, curPos=%s, cmdPos=%s\n" % \
                (state, self.number, self.currentPosition, self.commandPosition))
            self.atLowerLimit = state
            self.lock.release()
            limitsActive = (int(self.pmac.getAxisIVariable(self.number, 24)) & 0x20000) == 0
            if limitsActive and self.atLowerLimit:
                self.lock.acquire()
                self.setCommandPosition(self.currentPosition)
                self.jogPositive = False
                self.jogNegative = False
                self.stoppedOnLimit = True
                self.currentVelocity = 0
                self.calcMotorStatus()
                self.lock.release()
            addr = 'x:$%x,24' % PmacMotor.macroRingStatusMemoryLocations[self.number]
            if self.atLowerLimit:
                self.macroRingStatus = self.macroRingStatus | 0x40000
                self.pmac.setMemory(addr, self.macroRingStatus)
            else:
                self.macroRingStatus = self.macroRingStatus & ~0x40000
                self.pmac.setMemory(addr, self.macroRingStatus)

    def checkForTrigger(self):
        triggered = False
        if self.atLowerLimit:
            if self.pmac.getAxisMsIVariable(self.number, 913) == 2:
                if self.pmac.getAxisMsIVariable(self.number, 912) in [2,6]:
                    triggered = True
        else:
            if self.pmac.getAxisMsIVariable(self.number, 913) == 2:
                if self.pmac.getAxisMsIVariable(self.number, 912) in [10,14]:
                    triggered = True
        if self.atUpperLimit:
            if self.pmac.getAxisMsIVariable(self.number, 913) == 1:
                if self.pmac.getAxisMsIVariable(self.number, 912) in [2,6]:
                    triggered = True
        else:
            if self.pmac.getAxisMsIVariable(self.number, 913) == 1:
                if self.pmac.getAxisMsIVariable(self.number, 912) in [10,14]:
                    triggered = True
        addr = 'x:$%x,24' % PmacMotor.macroRingStatusMemoryLocations[self.number]
        if triggered and not self.lastTriggered:
            getUi().output("Triggered\n")
            self.macroRingStatus = self.macroRingStatus | 0x800
            self.pmac.setMemory(addr, self.macroRingStatus)
            if self.homing:
                self.jogPositive = False
                self.jogNegative = False
                self.homeComplete = True
                self.homing = False
                self.stoppedOnLimit = False
                self.setCurrentPosition(-int(self.pmac.getAxisIVariable(self.number, 26) / 16))
                self.setCommandPosition(0.0)
            elif self.jogStopTriggerActive:
                self.jogPositive = False
                self.jogNegative = False
                self.stoppedOnLimit = False
                self.currentVelocity = 0
                self.jogStopTriggerActive = False
                self.setCommandPosition(self.currentPosition + self.jogStopTriggerAfter)
        elif not triggered and self.lastTriggered:
            getUi().output("Not triggered\n")
            self.macroRingStatus = self.macroRingStatus & ~0x800
            self.pmac.setMemory(addr, self.macroRingStatus)
        self.lastTriggered = triggered

    def setUpperLimitState(self, state):
        if self.atUpperLimit != state:
            self.lock.acquire()
            getUi().output("Upper limit, on=%s, motor=%s, curPos=%s, cmdPos=%s\n" % \
                (state, self.number, self.currentPosition, self.commandPosition))
            self.atUpperLimit = state
            self.lock.release()
            limitsActive = (int(self.pmac.getAxisIVariable(self.number, 24)) & 0x20000) == 0
            if limitsActive and self.atUpperLimit:
                self.lock.acquire()
                self.setCommandPosition(self.currentPosition)
                self.jogPositive = False
                self.jogNegative = False
                self.stoppedOnLimit = True
                self.currentVelocity = 0
                self.calcMotorStatus()
                self.lock.release()
            addr = 'x:$%x,24' % PmacMotor.macroRingStatusMemoryLocations[self.number]
            if self.atUpperLimit:
                self.macroRingStatus = self.macroRingStatus | 0x20000
                self.pmac.setMemory(addr, self.macroRingStatus)
            else:
                self.macroRingStatus = self.macroRingStatus & ~0x20000
                self.pmac.setMemory(addr, self.macroRingStatus)

    def setHomeSwitchState(self, state):
        pass       # TODO


    def getCurrentPosition(self):
        return self.currentPosition

