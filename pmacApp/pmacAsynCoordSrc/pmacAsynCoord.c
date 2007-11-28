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

motorAxisDrvSET_t pmacAsynCoord = {
    14,
    motorAxisReport,            /**< Standard EPICS driver report function (optional) */
    motorAxisInit,              /**< Standard EPICS driver initialisation function (optional) */
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

epicsExportAddress(drvet, pmacAsynCoord);

#define CS_STATUS1_RUNNING_PROG       (0x1<<0)
#define CS_STATUS1_SINGLE_STEP_MODE   (0x1<<1)
#define CS_STATUS1_CONTINUOUS_MODE    (0x1<<2)
#define CS_STATUS1_MOVE_BY_TIME_MODE  (0x1<<3)
#define CS_STATUS1_CONTINUOUS_REQUEST (0x1<<4)
#define CS_STATUS1_RADIUS_INC_MODE    (0x1<<5)
#define CS_STATUS1_A_INC              (0x1<<6)
#define CS_STATUS1_A_FEEDRATE         (0x1<<7)
#define CS_STATUS1_B_INC              (0x1<<8)
#define CS_STATUS1_B_FEEDRATE         (0x1<<9)
#define CS_STATUS1_C_INC              (0x1<<10)
#define CS_STATUS1_C_FEEDRATE         (0x1<<11)
#define CS_STATUS1_U_INC              (0x1<<12)
#define CS_STATUS1_U_FEEDRATE         (0x1<<13)
#define CS_STATUS1_V_INC              (0x1<<14)
#define CS_STATUS1_V_FEEDRATE         (0x1<<15)
#define CS_STATUS1_W_INC              (0x1<<16)
#define CS_STATUS1_W_FEEDRATE         (0x1<<17)
#define CS_STATUS1_X_INC              (0x1<<18)
#define CS_STATUS1_X_FEEDRATE         (0x1<<19)
#define CS_STATUS1_Y_INC              (0x1<<20)
#define CS_STATUS1_Y_FEEDRATE         (0x1<<21)
#define CS_STATUS1_Z_INC              (0x1<<22)
#define CS_STATUS1_Z_FEEDRATE         (0x1<<23)

#define CS_STATUS2_CIRCLE_SPLINE_MODE (0x1<<0)
#define CS_STATUS2_CCW_RAPID_MODE     (0x1<<1)
#define CS_STATUS2_2D_CUTTER_COMP     (0x1<<2)
#define CS_STATUS2_2D_LEFT_3D_CUTTER  (0x1<<3)
#define CS_STATUS2_PVT_SPLINE_MODE    (0x1<<4)
#define CS_STATUS2_SEG_STOPPING       (0x1<<5)
#define CS_STATUS2_SEG_ACCEL          (0x1<<6)
#define CS_STATUS2_SEG_MOVING         (0x1<<7)
#define CS_STATUS2_PRE_JOG            (0x1<<8)
#define CS_STATUS2_CUTTER_MOVE_BUFFD  (0x1<<9)
#define CS_STATUS2_CUTTER_STOP        (0x1<<10)
#define CS_STATUS2_CUTTER_COMP_OUTSIDE (0x1<<11)
#define CS_STATUS2_DWELL_MOVE_BUFFD   (0x1<<12)
#define CS_STATUS2_SYNCH_M_ONESHOT    (0x1<<13)
#define CS_STATUS2_EOB_STOP           (0x1<<14)
#define CS_STATUS2_DELAYED_CALC       (0x1<<15)
#define CS_STATUS2_ROTARY_BUFF	      (0x1<<16)
#define CS_STATUS2_IN_POSITION	      (0x1<<17)
#define CS_STATUS2_FOLLOW_WARN	      (0x1<<18)
#define CS_STATUS2_FOLLOW_ERR	      (0x1<<19)
#define CS_STATUS2_AMP_FAULT	      (0x1<<20)
#define CS_STATUS2_MOVE_IN_STACK      (0x1<<21)
#define CS_STATUS2_RUNTIME_ERR	      (0x1<<22)
#define CS_STATUS2_LOOKAHEAD	      (0x1<<23)

#define CS_STATUS3_LIMIT	      (0x1<<1)

typedef struct drvPmac * PMACDRV_ID;
typedef struct drvPmac
{
    PMACDRV_ID pNext;
    asynUser * pasynUser;
    int card;
    int cs;
    AXIS_HDL axis;
    epicsThreadId motorThread;
    epicsTimeStamp now;
} drvPmac_t;

/* These two #define affect the behaviour of the driver.

   REMOVE_LIMITS_ON_HOME removes the limits protection when homing if
    - ms??,i912 indicates you are homing onto a limit
    - ms??,i913 and the home velocity indicate that the limit you trigger.
      for home detection is the one you are homing towards.
    - any home offset is in the opposite sense to the home velocity.
*/

#undef REMOVE_LIMITS_ON_HOME
/*#define REMOVE_LIMITS_ON_HOME*/

typedef struct motorAxisHandle
{
    PMACDRV_ID pDrv;
    int coord_system;
    int axis;
    asynUser * pasynUser;
    PARAMS params;
    motorAxisLogFunc print;
    void * logParam;
    epicsMutexId axisMutex;
#ifdef SIMULATE_SET_POSITION
    double enc_offset;
    int homeSignal;
#endif
#ifdef REMOVE_LIMITS_ON_HOME
    int limitsDisabled;
#endif
} motorAxis;

const char axisName[] = {' ', 'A','B','C','U','V','W','X','Y','Z'};

static PMACDRV_ID pFirstDrv = NULL;

static int drvPmacLogMsg( void * param, const motorAxisLogMask_t logMask, const char *pFormat, ...);
static motorAxisLogFunc drvPrint = drvPmacLogMsg;
static motorAxisLogFunc drvPrintParam = NULL;

#define TRACE_FLOW    motorAxisTraceFlow
#define TRACE_DRIVER  motorAxisTraceIODriver
#define TRACE_ERROR   motorAxisTraceError

#define MAX(a,b) ((a)>(b)? (a): (b))
#define MIN(a,b) ((a)<(b)? (a): (b))

/* PMAC specific parameters relating to coordinate systems */
#define NAXES	9
#define NAME(pAxis) (axisName[(pAxis->axis)])
#define CSVAR(pAxis) (pAxis->coord_system + 50)
/* Use Q71 - Q79 for motor demand positions */
#define DEMAND "Q7%d"
/* Use Q81 - Q89 for motor readback positions */
#define READBACK "Q8%d"


static void motorAxisReportAxis( AXIS_HDL pAxis, int level )
{
    printf( "Found driver for drvPmac card %d, C.S. %d, axis %c\n", pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis) );

    if (level > 0) printf( "drvPmac->axisMutex = %p\n", pAxis->axisMutex );
#ifdef SIMULATE_SET_POSITION
    if (level > 0) printf( "Encoder offset = %f\n", pAxis->enc_offset );
#endif

    if (level > 1) {
        motorParam->dump( pAxis->params );
    }
}

