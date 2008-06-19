#include <iocsh.h>
#include "pmacAsynMotor.h"
#include "epicsExport.h"

extern "C" {

/* int pmacSetMovingPollPeriod(int card, int movingPollPeriod) */
static const iocshArg pmacSetMovingPollPeriodArg0 = {"Card number", iocshArgInt};
static const iocshArg pmacSetMovingPollPeriodArg1 = {"Moving poll period", iocshArgInt};
static const iocshArg * const pmacSetMovingPollPeriodArgs[2] = {&pmacSetMovingPollPeriodArg0, &pmacSetMovingPollPeriodArg1};
static const iocshFuncDef pmacSetMovingPollPeriodDef = {"pmacSetMovingPollPeriod", 2, pmacSetMovingPollPeriodArgs};
static void pmacSetMovingPollPeriodCallFunc(const iocshArgBuf *args)
{
    pmacSetMovingPollPeriod(args[0].ival, args[1].ival);
}


/* int pmacSetIdlePollPeriod(int card, int idlePollPeriod) */
static const iocshArg pmacSetIdlePollPeriodArg0 = {"Card number", iocshArgInt};
static const iocshArg pmacSetIdlePollPeriodArg1 = {"Idle poll period", iocshArgInt};
static const iocshArg * const pmacSetIdlePollPeriodArgs[2] = {&pmacSetIdlePollPeriodArg0, &pmacSetIdlePollPeriodArg1};
static const iocshFuncDef pmacSetIdlePollPeriodDef = {"pmacSetIdlePollPeriod", 2, pmacSetIdlePollPeriodArgs};
static void pmacSetIdlePollPeriodCallFunc(const iocshArgBuf *args)
{
    pmacSetIdlePollPeriod(args[0].ival, args[1].ival);
}


/* int pmacAsynMotorCreate(port, address, card, Number of axes).*/
static const iocshArg pmacAsynMotorCreateArg0 = { "port",          iocshArgString};
static const iocshArg pmacAsynMotorCreateArg1 = { "address",       iocshArgInt};
static const iocshArg pmacAsynMotorCreateArg2 = { "card",          iocshArgInt};
static const iocshArg pmacAsynMotorCreateArg3 = { "Number of Axes",iocshArgInt};

static const iocshArg *const pmacAsynMotorCreateArgs[] = {
  &pmacAsynMotorCreateArg0,
  &pmacAsynMotorCreateArg1,
  &pmacAsynMotorCreateArg2,
  &pmacAsynMotorCreateArg3
};
static const iocshFuncDef pmacAsynMotorCreateDef ={"pmacAsynMotorCreate",4,pmacAsynMotorCreateArgs};

static void pmacAsynMotorCreateCallFunc(const iocshArgBuf *args)
{
    pmacAsynMotorCreate( args[0].sval, args[1].ival, args[2].ival, args[3].ival );
}


/*Register functions for IOC shell.*/
void pmacAsynMotorRegister(void)
{
  iocshRegister(&pmacSetMovingPollPeriodDef, pmacSetMovingPollPeriodCallFunc);
  iocshRegister(&pmacSetIdlePollPeriodDef, pmacSetIdlePollPeriodCallFunc);
  iocshRegister(&pmacAsynMotorCreateDef, pmacAsynMotorCreateCallFunc);
}
epicsExportRegistrar(pmacAsynMotorRegister);

} // extern "C"

