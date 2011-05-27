from pkg_resources import require
require('dls_simulationlib')
require('dls_pmaclib')
import dls_simulationlib.simsocket
from dls_pmaclib.dls_pmcpreprocessor import clsPmacParser
from PmacParser import *
from PmacUi import *
import os, sys, math

class FileUser(PmacParser):
    def __init__(self, fileName, pmac, includePaths):
        getUi().output("Reading PMC file %s\n" % fileName)
        PmacParser.__init__(self, pmac)
        p = clsPmacParser(includePaths = includePaths)
        converted = p.parse(fileName, debug=True)
        if converted is not None:
            for line in p.output:
                self.process(line, True)

    def reply(self, data):
        pass

class ConsoleUser(PmacParser):
    def __init__(self, pmac, prompt):
        PmacParser.__init__(self, pmac)
        self.prompt = prompt

    def reply(self, data):
        text = data.rstrip()
        if len(text) > 0:
            getUi().output("%s\n" % text)

    def doConsole(self, machine):
        going = True
        getUi().setActiveMachine(machine)
        while going:
            text = getUi().inputLine('%s> ' % self.prompt)
            getUi().output('%s> %s\n' % (self.prompt, text.rstrip()))
            if text.strip().lower() == 'quit':
                going = False
            else:
                self.process(text + '\n', True)
        getUi().setActiveMachine(None)

class TcpUser(dls_simulationlib.simsocket.dataSocket, PmacParser):
    def __init__(self, clientSocket, pmac):
        dls_simulationlib.simsocket.dataSocket.__init__(self, clientSocket)
        PmacParser.__init__(self, pmac)
        self.tcpInputData = ''
        self.tcpOutputData = ''

    def receive(self, data):
        '''Override to receive data from the socket'''
        self.tcpInputData += data
        command, self.tcpInputData = self.findPacket(self.tcpInputData)
        while len(command) > 0:
            if len(command) >= 8:
                if command[1] == '\xbf':   #Get response
                    dataSize = ord(command[6]) * 256 + ord(command[7])
                    if dataSize > 0 and dataSize <= (len(command)-8):
                        self.process(command[8:8+dataSize])
                        self.transmit(self.getOutputData())
                elif command[1] == '\xb3':   #Flush
                    self.transmit(self.processPmacFlush())
                elif command[1] == '\xc2':   #Read ready
                    self.transmit(self.processReadReady())
            command, self.tcpInputData = self.findPacket(self.tcpInputData)

    def findPacket(self, data):
        '''Looks for a complete packet in the input stream. Returns a
           tuple containing the command packet followed by any remaining data.'''
        command = ""
        remaining = ""
        found = False
        dataSize = 0
        state = "searching"
        for ch in data:
            if state == "searching":
                if ch == '\x40' or ch == '\xc0':
                    command += ch
                    state = "header"
            elif state == "header":
                command += ch
                if len(command) == 7:
                    dataSize = ord(ch) * 256
                elif len(command) == 8:
                    dataSize += ord(ch)
                    if dataSize == 0:
                        state = "remaining"
                        found = True
                    else:
                        state = "data"
            elif state == "data":
                command += ch
                if (len(command) - 8) >= dataSize:
                    state = "remaining"
                    found = True
            elif state == "remaining":
                remaining += ch
        if found:
            return (command, remaining)
        else:
            return ('', command)

    def getOutputData(self):
        '''Returns the next block of data to return to the host.  The data
        is selected according to the manual.'''
        result = ""
        stopOnCr = False
        for ch in self.tcpOutputData:
            result += ch
            if ch == '\x06' or ch == '\n':
                break
            elif ch == '\x07' or ch == '\x02':
                stopOnCr = True
            elif ch == '\r' and stopOnCr:
                break
            elif len(result) >= 1440:
                break
        self.tcpOutputData = self.tcpOutputData[len(result):]
        if len(result) > 0:
            result += '\x06'
        return result
    
    def disconnectInd(self):
        '''Override to receive disconnect indications for the socket.'''
        #self.xps.userClosed(self)

    def reply(self, data):
        self.tcpOutputData += data

    def processPmacFlush(self):
        '''Processes a PMAC flush command by reinitialising the
        input state of the controller. The manual states one character
        is returned but does not indicate the value.  The support does
        not check the value.'''
        self.cmdState = "none"
        self.cmdParameter = ""
        self.cmdAddress = ""
        self.tcpOutputData = ""
        return '\x40'

    def processReadReady(self):
        '''Returns a string consisting of 1 character which is NULL if there
        is no data ready to read.  Two characters are returned, the first
        is zero for no data to read.  The second is unspecified.'''
        if len(self.tcpOutputData) == 0:
            return '\x00\x40'
        else:
            return '\x01\x40'

class TcpUserServer(dls_simulationlib.simsocket.listenSocket):
    def __init__(self, pmac, tcpPort):
        dls_simulationlib.simsocket.listenSocket.__init__(self)
        self.pmac = pmac
        self.tcpPort = tcpPort
        self.createTcpServer(self.tcpPort)

    def createClient(self, clientSocket):
        '''Override to create a client handler object.'''
        getUi().output("Connecting PMAC user on port %s.\n" % self.tcpPort)
        return TcpUser(clientSocket, self.pmac)