static void motorAxisReport( int level )
{
    PMACDRV_ID pDrv;

    for (pDrv = pFirstDrv; pDrv != NULL; pDrv = pDrv->pNext) {
        int i;

        for ( i = 0; i < NAXES; i++ ) {
            motorAxisReportAxis( &(pDrv->axis[i]), level );
        }
    }
}


static int motorAxisInit( void )
{
    return MOTOR_AXIS_OK;
}

static int motorAxisSetLog( AXIS_HDL pAxis, motorAxisLogFunc logFunc, void * param )
{
    if (pAxis == NULL) {
        if (logFunc == NULL) {
            drvPrint=drvPmacLogMsg;
            drvPrintParam = NULL;
        } else {
            drvPrint=logFunc;
            drvPrintParam = param;
        }
    } else {
        if (logFunc == NULL) {
            pAxis->print=drvPmacLogMsg;
            pAxis->logParam = NULL;
        } else {
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

    if (pDrv != NULL) {
        if (axis >= 0 && axis < NAXES) pAxis = &(pDrv->axis[axis]);
    }
        
    return pAxis;
}

static int motorAxisClose( AXIS_HDL pAxis )
{
    return MOTOR_AXIS_OK;
}

static int motorAxisGetInteger( AXIS_HDL pAxis, motorAxisParam_t function, int * value )
{
    if (pAxis == NULL) {
    	return MOTOR_AXIS_ERROR;
    } else {
        return motorParam->getInteger( pAxis->params, (paramIndex) function, value );
    }
}

static int motorAxisGetDouble( AXIS_HDL pAxis, motorAxisParam_t function, double * value )
{
    if (pAxis == NULL) {
    	return MOTOR_AXIS_ERROR;
    } else {
        return motorParam->getDouble( pAxis->params, (paramIndex) function, value );
    }
}

static int motorAxisSetCallback( AXIS_HDL pAxis, motorAxisCallbackFunc callback, void * param )
{
    if (pAxis == NULL) {
    	return MOTOR_AXIS_ERROR;
    } else {
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

    if ( !logGlobal ) {
        pAxis->print( pAxis->logParam, TRACE_DRIVER, "Sending to PMAC %d command : %s\n",pAxis->pDrv->card, command );
    }
        
    status = pasynOctetSyncIO->writeRead( pasynUser,
                                          command, strlen(command),
                                          response, reply_buff_size,
                                          timeout,
                                          &nwrite, &nread, &eomReason );

    if ( !logGlobal && nread != 0 ) {
        pAxis->print( pAxis->logParam, TRACE_DRIVER, "Recvd from PMAC %d response: %s\n",pAxis->pDrv->card, response );
    }
        
    if (status) {
        motorParam->setInteger( pAxis->params, motorAxisCommError, 1 );
        asynPrint( pasynUser,
                   ASYN_TRACE_ERROR,
                   "Read/write error to PMAC card %d, axis %c command %s. Status=%d, Error=%s\n",
                   pAxis->pDrv->card, NAME(pAxis), command,
                   status, pasynUser->errorMessage);
        return MOTOR_AXIS_ERROR;
    }

    motorParam->setInteger( pAxis->params, motorAxisCommError, 0 );
    return MOTOR_AXIS_OK;
}

static int motorAxisSetDouble( AXIS_HDL pAxis, motorAxisParam_t function, double value )
{
    int status = MOTOR_AXIS_OK;

    if (pAxis == NULL) {
    	return MOTOR_AXIS_ERROR;
    } else {
        char command[64]="\0";
        char response[64];

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK) {
            switch (function) {
            case motorAxisPosition: {
                int position = (int) floor(value*32 + 0.5);

                sprintf( command, "&%d%c=%d",
                         pAxis->coord_system, NAME(pAxis), position);

                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Set card %d, axis %c to position %f\n",
                              pAxis->pDrv->card, NAME(pAxis), value );
                break;
            }
            case motorAxisEncoderRatio: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot set PMAC card %d, cs %d, axis %c encoder ratio (%f)\n",
                              pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisLowLimit: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC card %d, cs %d, axis %c low limit (%f)\n",
                              pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisHighLimit: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC card %d, cs %d, axis %c high limit (%f)\n",
                              pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisPGain: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC card %d, cs %d, axis %c pgain (%f)\n",
                              pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisIGain: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC card %d, cs %d, axis %c igain (%f)\n",
                              pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisDGain: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC card %d, cs %d, axis %c dgain (%f)\n",
                              pAxis->pDrv->card, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisClosedLoop: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot set PMAC card %d, axis %c closed loop (%s)\n",
                              pAxis->pDrv->card, NAME(pAxis), (value!=0?"ON":"OFF") );
                break;
            }
            default:
                status = MOTOR_AXIS_ERROR;
                break;
            }

            if ( command[0] != 0 && status == MOTOR_AXIS_OK) {
                status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            }

            if (status == MOTOR_AXIS_OK ) {
                motorParam->setDouble( pAxis->params, function, value );
                motorParam->callCallback( pAxis->params );
            }
            epicsMutexUnlock( pAxis->axisMutex );
        }
    }
  return status;
}

