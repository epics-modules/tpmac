#include <iocsh.h>
#include "pmacAsynMotor.h"
#include "epicsExport.h"

extern "C" {

static const iocshArg sendBufferArg0 = {"Port name", iocshArgString};
static const iocshArg sendBufferArg1 = {"Address", iocshArgInt};
static const iocshArg sendBufferArg2 = {"Command", iocshArgString};
static const iocshArg * const sendBufferArgs[] =  {&sendBufferArg0,
                                                              &sendBufferArg1,
                                                              &sendBufferArg2};
static const iocshFuncDef sendBufferDef = {"sendBuffer", 3, sendBufferArgs};
static void sendBufferCallFunc(const iocshArgBuf *args)
{
    sendBuffer(args[0].sval, args[1].ival, args[2].sval);
}


/* int pmacDisableLimitsCheck(int card, int axis, int allAxes) */
static const iocshArg pmacDisableLimitsCheckArg0 = {"Card number", iocshArgInt};
static const iocshArg pmacDisableLimitsCheckArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacDisableLimitsCheckArg2 = {"Do all axes", iocshArgInt};
static const iocshArg * const pmacDisableLimitsCheckArgs[] = {&pmacDisableLimitsCheckArg0,
								  &pmacDisableLimitsCheckArg1, 
								  &pmacDisableLimitsCheckArg2};
static const iocshFuncDef pmacDisableLimitsCheckDef = {"pmacDisableLimitsCheck", 3, pmacDisableLimitsCheckArgs};
static void pmacDisableLimitsCheckCallFunc(const iocshArgBuf *args)
{
  pmacDisableLimitsCheck(args[0].ival, args[1].ival, args[2].ival );
}



/* int pmacSetOpenLoopEncoderAxis(int card, int axis, int encoder_axis ) */
static const iocshArg pmacSetOpenLoopEncoderAxisArg0 = {"Card number", iocshArgInt};
static const iocshArg pmacSetOpenLoopEncoderAxisArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacSetOpenLoopEncoderAxisArg2 = {"Encoder readback axis", iocshArgInt};
static const iocshArg * const pmacSetOpenLoopEncoderAxisArgs[] = {&pmacSetOpenLoopEncoderAxisArg0,
								  &pmacSetOpenLoopEncoderAxisArg1, 
								  &pmacSetOpenLoopEncoderAxisArg2};
static const iocshFuncDef pmacSetOpenLoopEncoderAxisDef = {"pmacSetOpenLoopEncoderAxis", 3, pmacSetOpenLoopEncoderAxisArgs};
static void pmacSetOpenLoopEncoderAxisCallFunc(const iocshArgBuf *args)
{
  pmacSetOpenLoopEncoderAxis(args[0].ival, args[1].ival, args[2].ival );
}

/* int pmacSetAxisScale(int card, int axis, int scale ) */
static const iocshArg pmacSetAxisScaleArg0 = {"Card number", iocshArgInt};
static const iocshArg pmacSetAxisScaleArg1 = {"Axis number", iocshArgInt};
static const iocshArg pmacSetAxisScaleArg2 = {"Scale factor", iocshArgInt};
static const iocshArg * const pmacSetAxisScaleArgs[] = {&pmacSetAxisScaleArg0, &pmacSetAxisScaleArg1, &pmacSetAxisScaleArg2};
static const iocshFuncDef pmacSetAxisScaleDef = {"pmacSetAxisScale", 3, pmacSetAxisScaleArgs};
static void pmacSetAxisScaleCallFunc(const iocshArgBuf *args)
{
    pmacSetAxisScale(args[0].ival, args[1].ival, args[2].ival);
}

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
  iocshRegister(&pmacSetAxisScaleDef, pmacSetAxisScaleCallFunc);
  iocshRegister(&pmacSetOpenLoopEncoderAxisDef, pmacSetOpenLoopEncoderAxisCallFunc);
  iocshRegister(&pmacDisableLimitsCheckDef, pmacDisableLimitsCheckCallFunc);
  iocshRegister(&pmacAsynMotorCreateDef, pmacAsynMotorCreateCallFunc);
  iocshRegister(&sendBufferDef, sendBufferCallFunc);
}
epicsExportRegistrar(pmacAsynMotorRegister);

} // extern "C"

