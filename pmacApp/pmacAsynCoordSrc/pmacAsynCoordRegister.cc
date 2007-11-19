#include <iocsh.h>
#include "pmacAsynCoord.h"
#include "epicsExport.h"

extern "C" {

static const iocshArg pmacAsynCoordCreateArg0 = { "port",    iocshArgString};
static const iocshArg pmacAsynCoordCreateArg1 = { "address", iocshArgInt};
static const iocshArg pmacAsynCoordCreateArg2 = { "card",    iocshArgInt};
static const iocshArg pmacAsynCoordCreateArg3 = { "C.S. no", iocshArgInt};

static const iocshArg *const pmacAsynCoordCreateArgs[] = {
  &pmacAsynCoordCreateArg0,
  &pmacAsynCoordCreateArg1,
  &pmacAsynCoordCreateArg2,
  &pmacAsynCoordCreateArg3
};
static const iocshFuncDef pmacAsynCoordCreateDef ={"pmacAsynCoordCreate",4,pmacAsynCoordCreateArgs};

static void pmacAsynCoordCreateCallFunc(const iocshArgBuf *args)
{
    pmacAsynCoordCreate( args[0].sval, args[1].ival, args[2].ival, args[3].ival );
}

void pmacAsynCoordRegister(void)
{
  iocshRegister(&pmacAsynCoordCreateDef, pmacAsynCoordCreateCallFunc);
}
epicsExportRegistrar(pmacAsynCoordRegister);

} // extern "C"

