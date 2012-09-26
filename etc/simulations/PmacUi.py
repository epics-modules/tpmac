#!/bin/env python2.4

import curses, readline

def getUi(pmac=None):
    if pmac is not None and pmac.beamline is not None:
        if pmac.name not in PmacUi.namedUi:
            PmacUi.namedUi[pmac.name] = PmacNamedUi(pmac)
        return PmacUi.namedUi[pmac.name]
    else:
        if PmacUi.ui is None:
            PmacUi.ui = PmacUi(True)  # No UI made yet, create one with no console
        return PmacUi.ui

class PmacUi(object):
    ui = None
    namedUi = {}

    def __init__(self, noConsole):
        self.noConsole = noConsole
        if not self.noConsole:
            # Initialise the library and partition the screen
            self.wholeScr = curses.initscr()
            self.partitionScreen()
        # Initialise the source display variables
        self.fileName = ''
        self.fileContents = []
        self.lineNumber = -1
        self.firstDisplayLine = 0
        # Initialise the output window variables
        self.outputX = 0
        self.outputY = 0
        self.outputLines = ['']
        self.outputNewLine = False
        # Initialise the watch window variables
        self.watchVars = {}
        self.activeMachine = None
        # Initialise the trace log
        self.traceLogOn = False
        self.traceFileName = ''

    def setTraceLog(self, fileName):
        self.traceLogOn = True
        self.traceFileName = fileName

    def partitionScreen(self):
        sizeY, sizeX = self.wholeScr.getmaxyx()
        self.commandWindow = curses.newwin(1,sizeX, sizeY-1,0)
        self.outputWindow = curses.newwin(10,sizeX, sizeY-11,0)
        self.watchWindow = curses.newwin(sizeY-12,20, 0,0)
        self.sourceWindow = curses.newwin(sizeY-12,sizeX-21, 0,21)
        self.wholeScr.hline(sizeY-12,0, curses.ACS_HLINE, sizeX)
        self.wholeScr.vline(0,20, curses.ACS_VLINE, sizeY-11)
        self.wholeScr.refresh()
        self.outputWindow.idlok(1)

    def cleanUp(self):
        if not self.noConsole:
            curses.endwin()

    def displayLine(self, fileName, lineNumber):
        sizeY, sizeX = self.sourceWindow.getmaxyx()
        if self.fileName != fileName:
            # Read in the new file
            self.debug("Reading file %s" % fileName)
            self.fileName = fileName
            self.fileContents = []
            try:
                file = open(self.fileName, "r")
            except IOError:
                self.output("Cannot open file %s\n" % fileName)
            else:
                for line in file:
                    line = line.expandtabs()
                    line = line[0:sizeX-1].rstrip()
                    self.fileContents.append(line)
            self.fileName = fileName
            self.lineNumber = -1
        if len(self.fileContents) > 0 and self.lineNumber != lineNumber and \
                lineNumber <= len(self.fileContents):
            if False: #lineNumber >= self.firstDisplayLine and \
                #lineNumber < (self.firstDisplayLine + sizeY - 2):
                # The line is on the screen, just move the highlight
                self.sourceWindow.addstr(self.lineNumber-self.firstDisplayLine+1,1, 
                    self.fileContents[self.lineNumber], curses.A_NORMAL)
                self.sourceWindow.addstr(lineNumber-self.firstDisplayLine+1,1, 
                    self.fileContents[lineNumber], curses.A_BOLD)
            else:
                # Display the current file contents, centered on the line number
                self.firstDisplayLine = lineNumber - sizeY/2
                if self.firstDisplayLine < 0:
                    self.firstDisplayLine = 0
                for i in range(sizeY):
                    displayLine = self.firstDisplayLine + i
                    displayText = ''
                    if (displayLine-1) < len(self.fileContents):
                        displayText = self.fileContents[displayLine-1]
                    self.sourceWindow.hline(i,0, ' ', sizeX)
                    if len(displayText) > 0:
                        attr = curses.A_NORMAL
                        if displayLine == lineNumber:
                            attr = curses.A_BOLD
                        try:
                            self.sourceWindow.addnstr(i,0, displayText, sizeX, attr)
                        except:
                            getUi().output("addstr error [%s]\n" % repr(displayText))
            self.lineNumber = lineNumber
            self.sourceWindow.refresh()
            # Output to the log file
            self.outputToTraceLog('%s:%s %s\n' % (fileName, lineNumber,
                self.fileContents[lineNumber-1]))
            self.debug('Line %s: %s\n' % (lineNumber, self.fileContents[lineNumber-1]))

    def outputToTraceLog(self, text):
        if self.traceLogOn:
            logFile = open(self.traceFileName, 'a')
            logFile.write(text)

    def debug(self, text):
        logFile = open('debug.txt', 'a')
        logFile.write(text)

    def output(self, text):
        if self.noConsole:
            print text
        else:
            maxY, maxX = self.outputWindow.getmaxyx()
            for ch in text:
                if self.outputNewLine:
                    self.outputLines.append('')
                    self.outputY += 1
                    self.outputX = 0
                    if self.outputY >= maxY:
                        self.outputLines = self.outputLines[1:]
                        for line in range(maxY):
                            self.outputWindow.hline(line,0, ' ', maxX)
                            self.outputWindow.addstr(line,0, self.outputLines[line])
                        self.outputY = maxY-1
                    self.outputNewLine = False
                if ch == '\n':
                    self.outputNewLine = True
                else:
                    self.outputLines[-1] = self.outputLines[-1] + ch
                    try:
                        self.outputWindow.addch(self.outputY, self.outputX, ch)
                    except:
                        print "addch error: %s,%s, %s, %s,%s" % (self.outputY, self.outputX, 
                            ch, maxY, maxX)
                    self.outputX += 1
                    if self.outputX >= (maxX-1):
                        self.outputNewLine = True
            self.outputWindow.refresh()
        
    def inputLine(self, prompt):
        maxY, maxX = self.commandWindow.getmaxyx()
        self.commandWindow.hline(0,0, ' ', maxX)
        self.commandWindow.addstr(0,0, prompt)
        self.commandWindow.refresh()
        return self.commandWindow.getstr()

    def watchVar(self, machine, var, val):
        #if machine == self.activeMachine:
        if var not in self.watchVars:
            # Allocate a line for this watch variable
            self.watchVars[var] = len(self.watchVars)
        self.updateVar(machine, var, val)

    def updateVar(self, machine, var, val):
        #if machine == self.activeMachine:
        if var in self.watchVars:
            # Write the watch line
            line = self.watchVars[var]
            self.outputWatch(line, var, val)
            # Output to the log file
            self.outputToTraceLog('%s = %s\n' % (var, val))
            self.debug('%s = %s\n' % (var, val))

    def outputWatch(self, line, var, val):
        maxY, maxX = self.watchWindow.getmaxyx()
        self.watchWindow.hline(line,0, ' ', maxX)
        self.watchWindow.addstr(line,0, '%s=%s' % (var, val))
        self.watchWindow.refresh()

    def updateMVar(self, machine, chTypeString, chAddress):
        # Called when a memory location changes to update any
        # watched M-variable that points to the address.
        #if machine == self.activeMachine:
        for var,text in self.watchVars.iteritems():
            if var[0] == 'm':
                addr = machine.mVariables[int(var[1:])]
                typeString, address, offset, size, formatString = \
                    machine.decodeMVarAddress(addr)
                if typeString == chTypeString and address == chAddress:
                    valid, val = machine.getMemory(addr)
                    # Write the watch line
                    maxY, maxX = self.watchWindow.getmaxyx()
                    line = self.watchVars[var]
                    self.watchWindow.hline(line,0, ' ', maxX)
                    self.watchWindow.addstr(line,0, '%s=%s' % (var, val))
                    self.watchWindow.refresh()
                    # Output to the log file
                    self.outputToTraceLog('%s = %s\n' % (var, val))
                    self.debug('%s = %s\n' % (var, val))

    def unwatchVar(self, machine, var):
        #if machine == self.activeMachine:
        if var in self.watchVars:
            maxY, maxX = self.watchWindow.getmaxyx()
            line = self.watchVars[var]
            self.watchWindow.hline(line,0, ' ', maxX)
            self.watchWindow.refresh()
            del self.watchVars[var]

    def setActiveMachine(self, machine):
        self.activeMachine = machine