static int motorAxisSetInteger( AXIS_HDL pAxis, motorAxisParam_t function, int value )
{
    int status = MOTOR_AXIS_OK;

    if (pAxis == NULL) {
    	return MOTOR_AXIS_ERROR;
    } else {
        status = motorAxisSetDouble( pAxis, function, (double) value );
    }
    return status;
}


static int motorAxisMove( AXIS_HDL pAxis, double position, int relative, double min_velocity, double max_velocity, double acceleration )
{
    int status = MOTOR_AXIS_ERROR;

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
        char response[32];

        if (max_velocity != 0) {
        	sprintf(vel_buff, "I%d89=%f ", CSVAR(pAxis), max_velocity);
        }
        if (acceleration != 0) {
            if (max_velocity != 0) {
                sprintf(acc_buff, "I%d87=%f ", CSVAR(pAxis),
                		(fabs(max_velocity/acceleration) * 1000.0));
            }
        }

        sprintf( command, "%s%s&%d"DEMAND"=%.2fR", vel_buff, acc_buff,
        		pAxis->coord_system, pAxis->axis, position );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK) {
#ifdef REMOVE_LIMITS_ON_HOME
	    //            if ( pAxis->limitsDisabled )
            //{
	    //                char buffer[32];

                /* Re-enable limits */
	    //                sprintf( buffer, " i%d24=i%d24&$FDFFFF", pAxis->axis, pAxis->axis );
	    //                strcat( command, buffer );
	    //                pAxis->limitsDisabled = 0;
	    //            }
#endif

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

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
        char response[128];

        if (max_velocity != 0) {
        	sprintf(vel_buff, "I%d23=%f ", pAxis->axis, (forwards?1:-1)*(fabs(max_velocity) / 1000.0));
        }
        if (acceleration != 0) {
            if (max_velocity != 0) {
                sprintf(acc_buff, "I%d20=%f ", pAxis->axis, (fabs(max_velocity/acceleration) * 1000.0));
            }
        }
        sprintf( command, "%s%s#%d HOME", vel_buff, acc_buff,  pAxis->axis );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK) {
                
#ifdef REMOVE_LIMITS_ON_HOME
            /* If homing onto an end-limit and home velocity is in the right direction, clear limits protection */
            int macro_station = ((pAxis->axis-1)/2)*4 + (pAxis->axis-1)%2;
            int home_type, home_flag, flag_mode, nvals, home_offset;
            double home_velocity;
            char buffer[128];

            /* Read home flags and home direction from PMAC */ 
            sprintf( buffer, "ms%d,i912 ms%d,i913 i%d24 i%d23 i%d26", macro_station, macro_station, pAxis->axis, pAxis->axis, pAxis->axis );
            status = motorAxisWriteRead( pAxis, buffer, sizeof(response), response, 0 );
            nvals = sscanf( response, "$%x $%x $%x %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset );
            if (nvals !=5)
            	nvals = sscanf( response, "%d %d %d %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset );
            if (max_velocity != 0) home_velocity = (forwards?1:-1)*(fabs(max_velocity) / 1000.0);

            if ( status || nvals != 5) {
                pAxis->print( pAxis->logParam, TRACE_ERROR,
			      "drvPmac motorAxisHome: can not read home flags. Status: %d, nvals %d\n", status, nvals );
            } else if ( ( home_type <= 15 )      && 
                      ( home_type % 4 >= 2 )   &&
                      !( flag_mode & 0x20000 ) &&
                      (( home_velocity > 0 && home_flag == 1 && home_offset <= 0 ) || 
                       ( home_velocity < 0 && home_flag == 2 && home_offset >= 0 )    )   ) {
                sprintf( buffer, " i%d24=i%d24|$20000", pAxis->axis, pAxis->axis );
                strcat( command, buffer );
                pAxis->limitsDisabled = 1;
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Disabling limits whilst homing PMAC card %d, axis %d, type:%d, flag:$%x, vel:%f\n",
                              pAxis->pDrv->card, pAxis->axis, home_type, home_flag, home_velocity );
            } else {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot disable limits to home PMAC card %d, axis %d, type:%x, flag:$%d, vel:%f, mode:0x%x, offset: %d\n",
                              pAxis->pDrv->card, pAxis->axis, home_type, home_flag, home_velocity, flag_mode, home_offset );
            }
#endif

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

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
        char response[32];

        if (velocity != 0) {
        	sprintf(vel_buff, "I%d22=%f ", pAxis->axis, (fabs(velocity) / 1000.0));
        }
        if (acceleration != 0) {
            if (velocity != 0) {
                sprintf(acc_buff, "I%d20=%f ", pAxis->axis, (fabs(velocity/acceleration) * 1000.0));
            }
        }

        sprintf( command, "%s%s#%d %s", vel_buff, acc_buff, pAxis->axis, (velocity < 0 ? "J-": "J+") );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK) {
#ifdef REMOVE_LIMITS_ON_HOME
            if ( pAxis->limitsDisabled )
            {
                char buffer[32];

                /* Re-enable limits */
                sprintf( buffer, " i%d24=i%d24&$FDFFFF", pAxis->axis, pAxis->axis );
                strcat( command, buffer );
                pAxis->limitsDisabled = 0;
            }
#endif

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

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char command[128];
        char response[32];

        sprintf( command, "%s&%dA",  acc_buff, pAxis->coord_system );

        status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
    }
    return status;
}


