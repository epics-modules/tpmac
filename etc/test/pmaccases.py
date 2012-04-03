from pkg_resources import require
require('dls_autotestframework')
from dls_autotestframework import TestCase


import re
import cothread

class motorCaseBase(TestCase):
   """
   Base class for all motor test cases.
   This is where the base PV is hardcoded to be 'mp49t:sim'
   """
   
   def __init__(self, A):
      TestCase.__init__(self, A)
      self.__pv1 = "mp49t:geo:"
      self.__motors = ["A","B"]
      self.__pvDefer = "mp49t:geo:defer"
      self.__diag = 1
      self.__timeout = 10000
      
   def getPVBase(self):
      return self.__pv1

   def getMotors(self):
      return self.__motors

   def getDiag(self):
      return self.__diag

   def getTimeout(self):
      return self.__timeout

   def getPVDefer(self):
      return self.__pvDefer


   def doMoveSequence(self, distance, moves, axes=6):
      """
      Do a series of moves on each motor record. The number of moves
      and the distance are arguments to the function. The number of axes
      is also specified (max 6).

      The moves are triggered by writing to VAL.
      The start and end positions are checked (to be within RDBD)
      """

      move = 0.0
      axis_count = 0

      for motor in self.getMotors():

         axis_count = axis_count + 1
         if axis_count <= axes:

            self.diagnostic("Moving motor record " + self.getPVBase() + motor, self.getDiag());

            pv_rbv = self.getPVBase() + motor + ".RBV"
            pv_rdbd = self.getPVBase() + motor + ".RDBD"
            pv_val = self.getPVBase() + motor + ".VAL"

            rdbd = self.getPv(pv_rdbd)
            self.diagnostic(pv_rdbd + ": " + str(rdbd),  self.getDiag())
            
            for i in range(moves):
               
               move += distance
               
               self.diagnostic("Moving to " + str(move), self.getDiag())
               
               #Read RBV
               rbv = self.getPv(pv_rbv)
               self.diagnostic(pv_rbv + ": " + str(self.getPv(pv_rbv)), self.getDiag()+1)
               #Do a move
               self.putPv(pv_val, move, wait=True, timeout=self.getTimeout())
               #Read back RBV
               self.diagnostic(pv_rbv + ": " + str(self.getPv(pv_rbv)), self.getDiag()+1)

               #Verify RBV is within range
               self.verifyPvInRange(pv_rbv, move-rdbd, move+rdbd)

            #Now move back to zero
            move = 0.0
            self.putPv(pv_val, move, wait=True, timeout=self.getTimeout())
            self.verifyPvInRange(pv_rbv, move-rdbd, move+rdbd)


         

class motorCaseReadInit(motorCaseBase):
   """
   Test case to read motor record initial startup.
   It reads DMOV, MOVN, DVAL, OFF and MSTA.
   DVAL and OFF should have been set by autosave, so I want to move
   all to zero to start from a well defined state.
   """
   
   def runTest(self):

      init_dmov = 1
      init_movn = 0
      init_set = 0
      init_msta = 2 #Only check the 'done' bit.

      for motor in self.getMotors():

         self.diagnostic("Reading motor record " + self.getPVBase() + motor + " initial state", self.getDiag())
      
         pv_dmov = self.getPVBase() + motor + ".DMOV"
         pv_movn = self.getPVBase() + motor + ".MOVN"
         pv_set = self.getPVBase() + motor + ".SET"
         pv_val = self.getPVBase() + motor + ".VAL"
         pv_off = self.getPVBase() + motor + ".OFF"
         pv_msta = self.getPVBase() + motor + ".MSTA"

         self.diagnostic(pv_dmov + ": " + str(self.getPv(pv_dmov)), self.getDiag())
         self.diagnostic(pv_movn + ": " + str(self.getPv(pv_movn)), self.getDiag())
         self.diagnostic(pv_set + ": " + str(self.getPv(pv_set)), self.getDiag())
         self.diagnostic(pv_msta + ": " + str(self.getPv(pv_msta)), self.getDiag())
         
         self.verify(init_dmov, self.getPv(pv_dmov))
         self.verify(init_movn, self.getPv(pv_movn))
         self.verify(init_set, self.getPv(pv_set))

         self.diagnostic("Check that MSTA is not zero and at least in position is set...", self.getDiag())
         self.verify(init_msta, (0x2 & int(self.getPv(pv_msta))))

         #Move to zero, and set offsets to zero.
         self.putPv(pv_off, 0.0, wait=True, timeout=self.getTimeout())
         self.putPv(pv_val, 0.0, wait=True, timeout=self.getTimeout())
         

