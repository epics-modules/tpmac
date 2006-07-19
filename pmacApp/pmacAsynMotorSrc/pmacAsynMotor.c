#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "paramLib.h"

#include "epicsFindSymbol.h"
#include "epicsTime.h"
#include "epicsThread.h"
#include "epicsMutex.h"
#include "ellLib.h"

#include "drvSup.h"
#include "epicsExport.h"
#define DEFINE_MOTOR_PROTOTYPES 1
#include "motor_interface.h"
#include "asynDriver.h"
#include "asynOctetSyncIO.h"

motorAxisDrvSET_t pmacAsynMotor =
  {
    14,
    motorAxisReport,            /**< Standard EPICS driver report function (optional) */
    motorAxisInit,              /**< Standard EPICS dirver initialisation function (optional) */
    motorAxisSetLog,            /**< Defines an external logging function (optional) */
    motorAxisOpen,              /**< Driver open function */
    motorAxisClose,             /**< Driver close function */
    motorAxisSetCallback,       /**< Provides a callback function the driver can call when the status updates */
    motorAxisSetDouble,         /**< Pointer to function to set a double value */
    motorAxisSetInteger,        /**< Pointer to function to set an integer value */
    motorAxisGetDouble,         /**< Pointer to function to get a double value */
    motorAxisGetInteger,        /**< Pointer to function to get an integer value */
    motorAxisHome,              /**< Pointer to function to execute a more to reference or home */
    motorAxisMove,              /**< Pointer to function to execute a position move */
    motorAxisVelocityMove,      /**< Pointer to function to execute a velocity mode move */
    motorAxisStop               /**< Pointer to function to stop motion */
  };

epicsExportAddress(drvet, pmacAsynMotor);


#define PMAC_STATUS1_MAXRAPID_SPEED    (0x1<<0)
#define PMAC_STATUS1_ALT_CMNDOUT_MODE  (0x1<<1)
#define PMAC_STATUS1_SOFT_POS_CAPTURE  (0x1<<2)
#define PMAC_STATUS1_ERROR_TRIGGER     (0x1<<3)
#define PMAC_STATUS1_FOLLOW_ENABLE     (0x1<<4)
#define PMAC_STATUS1_FOLLOW_OFFSET     (0x1<<5)
#define PMAC_STATUS1_PHASED_MOTOR      (0x1<<6)
#define PMAC_STATUS1_ALT_SRC_DEST      (0x1<<7)
#define PMAC_STATUS1_USER_SERVO        (0x1<<8)
#define PMAC_STATUS1_USER_PHASE        (0x1<<9)
#define PMAC_STATUS1_HOMING            (0x1<<10)
#define PMAC_STATUS1_BLOCK_REQUEST     (0x1<<11)
#define PMAC_STATUS1_DECEL_ABORT       (0x1<<12)
#define PMAC_STATUS1_DESIRED_VELOCITY_ZERO (0x1<<13)
#define PMAC_STATUS1_DATABLKERR        (0x1<<14)
#define PMAC_STATUS1_DWELL             (0x1<<15)
#define PMAC_STATUS1_INTEGRATE_MODE    (0x1<<16)
#define PMAC_STATUS1_MOVE_TIME_ON      (0x1<<17)
#define PMAC_STATUS1_OPEN_LOOP         (0x1<<18)
#define PMAC_STATUS1_AMP_ENABLED       (0x1<<19)
#define PMAC_STATUS1_X_SERVO_ON        (0x1<<20)
#define PMAC_STATUS1_POS_LIMIT_SET     (0x1<<21)
#define PMAC_STATUS1_NEG_LIMIT_SET     (0x1<<22)
#define PMAC_STATUS1_MOTOR_ON          (0x1<<23)