static void drvPmacGetAxisStatus( AXIS_HDL pAxis, asynUser * pasynUser,
				  epicsUInt32 *status)
{
    char command[128];
    char response[128];
    int cmdStatus;
    double position, error, velocity;
    int nvals;

    error = 0;
    if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK) {
        /* Read all the status for this co-ordinate system in one go */
    	sprintf( command, "&%d"READBACK/*"F V"*/, pAxis->coord_system,
    			pAxis->axis );
        cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 1 );
        nvals = sscanf( response, "%lf"/*" %lf %lf"*/, &position/*,&error, &velocity*/ );

        if ( cmdStatus || nvals != 1) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                      "drvPmacAxisGetStatus: not all status values returned. Status: %d\nCommand :%s\nResponse:%s",
                      cmdStatus, command, response );
        } else {
            int direction;
/*            int homeSignal = ((status[1] & CS_STATUS2_HOME_COMPLETE) != 0);*/
/* TODO: possibly look at the aggregate of the home status of all motors in the c.s ?? */
            int homeSignal = 0;

            motorParam->setDouble(  pAxis->params, motorAxisPosition,      (position+error) );
            motorParam->setDouble(  pAxis->params, motorAxisEncoderPosn,   position );

            /* Don't set direction if velocity equals zero and was previously negative */
            motorParam->getInteger( pAxis->params, motorAxisDirection,     &direction );
            motorParam->setInteger( pAxis->params, motorAxisDirection,     ((velocity >= 0) || (velocity == 0 && direction)) );
            motorParam->setInteger( pAxis->params, motorAxisDone,          ((status[1] & CS_STATUS2_IN_POSITION) != 0) );
            motorParam->setInteger( pAxis->params, motorAxisHighHardLimit, ((status[0] & CS_STATUS3_LIMIT) != 0) );
            motorParam->setInteger( pAxis->params, motorAxisHomeSignal,    homeSignal );
            motorParam->setInteger( pAxis->params, motorAxisMoving,        ((status[0] & CS_STATUS2_IN_POSITION) == 0) );
            motorParam->setInteger( pAxis->params, motorAxisLowHardLimit,  ((status[0] & CS_STATUS3_LIMIT)!=0) );
            motorParam->callCallback( pAxis->params );           
        }