class motorCaseAutosaveRestoreCheck(motorCaseBase):
   """
   Class to check autosave restore. This means bringing down the IOC
   and restarting it, checking the restored positions. Do this twice to
   check the case when positions were not zero (which should not cause
   positions to be written on startup).
   """

   def runTest(self):

      offset = 1.0
      pos = 5.0

      for motor in self.getMotors():

         pv_val = self.getPVBase() + motor + ".VAL"
         pv_off = self.getPVBase() + motor + ".OFF"

         #First to a none zero position
         self.putPv(pv_off, offset, wait=True, timeout=self.getTimeout())
         self.putPv(pv_val, pos, wait=True, timeout=self.getTimeout())

      #Wait 1 minute for autosave to save the new numbers
      cothread.Sleep(60)

      #Now stop the IOC
      ioc = self.entity("ioc")
      self.diagnostic("Stopping IOC...", self.getDiag())
      ioc.stop()
      cothread.Sleep(10)
      #Start it again
      ioc.start()
      cothread.Sleep(10)

      for motor in self.getMotors():

         pv_val = self.getPVBase() + motor + ".VAL"
         pv_off = self.getPVBase() + motor + ".OFF"
         pv_rbv = self.getPVBase() + motor + ".RBV"
         pv_rdbd = self.getPVBase() + motor + ".RDBD"

         rdbd = self.getPv(pv_rdbd)

         #Verify RBV and OFF
         self.verifyPvInRange(pv_rbv, pos-rdbd, pos+rdbd)
         self.verifyPvInRange(pv_val, pos-rdbd, pos+rdbd)
         self.verify(offset, self.getPv(pv_off))

         #Now try from zero
         #First to a none zero position
         self.putPv(pv_off, 0, wait=True, timeout=self.getTimeout())
         self.putPv(pv_val, 0, wait=True, timeout=self.getTimeout())

      #Wait 1 minute for autosave to save the new numbers
      cothread.Sleep(60)

      #Now stop the IOC
      ioc = self.entity("ioc")
      self.diagnostic("Stopping IOC...", self.getDiag())
      ioc.stop()
      cothread.Sleep(10)
      #Start it again
      ioc.start()
      cothread.Sleep(10)
      
      for motor in self.getMotors():

         pv_val = self.getPVBase() + motor + ".VAL"
         pv_off = self.getPVBase() + motor + ".OFF"
         pv_rbv = self.getPVBase() + motor + ".RBV"
         pv_rdbd = self.getPVBase() + motor + ".RDBD"

         rdbd = self.getPv(pv_rdbd)

         #Verify RBV, VAL and OFF
         self.verifyPvInRange(pv_rbv, 0-rdbd, 0+rdbd)
         self.verifyPvInRange(pv_val, 0-rdbd, 0+rdbd)
         self.verify(0, self.getPv(pv_off))
         
         
         
   