#define PMAC_STATUS2_IN_POSITION       (0x1<<0)
#define PMAC_STATUS2_WARN_FOLLOW_ERR   (0x1<<1)
#define PMAC_STATUS2_ERR_FOLLOW_ERR    (0x1<<2)
#define PMAC_STATUS2_AMP_FAULT         (0x1<<3)
#define PMAC_STATUS2_NEG_BACKLASH      (0x1<<4)
#define PMAC_STATUS2_I2T_AMP_FAULT     (0x1<<5)
#define PMAC_STATUS2_I2_FOLLOW_ERR     (0x1<<6)
#define PMAC_STATUS2_TRIGGER_MOVE      (0x1<<7)
#define PMAC_STATUS2_PHASE_REF_ERR     (0x1<<8)
#define PMAC_STATUS2_PHASE_SEARCH      (0x1<<9)
#define PMAC_STATUS2_HOME_COMPLETE     (0x1<<10)
#define PMAC_STATUS2_POS_LIMIT_STOP    (0x1<<11)
#define PMAC_STATUS2_DESIRED_STOP      (0x1<<12)
#define PMAC_STATUS2_FORE_IN_POS       (0x1<<13)
#define PMAC_STATUS2_NA14              (0x1<<14)
#define PMAC_STATUS2_ASSIGNED_CS       (0x1<<15)

typedef struct drvPmac * PMACDRV_ID;
typedef struct drvPmac
{
    PMACDRV_ID pNext;
    asynUser * pasynUser;
    int card;
    int nAxes;
    AXIS_HDL axis;
    epicsThreadId motorThread;
    epicsTimeStamp now;
} drvPmac_t;

typedef struct motorAxisHandle
{
    PMACDRV_ID pDrv;
    int axis;
    asynUser * pasynUser;
    PARAMS params;
    motorAxisLogFunc print;
    void * logParam;
    epicsMutexId axisMutex;
} motorAxis;

static PMACDRV_ID pFirstDrv = NULL;

static int drvPmacLogMsg( void * param, const motorAxisLogMask_t logMask, const char *pFormat, ...);
static motorAxisLogFunc drvPrint = drvPmacLogMsg;
static motorAxisLogFunc drvPrintParam = NULL;

#define TRACE_FLOW    motorAxisTraceFlow
#define TRACE_ERROR   motorAxisTraceError

#define MAX(a,b) ((a)>(b)? (a): (b))
#define MIN(a,b) ((a)<(b)? (a): (b))

static void motorAxisReportAxis( AXIS_HDL pAxis, int level )
{
    printf( "Found driver for drvPmac card %d, axis %d\n", pAxis->pDrv->card, pAxis->axis );

    if (level > 0)
    {
        motorParam->dump( pAxis->params );
    }
}

static void motorAxisReport( int level )
{
    PMACDRV_ID pDrv;

    for (pDrv = pFirstDrv; pDrv != NULL; pDrv = pDrv->pNext)
    {
        int i;

        for ( i = 0; i < pDrv->nAxes; i++ )
            motorAxisReportAxis( &(pDrv->axis[i]), level );
    }
}


static int motorAxisInit( void )
{
    return MOTOR_AXIS_OK;
}

static int motorAxisSetLog( AXIS_HDL pAxis, motorAxisLogFunc logFunc, void * param )
{
    if (pAxis == NULL) 
    {
        if (logFunc == NULL)
        {
            drvPrint=drvPmacLogMsg;
            drvPrintParam = NULL;
        }
        else
        {
            drvPrint=logFunc;
            drvPrintParam = param;
        }
    }
    else
    {
        if (logFunc == NULL)
        {
            pAxis->print=drvPmacLogMsg;
            pAxis->logParam = NULL;
        }
        else
        {
            pAxis->print=logFunc;
            pAxis->logParam = param;
        }
    }
    return MOTOR_AXIS_OK;
}

static AXIS_HDL motorAxisOpen( int card, int axis, char * param )
{
    PMACDRV_ID pDrv;
    AXIS_HDL pAxis = NULL;

    for ( pDrv=pFirstDrv; pDrv != NULL && (card != pDrv->card); pDrv = pDrv->pNext){}

    if (pDrv != NULL)
        if (axis >= 1 && axis <= pDrv->nAxes) pAxis = &(pDrv->axis[axis-1]);

    return pAxis;
}

static int motorAxisClose( AXIS_HDL pAxis )
{
    return MOTOR_AXIS_OK;
}

static int motorAxisGetInteger( AXIS_HDL pAxis, motorAxisParam_t function, int * value )
{
    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
        return motorParam->getInteger( pAxis->params, (paramIndex) function, value );
    }
}

static int motorAxisGetDouble( AXIS_HDL pAxis, motorAxisParam_t function, double * value )
{
    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
        return motorParam->getDouble( pAxis->params, (paramIndex) function, value );
    }
}

static int motorAxisSetCallback( AXIS_HDL pAxis, motorAxisCallbackFunc callback, void * param )
{
    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
        return motorParam->setCallback( pAxis->params, callback, param );
    }
}