#ifdef REMOVE_LIMITS_ON_HOME
/*        if ( pAxis->limitsDisabled && (status[1] & CS_STATUS2_HOME_COMPLETE) )
        {
            /* Re-enable limits */
/*            sprintf( command, "i%d24=i%d24&$FDFFFF", pAxis->axis, pAxis->axis );
            cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            pAxis->limitsDisabled = (cmdStatus != 0);
	}*/
#endif

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

    return;
}


static int drvPmacGetCoordStatus(AXIS_HDL pAxis, asynUser *pasynUser,
				 epicsUInt32 *status)
{
    char command[32];
    char response[128];
    int nvals, cmdStatus;
    int retval = 0;

    if (epicsMutexLock(pAxis->axisMutex) == epicsMutexLockOK) {
	/* Read all the status for this co-ordinate system in one go */
    	sprintf( command, "&%d??", pAxis->coord_system );
    	cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 1 );
        nvals = sscanf( response, "%6x%6x%6x", &status[0], &status[1], &status[2]);

        if ( cmdStatus || nvals != 3) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                      "drvPmacAxisGetStatus: not all status values returned. Status: %d\nCommand :%s\nResponse:%s",
                      cmdStatus, command, response );
        } else {
        	retval = 1;
        }
        epicsMutexUnlock(pAxis->axisMutex);
    }
    return retval;
}


#define DELTA 0.1
static void drvPmacTask( PMACDRV_ID pDrv )
{
    while ( 1 ) {
        int i;
        epicsUInt32 status[3];

        /* Use the first axis to get the overall C.S. status */
        if(drvPmacGetCoordStatus(&(pDrv->axis[0]), pDrv->pasynUser, status)) {
        	for ( i = 0; i < NAXES; i++ ) {
        		AXIS_HDL pAxis = &(pDrv->axis[i]);

        		drvPmacGetAxisStatus( pAxis, pDrv->pasynUser, status);
        	}
        }

        epicsThreadSleep( DELTA );
    }
}

