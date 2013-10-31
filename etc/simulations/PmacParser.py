from PmacMotor import *
from PmacUi import *
import os, sys, math, re

def flatten(x):
    """flatten(sequence) -> list

    Returns a single, flat list which contains all elements retrieved
    from the sequence and all recursively contained sub-sequences
    (iterables).

    Examples:
    >>> [1, 2, [3,4], (5,6)]
    [1, 2, [3, 4], (5, 6)]
    >>> flatten([[[1,2,3], (42,None)], [4,5], [6], 7, MyVector(8,9,10)])
    [1, 2, 3, 42, None, 4, 5, 6, 7, 8, 9, 10]"""

    result = []
    for el in x:
        #if isinstance(el, (list, tuple)):
        if hasattr(el, "__iter__") and not isinstance(el, basestring):
            result.extend(flatten(el))
        else:
            result.append(el)
    return result

class PmacToken(object):
    def __init__(self, val, fileName="", lineNumber=0):
        self.val = val
        self.fileName = fileName
        self.lineNumber = lineNumber

    def __str__(self):
        return str(self.val)

    def __repr__(self):
        return repr(self.val)

    def __eq__(self, other):
        if isinstance(other, PmacToken):
            return self.val == other.val
        else:
            return self.val == other

    def __ne__(self, other):
        return not self.__eq__(other)

    def __getitem__(self, index):
        return self.val[index]

    def isdigit(self):
        return self.val.isdigit()

    def __int__(self):
        return int(self.val)

    def __float__(self):
        return float(self.val)

    def __len__(self):
        return len(self.val)

    def isInt(self):
        '''Returns True if the token is a number or a hexadecimal number.'''
        if self.val[0] == '$' and len(self.val) > 1:
            result = True
            for ch in self.val[1:]:
                if result:
                    if ch >= '0' and ch <= '9':
                        pass
                    elif ch >= 'a' and ch <= 'f':
                        pass
                    else:
                        result = False
        else:
            result = self.isdigit()
        return result

    def isFloat(self):
        result = True
        if self.isInt():
            result = True
        else:
            for ch in self.val:
                if result:
                    if ch >= '0' and ch <='9':
                        pass
                    elif ch == '.':
                        pass
                    else:
                        result = False
        return result

    def isName(self):
        return self.val.isalpha()

    def toInt(self):
        if self.val[0] == '$':
            result = int(self.val[1:], 16)
        else:
            result = int(self.val)
        return result

    def toFloat(self):
        result = 0.0
        if self.isInt():
            result = float(self.toInt())
        else:
            result = float(self.val)
        return result

    def isString(self):
        return len(self.val) >= 2 and self.val[0] == '"' and self.val[-1] == '"'

    def toString(self):
        return self.val[1:-1]

