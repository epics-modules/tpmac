#include <iocsh.h>
#include "pmacAsynMotor.h"
#include "epicsExport.h"

extern "C" {

static const iocshArg pmacAsynMotorCreateArg0 = { "port",          iocshArgString};
static const iocshArg pmacAsynMotorCreateArg1 = { "address",       iocshArgInt};
static const iocshArg pmacAsynMotorCreateArg2 = { "card",          iocshArgInt};
static const iocshArg pmacAsynMotorCreateArg3 = { "Number of Axes",iocshArgInt};

static const iocshArg *const pmacAsynMotorCreateArgs[] = {
  &pmacAsynMotorCreateArg0,
  &pmacAsynMotorCreateArg1,
  &pmacAsynMotorCreateArg2,
  &pmacAsynMotorCreateArg3,
};
static const iocshFuncDef pmacAsynMotorCreateDef ={"pmacAsynMotorCreate",4,pmacAsynMotorCreateArgs};

static void pmacAsynMotorCreateCallFunc(const iocshArgBuf *args)
{
    pmacAsynMotorCreate( args[0].sval, args[1].ival, args[2].ival, args[3].ival );
}

void pmacAsynMotorRegister(void)
{
  iocshRegister(&pmacAsynMotorCreateDef, pmacAsynMotorCreateCallFunc);
}
epicsExportRegistrar(pmacAsynMotorRegister);

} // extern "C"