class motorCaseMoveSequence1(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [0.001, 0.002, 0.003]
      moves = 10

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveSequence2(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [0.01, 0.02]
      moves = 10

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveSequence3(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [0.1, 0.2]
      moves = 10

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveSequence4(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [1.0]
      moves = 5

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveSequence5(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [10.0]
      moves = 5

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveSequence6(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [100.0]
      moves = 5

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveSequence7(motorCaseBase):
   """
   Class to do a series of moves on each motor record.
   """

   def runTest(self):

      distances = [1.230, 1.567, 4.010, 11.999]
      moves = 5

      for distance in distances:
         self.diagnostic("Moving distances of " + str(distance), self.getDiag())
         self.doMoveSequence(distance, moves)


class motorCaseMoveCheckStatus(motorCaseBase):
   """
   Class to check MSTA during a move
   """

   def runTest(self):

      for motor in self.getMotors():

         self.diagnostic("motorCaseMoveCheckStatus for motor " + str(motor) + "...", self.getDiag())

         pv_val = self.getPVBase() + motor + ".VAL"
         pv_dmov = self.getPVBase() + motor + ".DMOV"
         pv_msta = self.getPVBase() + motor + ".MSTA"
         pv_dir = self.getPVBase() + motor + ".DIR"
         pv_stop = self.getPVBase() + motor + ".STOP"

         #First move motor to zero
         self.diagnostic("Move to zero.", self.getDiag())
         self.putPv(pv_val, 0.0, wait=True, timeout=self.getTimeout())

         #Check MSTA Done is set
         self.verify(0x2, (int(self.getPv(pv_msta)) & 0x2))

         #Not do a move, with no callback
         self.diagnostic("Move to 100.", self.getDiag())
         self.putPv(pv_val, 100.0, wait=False)

         #Use cothread sleep instead of time.sleep(), which blocks the cothread library (stopping the previous caput working).
         cothread.Sleep(2.0)

         #Read DIR field
         direction = self.getPv(pv_dir)
         if (direction == 0):
            #Check MSTA Done is not set, and moving flag is on and direction positive is on.
            self.verify(0x401, (int(self.getPv(pv_msta)) & 0x401))
         else:
            #Check MSTA Done is not set, and moving flag is on.
            self.verify(0x400, (int(self.getPv(pv_msta)) & 0x400))

         #Now stop it, check status, and move back to zero
         self.diagnostic("Stopping.", self.getDiag())
         self.putPv(pv_stop, 1, wait=True, timeout=self.getTimeout())

         self.diagnostic("Move completed. Do final MSTA checks.", self.getDiag())
         #Check MSTA Done is set
         self.verify(0x2, (int(self.getPv(pv_msta)) & 0x2))
         #Check MSTA moving is not set
         self.verify(0x0, (int(self.getPv(pv_msta)) & 0x400))

         #Move back to zero
         self.diagnostic("Move back to zero.", self.getDiag())
         self.putPv(pv_val, 0.0, wait=True, timeout=self.getTimeout())




class motorCaseCheckOffset(motorCaseBase):
   """
   Class to check the use of offset. It checks
   that soft limits and user coordinates are
   offset correctly when setting an offset.
   """

   def runTest(self):

      for motor in self.getMotors():

         self.diagnostic("motorCaseCheckOffset for motor " + str(motor) + "...", self.getDiag())

         pv_rbv = self.getPVBase() + motor + ".RBV"
         pv_off = self.getPVBase() + motor + ".OFF"
         pv_val = self.getPVBase() + motor + ".VAL"
         pv_hlm = self.getPVBase() + motor + ".HLM"
         pv_llm = self.getPVBase() + motor + ".LLM"
         pv_rdbd = self.getPVBase() + motor + ".RDBD"

         #Read current soft limits, offset and val
         start_off = self.getPv(pv_off)
         start_val = self.getPv(pv_val)
         start_rbv = self.getPv(pv_rbv)
         start_hlm = self.getPv(pv_hlm)
         start_llm = self.getPv(pv_llm)
         rdbd = self.getPv(pv_rdbd)

         self.diagnostic("Increasing offset by 1.", self.getDiag())

         #Increase offset by 1
         self.putPv(pv_off, start_off+1, wait=True, timeout=self.getTimeout())

         self.diagnostic("Now check the offset fields.", self.getDiag())

         #Now check soft limits, val and rbv
         self.verify(start_off+1, self.getPv(pv_off))
         self.verify(start_val+1, self.getPv(pv_val))
         self.verify(start_hlm+1, self.getPv(pv_hlm))
         self.verify(start_llm+1, self.getPv(pv_llm))

         self.verifyPvInRange(pv_val, start_val+1-rdbd, start_val+1+rdbd)
         self.verifyPvInRange(pv_rbv, start_rbv+1-rdbd, start_rbv+1+rdbd)

         self.diagnostic("Reset original offset.", self.getDiag())

         #Set back old offset
         self.putPv(pv_off, start_off, wait=True, timeout=self.getTimeout())

         

class motorCaseSetPosition(motorCaseBase):
   """
   Class to attempt a set position. The position before and
   after are read and verfied, along with the motor record state.
   """

   def runTest(self):

      for motor in self.getMotors():

         pv_drbv = self.getPVBase() + motor + ".DRBV"
         pv_off = self.getPVBase() + motor + ".OFF"
         pv_dval = self.getPVBase() + motor + ".DVAL"
         pv_val = self.getPVBase() + motor + ".VAL"
         pv_set = self.getPVBase() + motor + ".SET"
         pv_rdbd = self.getPVBase() + motor + ".RDBD"
         pv_dmov = self.getPVBase() + motor + ".DMOV"

         #For the first test, set the offset to zero.
         self.putPv(pv_off, 0, wait=True, timeout=self.getTimeout())

         self.diagnostic("Setting position with a zero offset...")
         
         start_position = self.getPv(pv_drbv)
         offset = self.getPv(pv_off)
         rdbd = self.getPv(pv_rdbd)
         self.diagnostic("Motor " + str(motor) + " is at DRBV: " + str(start_position))
         self.diagnostic("Motor " + str(motor) + " has offset OFF: " + str(offset))
         
         #Do a set position to start_position + 1.0
         end_position = start_position + 1.0
         self.putPv(pv_set, 1, wait=True, timeout=self.getTimeout())
         self.putPv(pv_dval, end_position, wait=True, timeout=self.getTimeout())
         self.putPv(pv_off, offset, wait=True, timeout=self.getTimeout())
         self.putPv(pv_set, 0, wait=True, timeout=self.getTimeout())

         self.verifyPvInRange(pv_drbv, end_position-rdbd, end_position+rdbd)
         self.verifyPvInRange(pv_dval, end_position-rdbd, end_position+rdbd)
         self.verifyPvInRange(pv_val, end_position+offset-rdbd, end_position+offset+rdbd)
         self.verify(offset, self.getPv(pv_off))
         self.verify(1, self.getPv(pv_dmov))
         self.verify(0, self.getPv(pv_set))

               
         #Now repeat this with a non-zero offset
         self.putPv(pv_off, 1.1, wait=True, timeout=self.getTimeout())

         self.diagnostic("Setting position with a offset of 1.1...")

         start_position = self.getPv(pv_drbv)
         offset = self.getPv(pv_off)
         rdbd = self.getPv(pv_rdbd)
         self.diagnostic("Motor " + str(motor) + " is at DRBV: " + str(start_position))
         self.diagnostic("Motor " + str(motor) + " has offset OFF: " + str(offset))
         
         #Do a set position to start_position + 1.0
         end_position = start_position + 1.0
         self.putPv(pv_set, 1, wait=True, timeout=self.getTimeout())
         self.putPv(pv_dval, end_position, wait=True, timeout=self.getTimeout())
         self.putPv(pv_off, offset, wait=True, timeout=self.getTimeout())
         self.putPv(pv_set, 0, wait=True, timeout=self.getTimeout())

         self.verifyPvInRange(pv_drbv, end_position-rdbd, end_position+rdbd)
         self.verifyPvInRange(pv_dval, end_position-rdbd, end_position+rdbd)
         self.verifyPvInRange(pv_val, end_position+offset-rdbd, end_position+offset+rdbd)
         self.verify(offset, self.getPv(pv_off))
         self.verify(1, self.getPv(pv_dmov))
         self.verify(0, self.getPv(pv_set))

         #Move to zero, and set offsets to zero.
         self.putPv(pv_off, 0.0, wait=True, timeout=self.getTimeout())
         self.putPv(pv_val, 0.0, wait=True, timeout=self.getTimeout()) 

         


class motorCaseDeferredMoves(motorCaseBase):
   """
   Class to test deferred moves operation.

   Currently the cothread library does not support user supplied callback functions.
   When using ca_put_callback the caput function simply blocks. In a future release it
   will support it, and we tested it (see the commented out sections below).

   But for now, I am simply doing the deferred move in a single thread, and polling the
   DMOV fields to check that all the motors have finished. The ca_put to the motor
   record is without a callback.

   Using multiple threads here won't work, because in a thread we have no way of knowing
   when (or in which order) the ca_put_callback happens. So deferred moves logic only
   works in a single thread context.

   Using camonitor (on the DMOV) fields might be better way to do it, but I have not tried this.
   """

#Make use of a callback function when we have a cothread caput that supports user callbacks
#   def mycallback(self, result):
#      print "Callback"
#      print result

#Don't need this function when we have support for user defined callbacks in caput
   def pollDMOVs(self):
      for motor in self.getMotors():
         dmov = 0
         pv_dmov = self.getPVBase() + motor + ".DMOV"
         while (dmov==0):
            cothread.Sleep(0.1)
            dmov = self.getPv(pv_dmov)
            if dmov:
               self.diagnostic("Motor " + str(motor) + " has finished.")

      self.diagnostic("All motors finished.")


   def runTest(self):

      pv_defer = self.getPVDefer()
      self.diagnostic("Testing deferred moves...")
      self.diagnostic("Defer moves PV: " + str(pv_defer))

      #Turn off deferred moves (need to send a stop to all axes to cancel deferred moves for those axes)
      self.putPv(pv_defer, 0, wait=True, timeout=self.getTimeout())
      for motor in self.getMotors():
         pv_stop = self.getPVBase() + motor + ".STOP"
         self.putPv(pv_stop, 1, wait=True, timeout=self.getTimeout())

      #Move all axes to zero
      for motor in self.getMotors():
         pv_val = self.getPVBase() + motor + ".VAL"
         self.putPv(pv_val, 0.0, wait=True, timeout=self.getTimeout())

      #Set the velocity lower (so we can see the deferred moves in action)
      pv_velo = self.getPVBase() + "A.VELO"
      velo = self.getPv(pv_velo)
      for motor in self.getMotors():
         pv_velo = self.getPVBase() + motor + ".VELO"
         self.putPv(pv_velo, velo/2, wait=True, timeout=self.getTimeout())

      #Turn on deferred moves
      self.diagnostic("Turning on deferred moves...")
      self.putPv(pv_defer, 1, wait=True, timeout=self.getTimeout())

      #Multi threaded won't work, due to not being able to know when a
      #thread has called ca_put_callback or not. So it needs to be single
      #threaded, with the use of callbacks in the caput. Now, cothread
      #does not support callback in caput (yet), so we need to use camonitor
      #instead (and monitor DMOV) or poll DMOV manually.
      #The GDA caput does support callbacks, so that's what they use.

#Do this when cothread is released that supports use of a callback function     
#      for motor in self.getMotors():
#         pv_val = self.getPVBase() + motor + ".VAL"
#         self.putPv(pv_val, 2.0, callback=self.mycallback)
#         cothread.catools.caput(pv_val, 2.0, wait=False, callback=self.mycallback)

      for motor in self.getMotors():
         pv_val = self.getPVBase() + motor + ".VAL"
         self.putPv(pv_val, 5.0, wait=False, timeout=self.getTimeout())

      #Turn off deferred moves
      self.diagnostic("Turning off deferred moves...")
      self.putPv(pv_defer, 0, wait=True, timeout=self.getTimeout())

      #Now poll DMOVs
      self.pollDMOVs()

      #Reset the velocity
      for motor in self.getMotors():
         pv_velo = self.getPVBase() + motor + ".VELO"
         self.putPv(pv_velo, velo, wait=True, timeout=self.getTimeout())

      #Now do it again, faster this time

      #Turn on deferred moves
      self.diagnostic("Turning on deferred moves...")
      self.putPv(pv_defer, 1, wait=True, timeout=self.getTimeout())
      
      for motor in self.getMotors():
         pv_val = self.getPVBase() + motor + ".VAL"
         self.putPv(pv_val, 0.0, wait=False, timeout=self.getTimeout())

      #Turn off deferred moves
      self.diagnostic("Turning off deferred moves...")
      self.putPv(pv_defer, 0, wait=True, timeout=self.getTimeout())

      #Now poll DMOVs
      self.pollDMOVs()

      #Now test if we can cancle deferred moves

      #Turn on deferred moves
      self.diagnostic("Turning on deferred moves...")
      self.putPv(pv_defer, 1, wait=True, timeout=self.getTimeout())

      for motor in self.getMotors():
         pv_val = self.getPVBase() + motor + ".VAL"
         self.putPv(pv_val, 5.0, wait=False, timeout=self.getTimeout())

      #Turn off deferred moves (need to send a stop to all axes to cancel deferred moves for those axes)
      self.diagnostic("Cancelling deferred moves...")
      for motor in self.getMotors():
         pv_stop = self.getPVBase() + motor + ".STOP"
         self.putPv(pv_stop, 1, wait=True, timeout=self.getTimeout())
      self.putPv(pv_defer, 0, wait=True, timeout=self.getTimeout())

      #Now test if we can move axes normally.
      self.diagnostic("Testing a normal move...")
      for motor in self.getMotors():
         pv_val = self.getPVBase() + motor + ".VAL"
         self.putPv(pv_val, 7.0, wait=True, timeout=self.getTimeout())

      #Finally, test a deferred move back to zero

      #Turn on deferred moves
      self.diagnostic("Turning on deferred moves...")
      self.putPv(pv_defer, 1, wait=True, timeout=self.getTimeout())
      
      for motor in self.getMotors():
         pv_val = self.getPVBase() + motor + ".VAL"
         self.putPv(pv_val, 0.0, wait=False, timeout=self.getTimeout())

      #Turn off deferred moves
      self.diagnostic("Turning off deferred moves...")
      self.putPv(pv_defer, 0, wait=True, timeout=self.getTimeout())

      #Now poll DMOVs
      self.pollDMOVs()