int pmacAsynCoordCreate( char *port, int addr, int card, int cs )
{
    int i;
    int status = MOTOR_AXIS_OK;
    PMACDRV_ID pDrv;
    PMACDRV_ID * ppLast = &(pFirstDrv);

    if ((cs < 1) || (cs > 16)) {
    	drvPrint( drvPrintParam, TRACE_ERROR, "Invalid co-ordinate system number: %d\n", cs);
    	return MOTOR_AXIS_ERROR;
    }

    for ( pDrv = pFirstDrv;
          pDrv != NULL &&  (pDrv->card != card) && (pDrv->cs != cs);
          pDrv = pDrv->pNext )
    {
        ppLast = &(pDrv->pNext);
    }

    if ( pDrv == NULL) {
        drvPrint( drvPrintParam, TRACE_FLOW,
               "Creating PMAC co-ordinate system driver on port %s, address %d: card: %d, cs: %d\n",
               port, addr, card, cs );

        pDrv = (PMACDRV_ID) calloc( 1, sizeof(drvPmac_t) );

        if (pDrv != NULL) {
            pDrv->axis = (AXIS_HDL) calloc( NAXES, sizeof( motorAxis ) );

            if (pDrv->axis != NULL ) {
                pDrv->cs = cs;
                pDrv->card = card;
                status = motorAxisAsynConnect( port, addr, &(pDrv->pasynUser), "\006", "\r" );

                for (i=0; i<NAXES && status == MOTOR_AXIS_OK; i++ ) {
                    if ((pDrv->axis[i].params = motorParam->create( 0, MOTOR_AXIS_NUM_PARAMS )) != NULL &&
                        (pDrv->axis[i].axisMutex = epicsMutexCreate( )) != NULL ) {
                        pDrv->axis[i].pDrv = pDrv;
                        pDrv->axis[i].coord_system = cs;
                        pDrv->axis[i].axis = i + 1;
                        pDrv->axis[i].logParam  = pDrv->pasynUser;
                        pDrv->axis[i].pasynUser = pDrv->pasynUser;

                        asynPrint( pDrv->pasynUser, ASYN_TRACE_FLOW, "Created motor for card %d, C.S. %d, signal %d OK\n", card, cs, i );
                    } else {
                        asynPrint(pDrv->pasynUser, ASYN_TRACE_ERROR,
                                  "drvPmacCreate: unable to set create axis %d on %s: insufficient memory\n", 
                                  i, port );
                       status = MOTOR_AXIS_ERROR;
                    }
                }

                if ( status == MOTOR_AXIS_ERROR ) {
                    for (i=0; i<NAXES; i++ ) {
                        if (pDrv->axis[i].params != NULL) motorParam->destroy( pDrv->axis[i].params );
                        if (pDrv->axis[i].axisMutex != NULL) epicsMutexDestroy( pDrv->axis[i].axisMutex );
                    }
                    free ( pDrv );
                }
            } else {
                free ( pDrv );
                status = MOTOR_AXIS_ERROR;
            }
        } else {
            drvPrint( drvPrintParam, TRACE_ERROR,
                      "drvPmacCreate: unable to create driver for port %s: insufficient memory\n",
                      port );
            status = MOTOR_AXIS_ERROR;
        }

        if ( status == MOTOR_AXIS_OK ) {
        	*ppLast = pDrv;
        }
    } else {
        drvPrint( drvPrintParam, TRACE_ERROR, "C.S. %d for card %d already exists\n", cs, card );
        status = MOTOR_AXIS_ERROR;
    }

    if (status == MOTOR_AXIS_OK) {
        int i;
        epicsUInt32 status[3];

        /* Do an initial poll of all status */
        drvPmacGetCoordStatus(&(pDrv->axis[0]), pDrv->pasynUser, status);
        for ( i = 0; i < NAXES; i++ ) {
            AXIS_HDL pAxis = &(pDrv->axis[i]);

            drvPmacGetAxisInitialStatus( pAxis, pDrv->pasynUser );
            drvPmacGetAxisStatus( pAxis, pDrv->pasynUser, status );
        }

        pDrv->motorThread = epicsThreadCreate( "drvPmacCSThread",
                                               epicsThreadPriorityLow,
                                               epicsThreadGetStackSize(epicsThreadStackMedium),
                                               (EPICSTHREADFUNC) drvPmacTask, (void *) pDrv );
        if (pDrv->motorThread == NULL) {
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

    if ( pasynUser == NULL ) {
        va_start(pvar, pFormat);
        vprintf( pFormat, pvar );
        va_end (pvar);
    } else if ( pasynTrace->getTraceMask(pasynUser) & reason ) {
        va_start(pvar, pFormat);
        nchar = pasynTrace->vprint( pasynUser, reason, pFormat, pvar );
        va_end (pvar);
    }

    return(nchar);
}