class PmacParser(object):
    printPolling = False
    printCmd = False

    def __init__(self, pmac):
        self.tokens = []
        self.bufferOpen = False
        self.pmac = pmac
        self.currentMotor = PmacToken('1','',0)
        self.currentCoordSystem = PmacToken('1','',0)
        self.outputData = ""
        self.debug = False
        self.trace = False
        self.traceOffset = 0
        self.lineNumber = 0
        self.programMode = False
        self.debugFileName = ""
        self.debugLineNumber = 0
        self.traceFileName = ""
        self.traceLineNumber = 0
        self.traceWindow = None
        self.atBreakPoint = False
        self.stoppedAtFileName = ''
        self.stoppedAtLineNumber = 0
        self.pollLines = [\
            r'#\d+ \? P F',                       # Motor poll
            r'\?\?\?',                            # Controller poll
            r'P465',
            r'P466',
            r'P467',
            r'P468',
            r'P469',
            r'P470',
            r'P471',
            r'P472',
            r'&\d+\?\?',                          # Coordinate system state poll
            r'&\d+Q81Q82Q83Q84Q85Q86Q87Q88Q89']   # Coordinate system position poll

    def getCurrentAxis(self):
        return self.tokenToInt(self.currentMotor)

    def isPollLine(self, text):
        result = False
        for pollLine in self.pollLines:
            if not result:
                if re.match(pollLine, text) is not None:
                    result = True
        return result

    def executionYield(self):
        # Override this function to yield execution of a buffer
        pass

    def process(self, text, noPrint=False):
        '''The main entry point for the PMAC parser.'''
        parts = text.split()
        if len(parts) >= 3 and parts[0] == ';#*':
            # Debug information line
            self.debugFileName = parts[1]
            self.debugLineNumber = int(parts[2])
            # Register the filename for shortcut purposes
            self.pmac.registerPmacFileName(self.debugFileName)
        else:
            # Normal line
            self.lineNumber += 1
            # Ignore anything after the semi-colon
            parts = text.split(';', 2)
            if len(parts) > 0:
                self.tokens += self.lexer(parts[0])
                # If a buffer is open, look for a close in the token stream
                if self.bufferOpen and "close" in self.tokens:
                    self.bufferOpen = False
                # If a buffer is not open, parse the tokens now
                if not self.bufferOpen:
                    self.processTokens(noPrint, text)

    def processTokens(self, noPrint=True, text="", programMode=False):
        self.programMode = programMode
        self.parse()
        self.outputData = self.outputData.rstrip()
        if (PmacParser.printPolling and self.isPollLine(text)) or \
                (PmacParser.printCmd and not self.isPollLine(text)):
            if len(text) > 0:
                getUi(self.pmac).debug('>>%s\n' % repr(text))
                if len(self.outputData) > 0:
                    getUi(self.pmac).debug('<<%s\n' % repr(self.outputData))
        self.reply(self.outputData + '\r')
        self.outputData = ''

    def nestedProcessTokens(self, tokens):
        savedTokens = self.tokens
        self.tokens = tokens
        self.parse()
        self.tokens = savedTokens

    def nestedProcess(self, text, programMode, fileName, lineNumber):
        savedTokens = self.tokens
        savedCurrentMotor = self.currentMotor
        savedCurrentCoordSystem = self.currentCoordSystem
        savedProgramMode = self.programMode
        savedFileName = self.traceFileName
        savedLineNumber = self.traceLineNumber
        self.programMode = programMode
        self.traceFileName = fileName
        self.traceLineNumber = lineNumber
        self.tokens = self.lexer(text.lower())
        self.parse()
        self.tokens = savedTokens
        self.currentMotor = savedCurrentMotor
        self.currentCoordSystem = savedCurrentCoordSystem
        self.programMode = savedProgramMode
        self.traceFileName = savedFileName
        self.traceLineNumber = savedLineNumber

    def unsupported(self, syntax):
        print "%s: Unsupported syntax: %s" % (self.pmac.name, syntax)
        #getUi(self.pmac).output("Unsupported syntax: %s" % syntax)
            
    def getToken(self, requireLineNumbers=False):
        '''Returns the next non-line number token, removing it from the token stream.'''
        result = None
        if len(self.tokens) > 0:
            result = self.tokens[0]
            self.tokens = self.tokens[1:]
        #print "{%s}" % result,
        return result

    def putToken(self, token):
        '''Replaces a token at the front of the token stream.'''
        if token is not None:
            self.tokens[:0] = [token]

    def parse(self):
        '''Parse tokens from the input stream until none remain or a buffer is left open.'''
        while len(self.tokens) > 0 and not self.bufferOpen:
            token = self.getToken()
            if self.trace and token is not None:
                if len(token.fileName) > 0:
                    getUi(self.pmac).displayLine(token.fileName, token.lineNumber)
                if self.pmac.isAtBreakPoint(token.fileName, token.lineNumber):
                    self.atBreakPoint = True
                    self.stoppedAtFileName = token.fileName
                    self.stoppedAtLineNumber = token.lineNumber
                    self.executionYield()
                    self.atBreakPoint = False
                elif self.pmac.isSingleStep(self.stoppedAtFileName != token.fileName or \
                        self.stoppedAtLineNumber != token.lineNumber):
                    self.atBreakPoint = True
                    self.stoppedAtFileName = token.fileName
                    self.stoppedAtLineNumber = token.lineNumber
                    self.executionYield()
                    self.atBreakPoint = False
            if token is None:
                pass
            elif token == 'i':
                self.parseIVar()
            elif token == 'ms':
                self.parseMacroStation()
            elif token == 'm':
                self.parseMVar()
            elif token == 'undefine' or token == 'undef':
                self.parseUndefine()
            elif token == 'open':
                self.parseOpen(token)
            elif token == 'close':
                self.pmac.cmdClose(self)
            elif token == '#':
                self.parseHash()
            elif token == '&':
                self.parseAmpersand()
            elif token == '?':
                self.pmac.cmdReportMotorStatus(self)
            elif token == '??':
                self.pmac.cmdReportCoordSystem(self, 
                    self.tokenToInt(self.currentCoordSystem))
            elif token == '???':
                self.pmac.cmdReportGlobalStatus(self)
            elif token == 'f':
                self.pmac.cmdReportMotorFollowingError(self)
            elif token == 'p':
                self.parseP()
            elif token == 'q':
                self.parseQ()
            elif token == 'j':
                self.parseJ()
            elif self.programMode and token in ['x', 'y', 'z', 'a', 'b', 'c', 'u', 'v', 'w']:
                self.putToken(token)
                self.parseAxisMove()
            elif token == 'b':
                self.parseB()
            elif token == 'r':
                self.pmac.cmdRunProgram(self, self.tokenToInt(self.currentCoordSystem))
            elif token == 'list':
                self.parseList()
            elif token == 'enable' or token == 'ena':
                self.parseEnable()
            elif token == 'address' or token == 'adr':
                self.parseAddress()
            elif token == 'load':
                self.parseLoad()
            elif token == 'linear':
                pass    # TODO
            elif token == 'abs':
                pass    # TODO
            elif token == 'frax':
                self.parseFrax()
            elif token == 'dwell' or token == 'dwe':
                self.parseDwell()
            elif token == 'debug':
                self.parseDebug()
            elif self.programMode and token == 'while':
                self.parseWhile()
            elif self.programMode and token == 'if':
                self.parseIf()
            elif token == "$":
                pass
            elif token == "$$":
                pass
            elif token == "$$$":
                pass
            elif token in ['command', 'cmd']:
                self.parseCommand()
            elif token == "a":
                #getUi(self.pmac).output("abort %s" % repr(self.currentCoordSystem))
                self.pmac.cmdAbortCoordSystem(self.tokenToInt(self.currentCoordSystem))
            elif token == 'k':
                #getUi(self.pmac).output("kill %s" % repr(self.currentMotor))
                self.pmac.cmdKillMotor(self.tokenToInt(self.currentMotor))
            elif token == 'cid':
                self.pmac.cmdCid(self)
            elif token == 'home' or token == 'hm':
                self.pmac.cmdHome(self, self.tokenToInt(self.currentMotor))
            elif token == 'disable' or token == 'dis':
                self.parseDisable()
            elif token == 'displc':
                self.parseDisablePlc()
            elif token in ['macroslvwrite', 'msw']:
                self.parseMacroSlaveWrite()
            elif token in ['macroslvread', 'msr']:
                self.parseMacroSlaveRead()
            elif token == 'v':
                self.parseV()
            elif token in ['ver', 'version']:
                self.parseVer()
            else:
                self.unsupported(['TOP', token, self.getToken(), self.getToken()])
                # Skip unknown tokens at the top level

    def parseV(self):
        self.pmac.cmdReportMotorVelocity(self)

    def parseVer(self):
        self.pmac.cmdVer(self)

    def parseMacroSlaveWrite(self):
        node = self.getToken()
        if self.tokenIsInt(node):
            node = self.tokenToInt(node)
            token = self.getToken()
            if token == ',':
                slaveType = self.getToken()
                if slaveType in ['i', 'mi']:
                    slaveVar = self.getToken()
                    if self.tokenIsInt(slaveVar):
                        slaveVar = self.tokenToInt(slaveVar)
                        token = self.getToken()
                        if token == ',':
                            token = self.getToken()
                            pmacVar = self.getToken()
                            if self.tokenIsInt(pmacVar):
                                if token == 'i':
                                    val = self.pmac.getIVariable(self.tokenToInt(pmacVar))
                                    self.pmac.setMsIVariable(node, slaveVar, val)
                                elif token == 'p':
                                    val = self.pmac.getPVariable(self.tokenToInt(pmacVar))
                                    self.pmac.setMsIVariable(node, slaveVar, val)
                                elif token == 'q': 
                                    val = self.pmac.getCsQVariable(\
                                        self.tokenToInt(self.currentCoordSystem), 
                                        self.tokenToInt(pmacVar))
                                    self.pmac.setMsIVariable(node, slaveVar, val)
                                elif token == 'm':
                                    val = self.pmac.getMVariable(self.tokenToInt(pmacVar))
                                    self.pmac.setMsIVariable(node, slaveVar, val)
                                else:
                                    self.unsupported(['msw', node, ',', slaveType, 
                                        slaveVar, ',', token, pmacVar])
                            else:
                                self.unsupported(['msw', node, ',', slaveType, 
                                    slaveVar, ',', token, pmacVar])
                        else:
                            self.unsupported(['msw', node, ',', slaveType, slaveVar, token])
                    else:
                        self.unsupported(['msw', node, ',', slaveType, slaveVar])
                else:
                    self.unsupported(['msw', node, ',', slaveType])
            else:
                self.unsupported(['msw', node, token])
        else:
            self.unsupported(['msw', node])

    def parseMacroSlaveRead(self):
        node = self.getToken()
        if self.tokenIsInt(node):
            node = self.tokenToInt(node)
            token = self.getToken()
            if token == ',':
                slaveType = self.getToken()
                if slaveType in ['i', 'mi']:
                    slaveVar = self.getToken()
                    if self.tokenIsInt(slaveVar):
                        slaveVar = self.tokenToInt(slaveVar)
                        token = self.getToken()
                        if token == ',':
                            token = self.getToken()
                            pmacVar = self.getToken()
                            if self.tokenIsInt(pmacVar):
                                if token == 'i':
                                    val = self.pmac.getMsIVariable(node, slaveVar)
                                    self.setIVariable(self.tokenToInt(pmacVar), val)
                                elif token == 'p':
                                    val = self.pmac.getMsIVariable(node, slaveVar)
                                    self.setPVariable(self.tokenToInt(pmacVar), val)
                                elif token == 'q': 
                                    val = self.pmac.getMsIVariable(node, slaveVar)
                                    self.setCsQVariable(\
                                        self.tokenToInt(self.currentCoordSystem), 
                                        self.tokenToInt(pmacVar), var)
                                elif token == 'm':
                                    val = self.pmac.getMsIVariable(node, slaveVar)
                                    self.setMVariable(self.tokenToInt(pmacVar), val)
                                else:
                                    self.unsupported(['msr', node, ',', slaveType, 
                                        slaveVar, ',', token, pmacVar])
                            else:
                                self.unsupported(['msr', node, ',', slaveType, 
                                    slaveVar, ',', token, pmacVar])
                        else:
                            self.unsupported(['msr', node, ',', slaveType, slaveVar, token])
                    else:
                        self.unsupported(['msr', node, ',', slaveType, slaveVar])
                else:
                    self.unsupported(['msr', node, ',', slaveType])
            else:
                self.unsupported(['msr', node, token])
        else:
            self.unsupported(['msr', node])

    def setIVariable(self, var, val):
