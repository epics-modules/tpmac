#include <iocsh.h>
#include "pmacAsynCoord.h"
#include "epicsExport.h"

extern "C" {

/* int pmacSetCoordMovingPollPeriod(int card, int movingPollPeriod) */
static const iocshArg pmacSetCoordMovingPollPeriodArg0 = {"CS number", iocshArgInt};
static const iocshArg pmacSetCoordMovingPollPeriodArg1 = {"Moving poll period", iocshArgInt};
static const iocshArg * const pmacSetCoordMovingPollPeriodArgs[2] = {&pmacSetCoordMovingPollPeriodArg0, &pmacSetCoordMovingPollPeriodArg1};
static const iocshFuncDef pmacSetCoordMovingPollPeriodDef = {"pmacSetCoordMovingPollPeriod", 2, pmacSetCoordMovingPollPeriodArgs};
static void pmacSetCoordMovingPollPeriodCallFunc(const iocshArgBuf *args)
{
    pmacSetCoordMovingPollPeriod(args[0].ival, args[1].ival);
}


/* int pmacSetCoordIdlePollPeriod(int card, int idlePollPeriod) */
static const iocshArg pmacSetCoordIdlePollPeriodArg0 = {"CS number", iocshArgInt};
static const iocshArg pmacSetCoordIdlePollPeriodArg1 = {"Idle poll period", iocshArgInt};
static const iocshArg * const pmacSetCoordIdlePollPeriodArgs[2] = {&pmacSetCoordIdlePollPeriodArg0, &pmacSetCoordIdlePollPeriodArg1};
static const iocshFuncDef pmacSetCoordIdlePollPeriodDef = {"pmacSetCoordIdlePollPeriod", 2, pmacSetCoordIdlePollPeriodArgs};
static void pmacSetCoordIdlePollPeriodCallFunc(const iocshArgBuf *args)
{
    pmacSetCoordIdlePollPeriod(args[0].ival, args[1].ival);
}


static const iocshArg pmacAsynCoordCreateArg0 = { "port",     iocshArgString};
static const iocshArg pmacAsynCoordCreateArg1 = { "address",  iocshArgInt};
static const iocshArg pmacAsynCoordCreateArg2 = { "C.S. no",  iocshArgInt};
static const iocshArg pmacAsynCoordCreateArg3 = { "reference",iocshArgInt};
static const iocshArg pmacAsynCoordCreateArg4 = { "program",  iocshArgInt};

static const iocshArg *const pmacAsynCoordCreateArgs[] = {
  &pmacAsynCoordCreateArg0,
  &pmacAsynCoordCreateArg1,
  &pmacAsynCoordCreateArg2,
  &pmacAsynCoordCreateArg3,
  &pmacAsynCoordCreateArg4
};
static const iocshFuncDef pmacAsynCoordCreateDef ={"pmacAsynCoordCreate",5,pmacAsynCoordCreateArgs};

static void pmacAsynCoordCreateCallFunc(const iocshArgBuf *args)
{
    pmacAsynCoordCreate( args[0].sval, args[1].ival, args[2].ival, args[3].ival, args[4].ival );
}

void pmacAsynCoordRegister(void)
{
  iocshRegister(&pmacAsynCoordCreateDef, pmacAsynCoordCreateCallFunc);
  iocshRegister(&pmacSetCoordMovingPollPeriodDef, pmacSetCoordMovingPollPeriodCallFunc);
  iocshRegister(&pmacSetCoordIdlePollPeriodDef, pmacSetCoordIdlePollPeriodCallFunc);
}
epicsExportRegistrar(pmacAsynCoordRegister);

} // extern "C"

