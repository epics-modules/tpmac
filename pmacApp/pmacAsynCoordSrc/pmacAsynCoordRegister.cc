#include <iocsh.h>
#include "pmacAsynCoord.h"
#include "epicsExport.h"

extern "C" {

/* int pmacSetDefaultCoordSteps(double defaultSteps) */
static const iocshArg pmacSetDefaultCoordStepsArg0 = {"Default Steps", iocshArgDouble};
static const iocshArg * const pmacSetDefaultCoordStepsArgs[1] = {&pmacSetDefaultCoordStepsArg0};
static const iocshFuncDef pmacSetDefaultCoordStepsDef = {"pmacSetDefaultCoordSteps", 1, pmacSetDefaultCoordStepsArgs};
static void pmacSetDefaultCoordStepsCallFunc(const iocshArgBuf *args)
{
  pmacSetDefaultCoordSteps(args[0].dval);
}

/* int pmacSetCoordStepsPerUnit(int csRef, int axis, double stepsPerUnit) */
static const iocshArg pmacSetCoordStepsPerUnitArg0 = {"CS Ref", iocshArgInt};
static const iocshArg pmacSetCoordStepsPerUnitArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacSetCoordStepsPerUnitArg2 = {"Steps per unit", iocshArgDouble};
  static const iocshArg * const pmacSetCoordStepsPerUnitArgs[3] = {&pmacSetCoordStepsPerUnitArg0, &pmacSetCoordStepsPerUnitArg1, &pmacSetCoordStepsPerUnitArg2};
static const iocshFuncDef pmacSetCoordStepsPerUnitDef = {"pmacSetCoordStepsPerUnit", 3, pmacSetCoordStepsPerUnitArgs};
static void pmacSetCoordStepsPerUnitCallFunc(const iocshArgBuf *args)
{
  pmacSetCoordStepsPerUnit(args[0].ival, args[1].ival, args[2].dval);
}

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
  iocshRegister(&pmacSetCoordStepsPerUnitDef, pmacSetCoordStepsPerUnitCallFunc);
  iocshRegister(&pmacSetDefaultCoordStepsDef, pmacSetDefaultCoordStepsCallFunc);
}
epicsExportRegistrar(pmacAsynCoordRegister);

} // extern "C"