static int motorAxisAsynConnect( const char * port, int addr, asynUser ** ppasynUser, char * inputEos, char * outputEos )
{
    asynStatus status;
 
    status = pasynOctetSyncIO->connect( port, addr, ppasynUser, NULL);
    if (status) {
        drvPrint( drvPrintParam, TRACE_ERROR,
                  "drvPmacCreate: unable to connect to port %s\n", 
                  port);
        return MOTOR_AXIS_ERROR;
    }

    status = pasynOctetSyncIO->setInputEos(*ppasynUser, inputEos, strlen(inputEos) );
    if (status) {
        asynPrint(*ppasynUser, ASYN_TRACE_ERROR,
                  "drvPmacCreate: unable to set input EOS on %s: %s\n", 
                  port, (*ppasynUser)->errorMessage);
        pasynOctetSyncIO->disconnect(*ppasynUser);
        return MOTOR_AXIS_ERROR;
    }

    status = pasynOctetSyncIO->setOutputEos(*ppasynUser, outputEos, strlen(outputEos));
    if (status) {
        asynPrint(*ppasynUser, ASYN_TRACE_ERROR,
                  "drvPmacCreate: unable to set output EOS on %s: %s\n", 
                  port, (*ppasynUser)->errorMessage);
        pasynOctetSyncIO->disconnect(*ppasynUser);
        return MOTOR_AXIS_ERROR;
    }

    return MOTOR_AXIS_OK;
}

static int motorAxisWriteRead( AXIS_HDL pAxis, char * command, size_t reply_buff_size, char * response, int logGlobal )
{
    asynStatus status;
    const double timeout=6.0;
    size_t nwrite, nread;
    int eomReason;
    asynUser * pasynUser = (logGlobal? pAxis->pDrv->pasynUser: pAxis->pasynUser);

    status = pasynOctetSyncIO->writeRead( pasynUser,
                                          command, strlen(command),
                                          response, reply_buff_size,
                                          timeout,
                                          &nwrite, &nread, &eomReason );

    if (status)
    {
        motorParam->setInteger( pAxis->params, motorAxisCommError, 1 );
        asynPrint( pasynUser,
                   ASYN_TRACE_ERROR,
                   "Read/write error to PMAC card %d, axis %d command %s. Status=%d, Error=%s\n",
                   pAxis->pDrv->card, pAxis->axis, command,
                   status, pasynUser->errorMessage);
        return MOTOR_AXIS_ERROR;
    }

    motorParam->setInteger( pAxis->params, motorAxisCommError, 0 );
    return MOTOR_AXIS_OK;
}