class PmacNamedUi(PmacUi):
    def __init__(self, pmac):
        self.pmac = pmac
        super(PmacNamedUi, self).__init__(True)
        self.console = self.pmac.beamline.addSimulationView(self.pmac, "console",
            self.pmac.beamline.UI_CONSOLE)
        self.watch = self.pmac.beamline.addSimulationView(self.pmac, "watch",
            self.pmac.beamline.UI_PAIRS)
        self.source = self.pmac.beamline.addSimulationView(self.pmac, "file",
            self.pmac.beamline.UI_FILE)
        
    def output(self, text):
        self.console.output(text)

    def outputWatch(self, line, var, val):
        self.watch.output(line, var, val)

    def displayLine(self, fileName, lineNumber):
        self.source.displayFile(fileName)
        self.source.centreOn(lineNumber)
        
        sizeY, sizeX = self.sourceWindow.getmaxyx()
        if self.fileName != fileName:
            # Read in the new file
            self.debug("Reading file %s" % fileName)
            self.fileName = fileName
            self.fileContents = []
            try:
                file = open(self.fileName, "r")
            except IOError:
                self.output("Cannot open file %s\n" % fileName)
            else:
                for line in file:
                    line = line.expandtabs()
                    line = line[0:sizeX-1].rstrip()
                    self.fileContents.append(line)
            self.fileName = fileName
            self.lineNumber = -1
        if len(self.fileContents) > 0 and self.lineNumber != lineNumber and \
                lineNumber <= len(self.fileContents):
            if False: #lineNumber >= self.firstDisplayLine and \
                #lineNumber < (self.firstDisplayLine + sizeY - 2):
                # The line is on the screen, just move the highlight
                self.sourceWindow.addstr(self.lineNumber-self.firstDisplayLine+1,1, 
                    self.fileContents[self.lineNumber], curses.A_NORMAL)
                self.sourceWindow.addstr(lineNumber-self.firstDisplayLine+1,1, 
                    self.fileContents[lineNumber], curses.A_BOLD)
            else:
                # Display the current file contents, centered on the line number
                self.firstDisplayLine = lineNumber - sizeY/2
                if self.firstDisplayLine < 0:
                    self.firstDisplayLine = 0
                for i in range(sizeY):
                    displayLine = self.firstDisplayLine + i
                    displayText = ''
                    if (displayLine-1) < len(self.fileContents):
                        displayText = self.fileContents[displayLine-1]
                    self.sourceWindow.hline(i,0, ' ', sizeX)
                    if len(displayText) > 0:
                        attr = curses.A_NORMAL
                        if displayLine == lineNumber:
                            attr = curses.A_BOLD
                        try:
                            self.sourceWindow.addnstr(i,0, displayText, sizeX, attr)
                        except:
                            getUi().output("addstr error [%s]\n" % repr(displayText))
            self.lineNumber = lineNumber
            self.sourceWindow.refresh()
            # Output to the log file
            self.outputToTraceLog('%s:%s %s\n' % (fileName, lineNumber,
                self.fileContents[lineNumber-1]))
            self.debug('Line %s: %s\n' % (lineNumber, self.fileContents[lineNumber-1]))

    
