#!/dls_sw/tools/bin/python2.4

# Test suite to use with pyUnit

from pkg_resources import require

require('cothread==2.3')

require('dls_autotestframework==2.13')
from dls_autotestframework import *

from pmaccases import *
import pyclbr

################################################
# Test suite for the motor support module.
    
class motorTestSuite(TestSuite):

   def loadCasePlugins(self):
      classes = pyclbr.readmodule("motorcases")
      for c in classes:
         if not (c.endswith("Base")):
            classobj = eval(c)
            if (issubclass(classobj, TestCase)):
               if not (classobj == TestCase):
                  classinstance = classobj(self)

   def createTests(self):
      # Define the targets for this test suite
      Target("pmac_model3", self, [
            BuildEntity('tpmac'),
            IocEntity('ioc', directory='iocs/pmacGeobrickExamplePortDriver', bootCmd='bin/linux-x86/stpmacTest.boot'),
            EpicsDbEntity('db', directory='iocs/pmacGeobrickExamplePortDriver', fileName="db/pmacTest.db")])
      

      motorCaseReadInit(self)
      motorCaseMoveSequence1(self) #do a few moves before the autosave test
      motorCaseAutosaveRestoreCheck(self)
      motorCaseMoveSequence1(self)
      motorCaseMoveSequence2(self)
      motorCaseMoveSequence3(self)
      motorCaseMoveSequence4(self)
      motorCaseMoveSequence5(self)
      motorCaseMoveSequence6(self)
      motorCaseMoveCheckStatus(self)
      motorCaseCheckOffset(self)
      motorCaseSetPosition(self)
      motorCaseDeferredMoves(self)

#      self.loadCasePlugins()
      
       


################################################
# Main entry point

if __name__ == "__main__":
      # Create and run the test sequence
      motorTestSuite()