static int motorAxisSetDouble( AXIS_HDL pAxis, motorAxisParam_t function, double value )
{
    int status = MOTOR_AXIS_OK;

    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
        char command[32]="\0";
        char response[32];

        switch (function)
        {
        case motorAxisPosition:
        {
            sprintf( command, "%d=%f", pAxis->axis, value );
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Set card %d, axis %d to position %f\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisEncoderRatio:
        {
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Cannot set PMAC card %d, axis %d encoder ratio (%f)\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisLowLimit:
        {
            sprintf( command, "I%d14=%f", pAxis->axis, value );
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Setting PMAC card %d, axis %d low limit (%f)\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisHighLimit:
        {
            sprintf( command, "I%d13=%f", pAxis->axis, value );
           pAxis->print( pAxis->logParam, TRACE_FLOW, "Setting PMAC card %d, axis %d high limit (%f)\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisPGain:
        {
            sprintf( command, "I%d30=%f", pAxis->axis, value );
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Setting PMAC card %d, axis %d pgain (%f)\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisIGain:
        {
            sprintf( command, "I%d33=%f", pAxis->axis, value );
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Setting PMAC card %d, axis %d igain (%f)\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisDGain:
        {
            sprintf( command, "I%d31=%f", pAxis->axis, value );
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Setting PMAC card %d, axis %d dgain (%f)\n", pAxis->pDrv->card, pAxis->axis, value );
            break;
        }
        case motorAxisClosedLoop:
        {
            pAxis->print( pAxis->logParam, TRACE_FLOW, "Cannot set PMAC card %d, axis %d closed loop (%s)\n", pAxis->pDrv->card, pAxis->axis, (value!=0?"ON":"OFF") );
            break;
        }
        default:
            status = MOTOR_AXIS_ERROR;
            break;
        }


        if ( command[0] != 0 && status == MOTOR_AXIS_OK)
        {
            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
        }

        if (status == MOTOR_AXIS_OK ) status = motorParam->setDouble( pAxis->params, function, value );
    }
  return status;
}

static int motorAxisSetInteger( AXIS_HDL pAxis, motorAxisParam_t function, int value )
{
    int status = MOTOR_AXIS_OK;

    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
        status = motorAxisSetDouble( pAxis, function, (double) value );
    }
    return status;
}


static int motorAxisMove( AXIS_HDL pAxis, double position, int relative, double min_velocity, double max_velocity, double acceleration )
{
    int status = MOTOR_AXIS_ERROR;

    if (pAxis != NULL)
    {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
        char response[32];

        if (max_velocity != 0) sprintf(vel_buff, "I%d22=%f ", pAxis->axis, (max_velocity / 1000.0));
        if (acceleration != 0) sprintf(acc_buff, "I%d19=%f ", pAxis->axis, (fabs(acceleration) / 1000000.0));

        sprintf( command, "%s%s#%d %s%.2f", vel_buff, acc_buff, pAxis->axis, (relative?"MR ":"J="), position );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {
            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            motorParam->setInteger( pAxis->params, motorAxisMoving, 1 );
            motorParam->callCallback( pAxis->params );
            epicsMutexUnlock( pAxis->axisMutex );
        }
    }
    return status;
}

static int motorAxisHome( AXIS_HDL pAxis, double min_velocity, double max_velocity, double acceleration, int forwards )
{
    int status = MOTOR_AXIS_ERROR;

    if (pAxis != NULL)
    {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
        char response[32];

        if (max_velocity != 0) sprintf(vel_buff, "I%d23=%f ", pAxis->axis, (forwards?1:-1)*(fabs(max_velocity) / 1000.0));
        if (acceleration != 0) sprintf(acc_buff, "I%d19=%f ", pAxis->axis, (fabs(acceleration) / 1000000.0));
        sprintf( command, "%s%s#%d HOME", vel_buff, acc_buff,  pAxis->axis );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {
            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            motorParam->setInteger( pAxis->params, motorAxisMoving, 1 );
            motorParam->callCallback( pAxis->params );
            epicsMutexUnlock( pAxis->axisMutex );
        }
    }
    return status;
}


static int motorAxisVelocityMove(  AXIS_HDL pAxis, double min_velocity, double velocity, double acceleration )
{
    int status = MOTOR_AXIS_ERROR;

    if (pAxis != NULL)
    {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
        char response[32];

        if (velocity != 0) sprintf(vel_buff, "I%d22=%f ", pAxis->axis, (fabs(velocity) / 1000.0));
        if (acceleration != 0) sprintf(acc_buff, "I%d19=%f ", pAxis->axis, (fabs(acceleration) / 1000000.0));

        sprintf( command, "%s%s#%d %s", vel_buff, acc_buff, pAxis->axis, (velocity < 0 ? "J-": "J+") );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {
            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            motorParam->setInteger( pAxis->params, motorAxisMoving, 1 );
            motorParam->callCallback( pAxis->params );
            epicsMutexUnlock( pAxis->axisMutex );
        }
    }
    return status;
}

static int motorAxisProfileMove( AXIS_HDL pAxis, int npoints, double positions[], double times[], int relative, int trigger )
{
  return MOTOR_AXIS_ERROR;
}

static int motorAxisTriggerProfile( AXIS_HDL pAxis )
{
  return MOTOR_AXIS_ERROR;
}

static int motorAxisStop( AXIS_HDL pAxis, double acceleration )
{
    int status = MOTOR_AXIS_ERROR;

    if (pAxis != NULL)
    {
        char acc_buff[32]="\0";
        char command[128];
        char response[32];

        if (acceleration != 0) sprintf(acc_buff, "I%d19=%f ", pAxis->axis, (fabs(acceleration) / 1000000.0));

        sprintf( command, "%s#%d J/",  acc_buff, pAxis->axis );

        status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
    }
    return status;
}

/** \defgroup drvPmacTask Routines to implement the motor axis simulation task
@{

*/

/** Process one iteration of an axis

    This routine takes a single axis and propogates its motion forward a given amount
    of time.

    \param pAxis  [in]   Pointer to axis information.
    \param delta  [in]   Time in seconds to propogate motion forwards.

    \return Integer indicating 0 (MOTOR_AXIS_OK) for success or non-zero for failure.
*/


static void drvPmacGetAxisStatus( AXIS_HDL pAxis, asynUser * pasynUser )
{
    char command[128];
    char response[128];
    int cmdStatus;
    double position, error, velocity;
    int nvals;
    epicsUInt32 status[2];

    if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
    {
        /* Read all the status for this axis in one go */
        sprintf( command, "#%d ? P F V", pAxis->axis );
        cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 1 );
        nvals = sscanf( response, "%6x%6x %lf %lf %lf", &status[0], &status[1], &position, &error, &velocity );

        if ( cmdStatus || nvals != 5)
        {
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                      "drvPmacAxisGetStatus: not all status values returned\n" );
        }
        else
        {
            motorParam->setDouble(  pAxis->params, motorAxisPosition,      (position+error) );
            motorParam->setDouble(  pAxis->params, motorAxisEncoderPosn,   position );

            motorParam->setInteger( pAxis->params, motorAxisDirection,     (velocity >= 0) );
            motorParam->setInteger( pAxis->params, motorAxisDone,          ((status[0] & PMAC_STATUS1_DESIRED_VELOCITY_ZERO) != 0) );
            motorParam->setInteger( pAxis->params, motorAxisHighHardLimit, ((status[0] & PMAC_STATUS1_POS_LIMIT_SET) != 0) );
            motorParam->setInteger( pAxis->params, motorAxisHomeSignal,    ((status[1] & PMAC_STATUS2_HOME_COMPLETE) != 0) );
            motorParam->setInteger( pAxis->params, motorAxisMoving,        ((status[0] & PMAC_STATUS1_DESIRED_VELOCITY_ZERO) == 0) );
            motorParam->setInteger( pAxis->params, motorAxisLowHardLimit,  ((status[0] & PMAC_STATUS1_NEG_LIMIT_SET)!=0) );
            motorParam->callCallback( pAxis->params );           
        }
        epicsMutexUnlock( pAxis->axisMutex );
    }
}

static void drvPmacGetAxisInitialStatus( AXIS_HDL pAxis, asynUser * pasynUser )
{
    char command[32];
    char response[128];
    int cmdStatus;
    double low_limit, high_limit, pgain, igain, dgain;
    int nvals;

    /* Read all the status for this axis in one go */
    if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
    {
        sprintf( command, "I%d13 I%d14 I%d30 I%d31 I%d33", pAxis->axis, pAxis->axis, pAxis->axis, pAxis->axis, pAxis->axis );
        cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 1 );
        nvals = sscanf( response, "%lf %lf %lf %lf %lf", &high_limit, &low_limit, &pgain, &dgain, &igain );

        if ( cmdStatus || nvals != 5)
        {
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                      "drvPmacGetAxisInitialStatus: not all status values returned\n" );
        }
        else
        {
            motorParam->setDouble(  pAxis->params, motorAxisLowLimit,  low_limit );
            motorParam->setDouble(  pAxis->params, motorAxisHighLimit, high_limit );
            motorParam->setDouble(  pAxis->params, motorAxisPGain,     pgain );
            motorParam->setDouble(  pAxis->params, motorAxisIGain,     igain );
            motorParam->setDouble(  pAxis->params, motorAxisDGain,     dgain );
            motorParam->callCallback( pAxis->params );
        }

        epicsMutexUnlock( pAxis->axisMutex );
    }
}

#define DELTA 0.1
static void drvPmacTask( PMACDRV_ID pDrv )
{
    while ( 1 )
    {
        int i;

	for ( i = 0; i < pDrv->nAxes; i++ )
	{
            AXIS_HDL pAxis = &(pDrv->axis[i]);

            drvPmacGetAxisStatus( pAxis, pDrv->pasynUser );
        }

        epicsThreadSleep( DELTA );
    }
}

int pmacAsynMotorCreate( char *port, int addr, int card, int nAxes )
{
    int i;
    int status = MOTOR_AXIS_OK;
    PMACDRV_ID pDrv;
    PMACDRV_ID * ppLast = &(pFirstDrv);

    for ( pDrv = pFirstDrv;
          pDrv != NULL &&  (pDrv->card != card);
          pDrv = pDrv->pNext )
    {
        ppLast = &(pDrv->pNext);
    }

    if (nAxes < 1 ) nAxes = 1;

    if ( pDrv == NULL)
    {
        drvPrint( drvPrintParam, TRACE_FLOW,
               "Creating PMAC motor driver on port %s, address %d: card: %d, naxes: %d\n",
               port, addr, card, nAxes );

        pDrv = (PMACDRV_ID) calloc( 1, sizeof(drvPmac_t) );

        if (pDrv != NULL)
	{
            pDrv->axis = (AXIS_HDL) calloc( nAxes, sizeof( motorAxis ) );

            if (pDrv->axis != NULL )
            {
                pDrv->nAxes = nAxes;
                pDrv->card = card;
                status = motorAxisAsynConnect( port, addr, &(pDrv->pasynUser), "\006", "\r" );

                for (i=0; i<nAxes && status == MOTOR_AXIS_OK; i++ )
                {
                    if ((pDrv->axis[i].params = motorParam->create( 0, MOTOR_AXIS_NUM_PARAMS )) != NULL &&
                        (pDrv->axis[i].axisMutex = epicsMutexCreate( )) != NULL )
                    {
                        char command[32], reply[32];

                        pDrv->axis[i].pDrv = pDrv;

                        pDrv->axis[i].axis = i+1;
                        pDrv->axis[i].logParam  = pDrv->pasynUser;
                        pDrv->axis[i].pasynUser = pDrv->pasynUser;

                        sprintf( command, "I%d00=1", pDrv->axis[i].axis );
                        motorAxisWriteRead( &(pDrv->axis[i]), command, sizeof(reply), reply, 0 );
                        asynPrint( pDrv->pasynUser, ASYN_TRACE_FLOW, "Created motor for card %d, signal %d OK\n", card, i );
                    }
                    else
                    {
                        asynPrint(pDrv->pasynUser, ASYN_TRACE_ERROR,
                                  "drvPmacCreate: unable to set create axis %d on %s: insufficient memory\n", 
                                  i, port );
                       status = MOTOR_AXIS_ERROR;
                    }
                }

                if ( status == MOTOR_AXIS_ERROR )
                {
                    for (i=0; i<nAxes; i++ )
                    {
                        if (pDrv->axis[i].params != NULL) motorParam->destroy( pDrv->axis[i].params );
                        if (pDrv->axis[i].axisMutex != NULL) epicsMutexDestroy( pDrv->axis[i].axisMutex );
                    }
                    free ( pDrv );
                }
            }
            else
            {
                free ( pDrv );
                status = MOTOR_AXIS_ERROR;
            }
        }
        else
        {
            drvPrint( drvPrintParam, TRACE_ERROR,
                      "drvPmacCreate: unable to create driver for port %s: insufficient memory\n",
                      port );
            status = MOTOR_AXIS_ERROR;
        }

        if ( status == MOTOR_AXIS_OK ) *ppLast = pDrv;
    }
    else
    {
        drvPrint( drvPrintParam, TRACE_ERROR, "Motor for card %d already exists\n", card );
        status = MOTOR_AXIS_ERROR;
    }

    if (status == MOTOR_AXIS_OK)
    {
        pDrv->motorThread = epicsThreadCreate( "drvPmacThread",
                                               epicsThreadPriorityLow,
                                               epicsThreadGetStackSize(epicsThreadStackMedium),
                                               (EPICSTHREADFUNC) drvPmacTask, (void *) pDrv );
        if (pDrv->motorThread == NULL)
        {
            asynPrint(pDrv->pasynUser, ASYN_TRACE_ERROR, "Cannot start motor polling thread\n" );
            return MOTOR_AXIS_ERROR;
        }
    }

    return status;
}

static int drvPmacLogMsg( void * param, const motorAxisLogMask_t mask, const char *pFormat, ...)
{

    va_list	pvar;
    int		nchar=0;
    asynUser *  pasynUser =  (asynUser *) param;
    int         reason = (int) mask;

    if ( pasynUser == NULL )
    {
        va_start(pvar, pFormat);
        vprintf( pFormat, pvar );
        va_end (pvar);
    }
    else if ( pasynTrace->getTraceMask(pasynUser) & reason )
    {
        va_start(pvar, pFormat);
        nchar = pasynTrace->vprint( pasynUser, reason, pFormat, pvar );
        va_end (pvar);
    }

    return(nchar);
}