#        if self.trace:
#            print "Trace: i%s <- %s" % (var, val)
        self.pmac.setIVariable(var, val)

    def setPVariable(self, var, val):
#        if self.trace:
#            print "Trace: p%s <- %s" % (var, val)
        self.pmac.setPVariable(var, val)

    def setCsQVariable(self, cs, var, val):
#        if self.trace:
#            print "Trace: q[%s]%s <- %s" % (cs, var, val)
        self.pmac.setCsQVariable(cs, var, val)

    def setMVariable(self, var, val):
#        if self.trace:
#            print "Trace: m%s <- %s" % (var, val)
        self.pmac.setMVariable(var, val)

    def parseDisable(self):
        token = self.getToken()
        if token == 'plc':
            self.parseDisablePlc()
        else:
            self.unsupported(['disable', token])

    def parseDisablePlc(self):
        token = self.getToken()
        if self.tokenIsInt(token):
            self.pmac.cmdDisablePlc(self, self.tokenToInt(token))
        else:
            self.unsupported(['disable', 'plc', token])

    def parseCommand(self):
        cmdString = self.getToken()
        if cmdString == '^':
            # It's a control code command
            cmdString = self.getToken()
            # TODO: Support control code commands
        else:
            # It's a regular command string
            if len(cmdString) >= 2 and cmdString[0] == '"' and cmdString[-1] == '"':
                self.nestedProcess(cmdString[1:-1], False, 
                    cmdString.fileName, cmdString.lineNumber)
            else:
                self.unsupported(['command', cmdString])

    def parseDebug(self):
        token = self.getToken()
        if token == 'print':
            token = self.getToken()
            if token == 'poll':
                PmacParser.printPolling = True
            elif token == 'cmd':
                PmacParser.printCmd = True
            elif token == 'move':
                pmacMotor.printMove = True
            elif token == 'off':
                pmacMotor.printMove = False
                PmacParser.printPolling = False
                PmacParser.printCmd = False
            else:
                self.unsupported(['debug', 'print', token])
        elif token == 'trace':
            token = self.getToken()
            if token == 'plc':
                num = self.getToken()
                if self.tokenIsInt(num):
                    self.pmac.cmdDebugTracePlc(self, self.tokenToInt(num))
                else:
                    self.unsupported(['debug', 'trace', 'plc', token])
            elif token in ['program', 'pgm']:
                token = self.getToken()
                if self.tokenIsInt(token):
                    self.pmac.cmdDebugTraceProgram(self, self.tokenToInt(token))
                else:
                    self.unsupported(['debug', 'trace', 'program', token])
            elif token == 'cs':
                token = self.getToken()
                if self.tokenIsInt(token):
                    self.pmac.cmdDebugTraceCs(self, self.tokenToInt(token))
                else:
                    self.unsupported(['debug', 'trace', 'program', token])
            elif token == 'off':
                self.pmacCmdDebugTraceOff(self)
            elif token == 'log':
                fileName = self.getToken()
                if self.tokenIsString(fileName):
                    self.pmac.cmdDebugTraceLog(self, fileName.toString())
                else:
                    self.unsupported(['debug', 'trace', 'log', fileName])
            else:
                self.unsupported(['debug', 'trace', token])
        elif token == 'break':
            token = self.getToken()
            if token in ['on', 'off']:
                fileName = self.getToken()
                lineNumber = self.getToken()
                if self.tokenIsString(fileName) and self.tokenIsInt(lineNumber):
                    self.pmac.cmdDebugBreak(self, fileName.toString(), lineNumber.toInt(),
                        token == 'on')
                else:
                    self.unsupported(['debug', 'break', fileName, lineNumber])
            elif token == 'clear':
                self.pmac.cmdDebugBreakClear(self)
            elif token == 'list':
                self.pmac.cmdDebugBreakList(self)
            else:
                self.unsupported(['debug', 'break', token])
        elif token == 'cont':
            self.pmac.cmdDebugContinue(self)
        elif token == 'step':
            self.pmac.cmdDebugStep(self)
        elif token == 'watch':
            token = self.getToken()
            var = self.getToken()
            if self.tokenIsInt(var):
                var = self.tokenToInt(var)
                if token == 'i':
                    self.pmac.cmdDebugWatchIVar(self, var)
                elif token == 'p':
                    self.pmac.cmdDebugWatchPVar(self, var)
                elif token == 'q':
                    self.pmac.cmdDebugWatchQVar(self, 
                        self.tokenToInt(self.currentCoordSystem), var)
                elif token == 'm':
                    self.pmac.cmdDebugWatchMVar(self, var)
                elif token == '#':
                    self.pmac.cmdDebugWatchMotor(self, var)
                else:
                    self.unsupported(['debug', 'watch', token, var])
            else:
                self.unsupported(['debug', 'watch', token, var])
        elif token == 'unwatch':
            token = self.getToken()
            var = self.getToken()
            if self.tokenIsInt(var):
                var = self.tokenToInt(var)
                if token == 'i':
                    self.pmac.cmdDebugUnwatchIVar(self, var)
                elif token == 'p':
                    self.pmac.cmdDebugUnwatchPVar(self, var)
                elif token == 'q':
                    self.pmac.cmdDebugUnwatchQVar(self, 
                        self.tokenToInt(self.currentCoordSystem), var)
                elif token == 'm':
                    self.pmac.cmdDebugUnwatchMVar(self, var)
                elif token == '#':
                    self.pmac.cmdDebugUnwatchMotor(self, var)
                else:
                    self.unsupported(['debug', 'unwatch', token, var])
            else:
                self.unsupported(['debug', 'unwatch', token, var])
        elif token == 'eloss':
            token = self.getToken()
            if token == 'on':
                self.pmac.cmdEncoderLoss(self.tokenToInt(self.currentMotor), True)
            elif token == 'off':
                self.pmac.cmdEncoderLoss(self.tokenToInt(self.currentMotor), False)
            else:
                self.unsupported(['debug', 'eloss', token])
        else:
            self.unsupported(['debug', token])

    def parseIf(self):
        cond = self.parseExpression()
        ifTokens = self.extractBody(['if'], ['endif'], ['else'])
        elseTokens = []
        token = self.getToken()
        if token == 'else':
            elseTokens = self.extractBody(['if'], ['endif'])
            token = self.getToken()
        if token == 'endif':
            if self.evaluateExpression(cond):
                self.nestedProcessTokens(ifTokens)
            elif len(elseTokens) > 0:
                self.nestedProcessTokens(elseTokens)
        else:
            self.unsupported(['if', '('] + flatten(cond) + [')', '...', token])

    def parseWhile(self):
        cond = self.parseExpression()
        bodyTokens = self.extractBody(['while'], ['endwhile', 'endw'])
        token = self.getToken()
        if token in ['endwhile', 'endw']:
            while self.evaluateExpression(cond):
                self.nestedProcessTokens(bodyTokens)
                self.executionYield()
        else:
            self.unsupported(['while', '('] + flatten(cond) + [')'] + bodyTokens + [token])

    def extractBody(self, begins, ends, middles=[]):
        # Returns the tokens that form the body of a compound statement.
        # 'begins' contains a list of tokens that form nested blocks.
        # 'ends' contains a list of tokens that end the blocks.
        # The token stream is assumed to contain the first token of 
        # the desired block.
        result = []
        depth = 0
        oldTrace = self.trace
        self.trace = False
        token = self.getToken(requireLineNumbers=True)
        while token is not None and ((depth > 0) or (not token in ends+middles)):
            result.append(token)
            if token in begins:
                depth += 1
            if token in ends:
                depth -= 1
            token = self.getToken(requireLineNumbers=True)
        self.putToken(token)
        self.trace = oldTrace
        return result

    def parseDwell(self):
        token = self.getToken()
        if self.tokenIsFloat(token):
            self.pmac.cmdDwell(self, self.tokenToInt(self.currentCoordSystem),
                self.tokenToFloat(token))
        else:
            self.unsupported(['dwell', token])

    def parseB(self):
        prog = self.getToken()
        if self.tokenIsInt(prog):
            self.pmac.cmdAssignProgram(self, self.tokenToInt(self.currentCoordSystem),
                self.tokenToInt(prog))
        else:
            self.unsupported(['b', prog])

    def parseFrax(self):
        token = self.getToken()
        if token == '(':
            going = True
            while going:
                token = self.getToken()
                if token in ['a','b','c','u','v','w','x','y','z']:
                    token = self.getToken()
                    if token == ',':
                        pass
                    elif token == ')':
                        going = False
                    else:
                        self.unsupported(['frax', '(', '...', token])
                else:
                    self.unsupported(['frax', '(', '...', token])
        else:
            self.putToken()

    def parseLoad(self):
        filename = self.getToken() + '.pmc'
        self.pmac.readPmcFile(filename)

    def parseAddress(self):
        token = self.getToken()
        if token == '#':
            value = self.getToken()
            if self.tokenIsInt(value):
                self.currentMotor = value
            elif value == 'p':
                value = self.getToken()
                if self.tokenIsInt(value):
                    self.currentMotor = PmacToken(\
                        str(int(self.pmac.getPVariable(self.tokenToInt(value)))),'',0)
                else:
                    self.unsupported(['address', token, 'p', value])
            else:
                self.unsupported(['address', token, value])
        elif token == '&':
            value = self.getToken()
            if self.tokenIsInt(value):
                self.currentCoordSystem = value
            elif value == 'p':
                value = self.getToken()
                if self.tokenIsInt(value):
                    self.currentCoordSystem = PmacToken(\
                        str(int(self.pmac.getPVariable(self.tokenToInt(value)))),'',0)
                else:
                    self.unsupported(['address', token, 'p', value])
            else:
                self.unsupported(['address', token, value])
        else:
            self.unsupported(['address', token])

    def parseEnable(self):
        token = self.getToken()
        if token == 'plc':
            plc = self.getToken()
            if self.tokenIsInt(plc):
                self.pmac.cmdEnablePlc(self, self.tokenToInt(plc))
                contToken = self.getToken()
                if contToken == ',':
                    while contToken == ',':
                        plc = self.getToken()
                        if self.tokenIsInt():
                            self.pmac.cmdEnablePlc(self, self.tokenToInt(plc))
                            contToken = self.getToken()
                        else:
                            self.unsupported(['enable', token, '...', ',', plc])
                            contToken = self.getToken()
                    self.putToken(contToken)
                elif contToken == '..':
                    plc2 = self.getToken()
                    if self.tokenIsInt(plc2):
                        for i in range(self.tokenToInt(plc)+1, self.tokenToInt(plc2)+1):
                            self.pmac.cmdEnablePlc(self, i)
                    else:
                        self.unsupported(['enable', token, plc, '..', plc2])
                else:
                    self.putToken(contToken)
            else:
                self.unsupported(['enable', token, plc])
        else:
            self.unsupported(['enable', token])

    def parseList(self):
        token = self.getToken()
        if token == 'inverse' or token == 'inv':
            self.pmac.cmdListInverse(self, self.tokenToInt(self.currentCoordSystem))
        elif token == 'forward' or token == 'fwd':
            self.pmac.cmdListForward(self, self.tokenToInt(self.currentCoordSystem))
        elif token == 'plc':
            id = self.getToken()
            if self.tokenIsInt(id):
                comma = self.getToken()
                if comma == ',':
                    self.unsupported(['list', token, id, comma])
                else:
                    self.putToken(comma)
                    self.pmac.cmdListPlc(self, self.tokenToInt(id))
            else:
                self.unsupported(['list', 'plc', id])
        elif token == 'program' or token == 'prog':
            id = self.getToken()
            if self.tokenIsInt(id):
                comma = self.getToken()
                if comma == ',':
                    self.unsupported(['list', token, id, comma])
                else:
                    self.putToken(comma)
                    self.pmac.cmdListProgram(self, self.tokenToInt(id))
            else:
                self.unsupported(['list', token, id])
        else:
            self.putToken(token)
            self.unsupported(['list'])

    def parseAxisMove(self):
        moves = []
        axis = self.getToken()
        while axis is not None and axis in ['x', 'y', 'z', 'a', 'b', 'c', 'u', 'v', 'w']:
            operator = self.getToken()
            if operator == '+':
                value = self.getToken()
                if self.tokenIsFloat(value):
                    moves.append((axis, self.tokenToFloat(value)))
                else:
                    self.unsupported([axis, operator, value])
            elif operator == '-':
                value = self.getToken()
                if self.tokenIsFloat(value):
                    moves.append((axis, -self.tokenToFloat(value)))
                else:
                    self.unsupported([axis, operator, value])
            elif operator == '=':
                self.unsupported([axis, operator])
            elif self.tokenIsFloat(operator):
                moves.append((axis, self.tokenToFloat(operator)))
            elif operator == '(':
                expr = self.parseExpression()
                if expr is not None:
                    operator = self.getToken()
                    if operator == ')':
                        moves.append((axis, self.evaluateExpression(expr)))
                    else:
                        self.unsupported([axis, '('] + expr + [operator])
            else:
                self.unsupported([axis, operator])
            axis = self.getToken()
        self.putToken(axis)
        if len(moves) > 0:
            self.pmac.cmdMoveAxis(self.tokenToInt(self.currentCoordSystem), moves)

    def parseJ(self):
        operator = self.getToken()
        if operator == '+':
            self.pmac.cmdJogPositive(self)
        elif operator == '-':
            self.pmac.cmdJogNegative(self)
        elif operator == '/':
            self.pmac.cmdJogStop(self)
        elif operator == '=':
            position = self.getToken()
            negative = False
            if position == '-':
                negative = True
                position = self.getToken()
            if self.tokenIsFloat(position):
                if negative:
                    self.pmac.cmdJogTo(self, -self.tokenToFloat(position))
                    self.parseJogCommandTrigger()
                else:
                    self.pmac.cmdJogTo(self, self.tokenToFloat(position))
                    self.parseJogCommandTrigger()
            else:
                self.unsupported(['j', operator, position])
        elif operator == '^':
            token = self.getToken()
            if token == '*':
                self.pmac.cmdJogRelativeVariable(self, self.tokenToInt(self.currentMotor))
                self.parseJogCommandTrigger()
            elif self.tokenIsFloat(token):
                self.pmac.cmdJogRelative(self, self.tokenToInt(self.currentMotor),
                    self.tokenToFloat(token))
                self.parseJogCommandTrigger()
            else:
                self.unsupported(['j', operator, token])
        else:
            self.unsupported(['j', operator])

    def parseJogCommandTrigger(self):
        token = self.getToken()
        if token == '^':
            token = self.getToken()
            if self.tokenIsFloat(token):
                self.pmac.cmdJogStopTrigger(self, 
                    self.tokenToInt(self.currentMotor), self.tokenToFloat(token))
            else:
                self.unsupported(['j', '...', '^', token])
        else:
            self.putToken(token)

    def parseAmpersand(self):
        number = self.getToken()
        if self.tokenIsInt(number):
            self.currentCoordSystem = number
        else:
            self.putToken(number)
            self.output('%s\n' % self.currentCoordSystem)

    def parseHash(self):
        number = self.getToken()
        if self.tokenIsInt(number):
            operator = self.getToken()
            if operator == '->':
                token = self.getToken()
                if token == 'o':
                    self.pmac.cmdClearAxisDef(self, 
                        self.tokenToInt(self.currentCoordSystem), 
                        self.tokenToInt(number))
                elif token == 'i':
                    token2 = self.getToken()
                    if token2 == '+' or token2 == '-':
                        offset = self.getToken()
                        if self.tokenIsFloat(offset):
                            self.pmac.cmdAssignAxisDef(self, 
                                self.tokenToInt(self.currentCoordSystem), 
                                self.tokenToInt(number), 
                                ['i', token2, offset])
                        else:
                            self.unsupported(['#', number, token, token2, offset])
                    else:
                        self.putToken(token2)
                        self.pmac.cmdAssignAxisDef(self, 
                            self.tokenToInt(self.currentCoordSystem), 
                            self.tokenToInt(number), ['i'])
                elif self.tokenIsFloat(token):
                    self.putToken(token)
                    self.parseAxisDefinition(number)
                elif token in ['x', 'y', 'z', 'a', 'b', 'c', 'u', 'v', 'w', '+', '-']:
                    self.putToken(token)
                    self.parseAxisDefinition(number)
                else:
                    self.putToken(token)
                    self.pmac.cmdReportCoordSystemAxisDef(self,
                        self.tokenToInt(self.currentCoordSystem), 
                        self.tokenToInt(number))
            else:
                self.putToken(operator)
                self.currentMotor = number
        else:
            self.putToken(number)
            self.output('%s\n' % self.currentMotor)

    def parseAxisDefinition(self, motor):
        axisDef = []
        going = True
        leading = self.getToken()
        if leading == '+':
            pass
        elif leading == '-':
            axisDef.append(leading)
        else:
            self.putToken(leading)
        while going:
            axis = self.getToken()
            scaleFactor = '1.0'
            if self.tokenIsFloat(axis):
                scaleFactor = axis
                axis = self.getToken()
            if self.tokenToFloat(scaleFactor) != 1.0:
                axisDef.append(scaleFactor)
            if axis in ['x', 'y', 'z', 'a', 'b', 'c', 'u', 'v', 'w']:
                axisDef.append(axis)
                offset = '0.0'
                operator = self.getToken()
                if operator == '+' or operator == '-':
                    offset = self.getToken()
                    if self.tokenIsFloat(offset):
                        nextAxis = self.getToken()
                        if nextAxis in ['x', 'y', 'z', 'a', 'b', 'c', 'u', 'v', 'w']:
                            self.putToken(nextAxis)
                            self.putToken(offset)
                            self.putToken(operator)
                            offset = '0.0'
                        else:
                            self.putToken(nextAxis)
                    else:
                        self.putToken(offset)
                        self.putToken(operator)
                        offset = '0.0'
                else:
                    self.putToken(operator)
                if self.tokenToFloat(offset) != 0.0:
                    axisDef.append(operator)
                    axisDef.append(offset)
                operator = self.getToken()
                if operator == '+' or operator == '-':
                    axisDef.append(operator)
                else:
                    self.putToken(operator)
                    going = False
            else:
                self.unsupported(['#', motor, '->'] + axisDef + [axis])
        self.pmac.cmdAssignAxisDef(self, 
            self.tokenToInt(self.currentCoordSystem),
            self.tokenToInt(motor), axisDef)

    def parseOpen(self, openToken):
        # If there's a close in the token queue, continue processing
        if 'close' in self.tokens:
            what = self.getToken()
            if what == 'plc':
                address = self.getToken()
                if self.tokenIsInt(address):
                    self.parseOpenPlc(address)
                else:
                    self.unsupported(['open', what, address])
            elif what == 'prog' or what == 'program':
                address = self.getToken()
                if self.tokenIsInt(address):
                    self.parseOpenProgram(address)
                else:
                    self.unsupported(['open', what, address])
            elif what == 'forward' or what == 'fwd':
                self.parseOpenForward()
            elif what == 'inverse' or what == 'inv':
                self.parseOpenInverse()
            else:
                self.unsupported(['open', what])
        else:
            self.putToken(openToken)
            self.bufferOpen = True

    def parseOpenForward(self):
        token = self.getToken(requireLineNumbers=True)
        while token != 'close':
            if token == 'clear':
                self.pmac.cmdClearForward(self, 
                    self.tokenToInt(self.currentCoordSystem))
            else:
                self.pmac.cmdAddForward(self, 
                    self.tokenToInt(self.currentCoordSystem), token)
            token = self.getToken(requireLineNumbers=True)

    def parseOpenInverse(self):
        token = self.getToken(requireLineNumbers=True)
        while token != 'close':
            if token == 'clear':
                self.pmac.cmdClearInverse(self, 
                    self.tokenToInt(self.currentCoordSystem))
            else:
                self.pmac.cmdAddInverse(self, 
                    self.tokenToInt(self.currentCoordSystem), token)
            token = self.getToken(requireLineNumbers=True)

    def parseOpenPlc(self, plc):
        token = self.getToken(requireLineNumbers=True)
        while token != 'close':
            if token == 'clear':
                self.pmac.cmdClearPlc(self, self.tokenToInt(plc))
            else:
                self.pmac.cmdAddPlc(self, self.tokenToInt(plc), token)
            token = self.getToken(requireLineNumbers=True)

    def parseOpenProgram(self, prog):
        token = self.getToken(requireLineNumbers=True)
        while token != 'close':
            if token == 'clear':
                self.pmac.cmdClearProgram(self, self.tokenToInt(prog))
            else:
                self.pmac.cmdAddProg(self, self.tokenToInt(prog), token)
            token = self.getToken(requireLineNumbers=True)

    def parseUndefine(self):
        allToken = self.getToken()
        if allToken == 'all':
            self.pmac.cmdUndefineAll(self)
        else:
            self.putToken(allToken)
            self.pmac.cmdUndefine(self)

    def parseMacroStation(self):
        ms = self.getToken()
        if self.tokenIsInt(ms):
            sep = self.getToken()
            if sep == ',':
                varType = self.getToken()
                if varType == 'mi' or varType == 'i':
                    addr = self.getToken()
                    if self.tokenIsInt(addr):
                        sep = self.getToken()
                        if sep == '=':
                            self.pmac.cmdSetMsIVariable(self, self.tokenToInt(ms), 
                                self.tokenToInt(addr), 
                                self.evaluateExpression(self.parseExpression()))
                        else:
                            self.putToken(sep)
                            self.pmac.cmdReportMsIVariable(self, self.tokenToInt(ms),
                                self.tokenToInt(addr))

    def parseP(self):
        token1 = self.getToken()
        if self.tokenIsInt(token1):
            self.putToken(token1)
            rang = self.parseRange()
            if rang is not None:
                token3 = self.getToken()
                if token3 == '=':
                    val = self.evaluateExpression(self.parseExpression()) 
                    self.cmdSetPVariable(self.tokenToInt(rang[0]),
                        self.tokenToInt(rang[1]), self.tokenToInt(rang[2]),
                        val)
                else:
                    self.putToken(token3)
                    self.pmac.cmdReportPVariable(self, self.tokenToInt(rang[0]),
                        self.tokenToInt(rang[1]), self.tokenToInt(rang[2]))
        elif token1 == '(':
            addrExp = self.parseExpression()
            token2 = self.getToken()
            if token2 == ')':
                token3 = self.getToken()
                if token3 == '=':
                    self.cmdSetPVariable(int(self.evaluateExpression(addrExp)), 
                        1, 1, self.evaluateExpression(self.parseExpression()))
        else:
            self.putToken(token1)
            self.pmac.cmdReportMotorPosition(self)

    def cmdSetPVariable(self, low, high, incr, val):
        #if self.trace:
        #    print "Trace: p{%s,%s,%s} <- %s" % (low, high, incr, val)
        self.pmac.cmdSetPVariable(self, low, high, incr, val)

    def parseQ(self):
        token1 = self.getToken()
        if self.tokenIsInt(token1):
            self.putToken(token1)
            range = self.parseRange()
            if range is not None:
                token3 = self.getToken()
                if token3 == '=':
                    self.pmac.cmdSetQVariable(self, 
                        self.tokenToInt(self.currentCoordSystem), self.tokenToInt(range[0]), 
                        self.tokenToInt(range[1]), self.tokenToInt(range[2]), 
                        self.evaluateExpression(self.parseExpression()))
                else:
                    self.putToken(token3)
                    self.pmac.cmdReportQVariable(self, 
                        self.tokenToInt(self.currentCoordSystem), self.tokenToInt(range[0]), 
                        self.tokenToInt(range[1]), self.tokenToInt(range[2]))
        elif token1 == '(':
            addrExp = self.parseExpression()
            token2 = self.getToken()
            if token2 == ')':
                token3 = self.getToken()
                if token3 == '=':
                    self.pmac.cmdSetQVariable(self, 
                        self.tokenToInt(self.currentCoordSystem),
                        int(self.evaluateExpression(addrExp)), 
                        1, 1, self.evaluateExpression(self.parseExpression()))
        else:
            self.putToken(token1)
            self.pmac.cmdQuitProgramAtEndOfMove(self)

    def cmdSetQVariable(self, cs, low, high, incr, val):
        #if self.trace:
        #    print "Trace: q[%s]{%s,%s,%s} <- %s" % (cs, low, high, incr, val)
        self.pmac.cmdSetQVariable(self, cs, low, high, incr, val)

    def parseIVar(self):
        token1 = self.getToken()
        if self.tokenIsInt(token1):
            self.putToken(token1)
            range = self.parseRange()
            if range is not None:
                token3 = self.getToken()
                if token3 == '=':
                    token4 = self.getToken()
                    if token4 == '*':
                        self.cmdSetIVariable(self.tokenToInt(range[0]), 
                            self.tokenToInt(range[1]), self.tokenToInt(range[2]), '*')
                    else:
                        self.putToken(token4)
                        self.cmdSetIVariable(self.tokenToInt(range[0]), 
                            self.tokenToInt(range[1]), self.tokenToInt(range[2]), 
                            self.evaluateExpression(self.parseExpression()))
                else:
                    self.putToken(token3)
                    self.pmac.cmdReportIVariable(self, self.tokenToInt(range[0]), 
                        self.tokenToInt(range[1]), self.tokenToInt(range[2]))
        elif token1 == '(':
            addrExp = self.parseExpression()
            token2 = self.getToken()
            if token2 == ')':
                token3 = self.getToken()
                if token3 == '=':
                    self.cmdSetIVariable(int(self.evaluateExpression(addrExp)), 
                        1, 1, self.evaluateExpression(self.parseExpression()))

    def cmdSetIVariable(self, low, high, incr, val):
        #if self.trace:
        #    print "Trace: i{%s,%s,%s} <- %s" % (low, high, incr, val)
        self.pmac.cmdSetIVariable(self, low, high, incr, val)

    def parseMVar(self):
        token1 = self.getToken()
        if self.tokenIsInt(token1):
            self.putToken(token1)
            range = self.parseRange()
            if range is not None:
                token3 = self.getToken()
                if token3 == '=':
                    self.cmdSetMVariable(self.tokenToInt(range[0]), 
                        self.tokenToInt(range[1]), self.tokenToInt(range[2]), 
                        self.evaluateExpression(self.parseExpression()))
                elif token3 == '->':
                    self.parseMVarAddress(range)
                else:
                    self.putToken(token3)
                    self.pmac.cmdReportMVariable(self, self.tokenToInt(range[0]), 
                        self.tokenToInt(range[1]), self.tokenToInt(range[2]))
        elif token1 == '(':
            addrExp = self.parseExpression()
            token2 = self.getToken()
            if token2 == ')':
                token3 = self.getToken()
                if token3 == '=':
                    self.cmdSetMVariable(int(self.evaluateExpression(addrExp)), 
                        1, 1, self.evaluateExpression(self.parseExpression()))
    
    def cmdSetMVariable(self, low, high, incr, val):
        #if self.trace:
        #    print "Trace: m{%s,%s,%s} <- %s" % (low, high, incr, val)
        self.pmac.cmdSetMVariable(self, low, high, incr, val)

    def parseMVarAddress(self, range):
        valid = False
        address = ''
        addrType = self.getToken()
        if addrType == 'd' or addrType == 'dp' or addrType == 'l' or\
                addrType == 'f' or addrType ==  'tws':
            address = self.getToken()
            if address == ':':
                address = self.getToken()
            if self.tokenIsInt(address):
                offset = 0
                width = 1
                format = 'u'
                valid = True
                addressVal = self.tokenToInt(address)
        elif addrType == '*':
            offset = 0
            width = 0
            format = ''
            addressVal = 0
            valid = True
        elif addrType == 'twb':
            pass   # TODO
        elif addrType == 'twd':
            pass   # TODO
        elif addrType == 'twr':
            pass   # TODO
        elif addrType == 'x' or addrType == 'y':
            address = self.getToken()
            if address == ':':
                address = self.getToken()
            if self.tokenIsInt(address):
                addressVal = self.tokenToInt(address)
                sep = self.getToken()
                if sep == ',':
                    offset = self.getToken()
                    if self.tokenIsInt(offset):
                        offset = self.tokenToInt(offset)
                        sep = self.getToken()
                        if sep == ',':
                            width = self.getToken()
                            if self.tokenIsInt(width):
                                width = self.tokenToInt(width)
                                sep = self.getToken()
                                if sep == ',':
                                    format = self.getToken()
                                    if format == 'u' or format == 's' or \
                                            format == 'd' or format == 'c':
                                        valid = True
                                else:
                                    self.putToken(sep)
                                    format = 'u'
                                    valid = True
                        else:
                            self.putToken(sep)
                            width = 1
                            format = 'u'
                            valid = True
        else:
            self.putToken(addrType)
            self.pmac.cmdReportMVariableAddress(self, self.tokenToInt(range[0]), 
                self.tokenToInt(range[1]), self.tokenToInt(range[2]))
        if valid:
            self.pmac.cmdSetMVariableAddress(self, self.tokenToInt(range[0]), 
                self.tokenToInt(range[1]), self.tokenToInt(range[2]), 
                addrType, addressVal, offset, width, format)

    def parseRange(self):
        '''Returns a list of three elements representing the range.'''
        result = None
        start = self.getToken()
        if self.tokenIsInt(start):
            sep = self.getToken()
            if sep == ',':
                number = self.getToken()
                if self.tokenIsInt(number):
                    sep = self.getToken()
                    if sep == ',':
                        step = self.getToken()
                        if self.tokenIsInt(step):
                            result = [start, number, step]
                    else:
                        self.putToken(sep)
                        result = [start, number, 
                            PmacToken('1', start.fileName, start.lineNumber)]
            elif sep == '..':
                end = self.getToken()
                if self.tokenIsInt(end):
                    result = [start, 
                        PmacToken(str(self.tokenToInt(end)-self.tokenToInt(start)+1),
                            start.fileName, start.lineNumber), 
                        PmacToken('1', start.fileName, start.lineNumber)]
            else:
                self.putToken(sep)
                result = [start, 
                    PmacToken('1', start.fileName, start.lineNumber),
                    PmacToken('1', start.fileName, start.lineNumber)]
        return result

    def parseExpression(self):
        '''Returns the list of tokens that form the expression.'''
        result = [self.parseExpressionLowPriority()]
        operator = self.getToken()
        while operator == 'and' or operator == 'or':
            result.append(operator)
            result.append(self.parseExpressionLowPriority())
            operator = self.getToken()
        self.putToken(operator)
        return result

    def parseExpressionLowPriority(self):
        '''Returns an expression using the low priority operators.'''
        result = [self.parseExpressionMiddlePriority()]
        operator = self.getToken()
        while operator in ['>', '=', '<', '!=', '!>', '!<']:
            result.append(operator)
            result.append(self.parseExpressionMiddlePriority())
            operator = self.getToken()
        self.putToken(operator)
        return result

    def parseExpressionMiddlePriority(self):
        '''Returns an expression using the middle priority operators.'''
        result = [self.parseExpressionHighPriority()]
        operator = self.getToken()
        while operator in ['+', '-', '|', '^']:
            result.append(operator)
            result.append(self.parseExpressionHighPriority())
            operator = self.getToken()
        self.putToken(operator)
        return result

    def parseExpressionHighPriority(self):
        '''Returns an expression using the higher priority operators.'''
        result = [self.parseMonadic()]
        operator = self.getToken()
        while operator in ['*', '/', '%', '&']:
            result.append(operator)
            result.append(self.parseMonadic())
            operator = self.getToken()
        self.putToken(operator)
        return result

    def parseMonadic(self):
        result = []
        token = self.getToken()
        if token == '-':
            result.append(token)
            result.append(self.parseOperand())
        else:
            self.putToken(token)
            result = self.parseOperand()
        return result;

    def parseOperand(self):
        '''Returns the list of tokens that form an operand.'''
        expr = []
        token = self.getToken()
        if token == '(':
            subExpr = self.parseExpression()
            token = self.getToken()
            if token == ')':
                expr.append(subExpr)
            else:
                self.unsupported(flatten(expr) + ['('] + flatten(subExpr) + [token])
        elif token == 'i':
            token2 = self.getToken()
            if self.tokenIsInt(token2):
                expr.append(token)
                expr.append(token2)
            elif token2 == '(':
                subExpr = self.parseExpression()
                token3 = self.getToken()
                if token3 == ')':
                    expr.append(token)
                    expr.append(subExpr)
                else:
                    self.unsupported(flatten(expr) + ['i', '('] + flatten(subExpr) + [token3])
            else:
                self.unsupported(flatten(expr) + [token, token2])
        elif token == 'm':
            token2 = self.getToken()
            if self.tokenIsInt(token2):
                expr.append(token)
                expr.append(token2)
            elif token2 == '(':
                subExpr = self.parseExpression()
                token3 = self.getToken()
                if token3 == ')':
                    expr.append(token)
                    expr.append(subExpr)
                else:
                    self.unsupported(flatten(expr) + ['m', '('] + flatten(subExpr) + [token3])
            else:
                self.unsupported(flatten(expr) + ['m', token2])
        elif token == 'p':
            token2 = self.getToken()
            if self.tokenIsInt(token2):
                expr.append(token)
                expr.append(token2)
            elif token2 == '(':
                subExpr = self.parseExpression()
                token3 = self.getToken()
                if token3 == ')':
                    expr.append(token)
                    expr.append(subExpr)
                else:
                    self.unsupported(flatten(expr) + ['p', '('] + flatten(subExpr) + [token3])
            else:
                self.unsupported(flatten(expr) + ['p', token2])
        elif token == 'q':
            token2 = self.getToken()
            if self.tokenIsInt(token2):
                expr.append(token)
                expr.append(token2)
            else:
                self.unsupported(flatten(expr) + ['q', token2])
        elif self.tokenIsFloat(token):
            expr.append(token)
        elif token == 'abs' or token == 'acos' or token == 'asin' or token == 'atan' or \
                token == 'cos' or token == 'exp' or token == 'int' or token == 'ln' or \
                token == 'sin' or token == 'sqrt' or token == 'tan':
            paren = self.getToken()
            if token == "atan" and paren == "2":
                token.val = "atan2"
                paren = self.getToken()
            if paren == '(':
                subexpr = self.parseExpression()
                paren = self.getToken()
                if paren == ')':
                    expr.append(token)
                    expr.append(subexpr)
                else:
                    self.unsupported(flatten(expr) + [token, '('] + flatten(subExpr) + [paren])
            else:
                self.unsupported(flatten(expr) + [token, paren])
        else:
            self.unsupported(flatten(expr) + [token])
                
        return expr

    def evaluateExpression(self, expr):
        result = 0.0
        if isinstance(expr, list):
            operator = '+'
            for item in expr:
                if isinstance(item, list):
                    result = self.evaluateDyadic(result, operator,
                        self.evaluateExpression(item))
                else:
                    if self.tokenIsFloat(item):
                        result = self.evaluateDyadic(result, operator, 
                            self.tokenToFloat(item))
                    elif item in ['i', 'm', 'p', 'q']:
                        operator = item
                    elif item in ['+', '-', '*', '/', '%', '&', '|', '^', '<', '=', '>', '!>', '!<']:
                        operator = item
                    elif item in ['abs', 'acos', 'asin', 'atan', 'cos', 'exp', 'int', 'ln',
                            'sin', 'sqrt', 'tan', 'atan2', 'and', 'or']:
                        operator = item
        else:
            result = self.tokenToFloat(expr)
        return result

    def evaluateDyadic(self, lhs, operator, rhs):
        result = 0.0
        if operator == '+':
            result = lhs + rhs
        elif operator == '-':
            result = lhs - rhs
        elif operator == '>':
            result = lhs > rhs
        elif operator == '!>':
            result = lhs <= rhs
        elif operator == '=':
            result = lhs == rhs
        elif operator == '!=':
            result = lhs != rhs
        elif operator == '<':
            result = lhs < rhs
        elif operator == '!<':
            result = lhs >= rhs
        elif operator == '*':
            result = lhs * rhs
        elif operator == '/':
            if rhs == 0.0:
                result = 0.0
            else:
                result = lhs / rhs
        elif operator == '%':
            if rhs == 0.0:
                result = 0.0
            else:
                result = lhs % rhs
        elif operator == '|':
            result = float(int(lhs) | int(rhs))
        elif operator == '&':
            result = float(int(lhs) & int(rhs))
        elif operator == '^':
            result = float(int(lhs) ^ int(rhs))
        elif operator == 'and':
            result = lhs and rhs
        elif operator == 'or':
            result = lhs or rhs
        elif operator == 'i':
            result = self.pmac.getIVariable(int(rhs))
        elif operator == 'q':
            result = self.pmac.getCsQVariable(self.tokenToInt(self.currentCoordSystem),
                int(rhs))
        elif operator == 'p':
            result = self.pmac.getPVariable(int(rhs))
        elif operator == 'm':
            result = self.pmac.getMVariable(int(rhs))
        elif operator == 'abs':
            result = math.fabs(rhs)
        elif operator == 'acos':
            if self.pmac.getIVariable(15) == 0:
                result = math.degrees(math.acos(rhs))
            else:
                result = math.acos(rhs)
        elif operator == 'asin':
            if self.pmac.getIVariable(15) == 0:
                result = math.degrees(math.asin(rhs))
            else:
                result = math.asin(rhs)
        elif operator == 'atan':
            if self.pmac.getIVariable(15) == 0:
                result = math.degrees(math.atan(rhs))
            else:
                result = math.atan(rhs)
        elif operator == 'cos':
            if self.pmac.getIVariable(15) == 0:
                result = math.cos(math.radians(rhs))
            else:
                result = math.cos(rhs)
        elif operator == 'exp':
            result = math.exp(rhs)
        elif operator == 'int':
            result = float(int(rhs))
        elif operator == 'ln':
            result = math.log(rhs)
        elif operator == 'sin':
            if self.pmac.getIVariable(15) == 0:
                result = math.sin(math.radians(rhs))
            else:
                result = math.sin(rhs)
        elif operator == 'sqrt':
            result = math.sqrt(rhs)
        elif operator == 'tan':
            if self.pmac.getIVariable(15) == 0:
                result = math.tan(math.radians(rhs))
            else:
                result = math.tan(rhs)
        elif operator == 'atan2':
            if self.pmac.getIVariable(15) == 0:
                result = math.degrees(math.atan2(rhs, self.pmac.getQVariable(0)))
            else:
                result = math.atan2(rhs, self.pmac.getQVariable(0))
        #print "Evaluate dyadic %s %s %s = %s" % (lhs, operator, rhs, result)
        return result

    def tokenIsInt(self, token):
        '''Returns True if the token is a number or a hexadecimal number.'''
        if token is None:
            result = False
        else:
            result = token.isInt()
        return result

    def tokenIsFloat(self, token):
        if token is None:
            result = False
        else:
            result = token.isFloat()
        return result

    def tokenIsName(self, token):
        if token is None:
            result = False
        else:
            result = token.isName()
        return result

    def tokenIsString(self, token):
        if token is None:
            result = False
        else:
            result = token.isString()
        return result

    def tokenToInt(self, token):
        return token.toInt()

    def tokenToFloat(self, token):
        return token.toFloat()

    def lexer(self, text):
        '''Seperate the given text into tokens returned as a list.'''
        result = []
        token = ''
        state = "none"
        for ch in text:
            if state == "none":
                if ch.isspace():
                    pass
                elif ch == '$':
                    token = ch
                    state = "dollar"
                elif ch.isalpha():
                    token = ch.lower()
                    state = "alpha"
                elif ch.isdigit():
                    token = ch
                    state = "number"
                elif ch < ' ':
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                elif ch in ['(', ')', '=', '^']:
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                elif ch == '"':
                    token = ch
                    state = "string"
                else:
                    token = ch
                    state = "punct"
            elif state == "dollar":
                if ch.isspace():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ""
                    state = "none"
                elif ch >= '0' and ch <= '9':
                    token += ch
                    state = "hexnumber"
                elif ch >= 'a' and ch <= 'f':
                    token += ch
                    state = "hexnumber"
                elif ch >= 'A' and ch <= 'F':
                    token += ch.lower()
                    state = "hexnumber"
                elif ch.isalpha():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    state = "alpha"
                    token = ch.lower()
                elif ch < ' ':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch in ['(', ')', '=', '^']:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch == '"':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "string"
                else:
                    token += ch
                    state = "punct"
            elif state == "alpha":
                if ch.isspace():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ""
                    state = "none"
                elif ch.isalpha():
                    token += ch.lower()
                elif ch.isdigit():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "number"
                elif ch == '$':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "dollar"
                elif ch < ' ':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch in ['(', ')', '=', '^']:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch == '"':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "string"
                else:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "punct"
            elif state == "number":
                if ch.isspace():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ""
                    state = "none"
                elif ch.isdigit():
                    token += ch
                elif ch == '.':
                    token += ch
                    state = "float"
                elif ch.isalpha():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch.lower()
                    state = "alpha"
                elif ch == '$':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "dollar"
                elif ch < ' ':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch in ['(', ')', '=', '^']:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch == '"':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "string"
                else:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "punct"
            elif state == "float":
                if ch.isspace():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ""
                    state = "none"
                elif ch.isdigit():
                    token += ch
                elif ch == '.':
                    if token[-1] == '.':
                        result.append(PmacToken(token[:-1], self.debugFileName, self.debugLineNumber))
                        token = '..'
                        state = "punct"
                    else:
                        result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                        token = ch
                        state = "punct"
                elif ch.isalpha():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch.lower()
                    state = "alpha"
                elif ch == '$':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "dollar"
                elif ch < ' ':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch in ['(', ')', '=', '^']:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch == '"':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "string"
                else:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "punct"
            elif state == "punct":
                if ch.isspace():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ""
                    state = "none"
                elif ch.isdigit():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "number"
                elif ch.isalpha():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch.lower()
                    state = "alpha"
                elif ch == '$':
                    if token == "$" or token == "$$":
                        token += ch
                    else:
                        result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                        token = ch
                        state = "dollar"
                elif ch < ' ':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch in ['(', ')', '^', '-']:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch == '=':
                    if token == '!':
                        token += ch
                    else:
                        result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                        result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                        state = "none"
                        token = ""
                elif ch == '"':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "string"
                else:
                    token += ch
                    if token == '->':
                        result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                        state = 'none'
                        token = ''
            elif state == "hexnumber":
                if ch.isspace():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ""
                    state = "none"
                elif ch.isdigit():
                    token += ch
                elif ch >= 'a' and ch <= 'f':
                    token += ch
                elif ch >= 'A' and ch <= 'F':
                    token += ch.lower()
                elif ch.isalpha():
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch.lower()
                    state = "alpha"
                elif ch == '$':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "dollar"
                elif ch < ' ':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch in ['(', ')', '=', '^']:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    result.append(PmacToken(ch, self.debugFileName, self.debugLineNumber))
                    state = "none"
                    token = ''
                elif ch == '"':
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "string"
                else:
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ch
                    state = "punct"
            elif state == "string":
                if ch == '"':
                    token += ch
                    result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
                    token = ''
                    state = "none"
                else:
                    token += ch
        if len(token) > 0:
            result.append(PmacToken(token, self.debugFileName, self.debugLineNumber))
        return result

    def output(self, data):
        self.outputData += data

    def openTraceWindow(self):
        self.trace = True

    def closeTraceWindow(self):
        self.trace = False


