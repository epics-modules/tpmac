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
#include "epicsEvent.h"
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
    15,
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
    motorAxisStop,              /**< Pointer to function to stop motion */
    motorAxisforceCallback,     /**< Pointer to function to request a poller status update */
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
    int movesDeferred;
    double movingPollPeriod;
    double idlePollPeriod;
    epicsEventId pollEventId;
} drvPmac_t;

/* Default polling periods (in milliseconds). */ 
static const int defaultMovingPollPeriod = 100;
static const int defaultIdlePollPeriod = 500;

/* These two #define affect the behaviour of the driver.

   REMOVE_LIMITS_ON_HOME removes the limits protection when homing if
    - ms??,i912 indicates you are homing onto a limit
    - ms??,i913 and the home velocity indicate that the limit you trigger.
      for home detection is the one you are homing towards.
    - any home offset is in the opposite sense to the home velocity.
   SIMULATE_SET_POSITION uses a driver level offset to simulate a PMAC command
   to set the current demand and actual to a specific position. The alternative
   is to write to M161 and M162. The consequences are:
    + Motor doesn't move (writing to M161 and M162 invariable creates a small move.
    + M161 and M162 don't need to be defined.
    - The offset doesn't survive a reboot - the motor record only calls SET_POSITION
      of the current actual is close to zero and is a long way away from the demand.
*/

#define REMOVE_LIMITS_ON_HOME
/* #define SIMULATE_SET_POSITION */

typedef struct motorAxisHandle
{
    PMACDRV_ID pDrv;
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
    double deferred_position;
    int deferred_move;
    int deferred_relative;
} motorAxis;

static PMACDRV_ID pFirstDrv = NULL;

static int drvPmacLogMsg( void * param, const motorAxisLogMask_t logMask, const char *pFormat, ...);
static motorAxisLogFunc drvPrint = drvPmacLogMsg;
static motorAxisLogFunc drvPrintParam = NULL;

#define TRACE_FLOW    motorAxisTraceFlow
#define TRACE_DRIVER  motorAxisTraceIODriver
#define TRACE_ERROR   motorAxisTraceError

#define MAX(a,b) ((a)>(b)? (a): (b))
#define MIN(a,b) ((a)<(b)? (a): (b))

static void motorAxisReportAxis( AXIS_HDL pAxis, int level )
{
    printf( "Found driver for drvPmac card %d, axis %d\n", pAxis->pDrv->card, pAxis->axis );

    if (level > 0) printf( "drvPmac->axisMutex = %p\n", pAxis->axisMutex );
#ifdef SIMULATE_SET_POSITION
    if (level > 0) printf( "Encoder offset = %f\n", pAxis->enc_offset );
#endif

    if (level > 1)
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
    	switch (function) {
    	case motorAxisDeferMoves:
    		*value = pAxis->pDrv->movesDeferred;
    		return MOTOR_AXIS_OK;
    		break;
    	default:
        	return motorParam->getInteger( pAxis->params, (paramIndex) function, value );
    	}
    }
}

static int motorAxisGetDouble( AXIS_HDL pAxis, motorAxisParam_t function, double * value )
{
    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
    	switch (function) {
    	case motorAxisDeferMoves:
    		*value = (double)pAxis->pDrv->movesDeferred;
    		return MOTOR_AXIS_OK;
    		break;
    	default:
    		return motorParam->getDouble( pAxis->params, (paramIndex) function, value );
    	}
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

    if ( !logGlobal )
        pAxis->print( pAxis->logParam, TRACE_DRIVER, "Sending to PMAC %d command : %s\n",pAxis->pDrv->card, command );
   
    status = pasynOctetSyncIO->writeRead( pasynUser,
                                          command, strlen(command),
                                          response, reply_buff_size,
                                          timeout,
                                          &nwrite, &nread, &eomReason );

    if ( !logGlobal && nread != 0 )
        pAxis->print( pAxis->logParam, TRACE_DRIVER, "Recvd from PMAC %d response: %s\n",pAxis->pDrv->card, response );

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

static int processDeferredMoves(drvPmac_t *pDrv)
{
	int status = MOTOR_AXIS_ERROR;
	int i;
	char command[512] = "";
	char response[32];

	for ( i = 0; i < pDrv->nAxes; i++ )
	{
		AXIS_HDL pAxis = &(pDrv->axis[i]);
		
		if (pAxis->deferred_move) {
			pAxis->deferred_move = 0;
			sprintf(command, "%s #%d%s%.2f", command, pAxis->axis,
					pAxis->deferred_relative ? "J^" : "J=",
					pAxis->deferred_position);
		}
	}

	status = motorAxisWriteRead( &pDrv->axis[0], command, sizeof(response), response, 0 );
	/* XXX: Set motor params? Call callback? */
	
	return status;
}

static int motorAxisSetDouble( AXIS_HDL pAxis, motorAxisParam_t function, double value )
{
    int status = MOTOR_AXIS_OK;

    if (pAxis == NULL) return MOTOR_AXIS_ERROR;
    else
    {
        char command[64]="\0";
        char response[64];

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {
            switch (function)
            {
            case motorAxisPosition:
            {
#ifdef SIMULATE_SET_POSITION
                double position, highLimit, lowLimit;

                motorParam->getDouble( pAxis->params, motorAxisPosition,  &position );
                motorParam->getDouble( pAxis->params, motorAxisHighLimit, &highLimit );
                motorParam->getDouble( pAxis->params, motorAxisLowLimit,  &lowLimit );

                pAxis->enc_offset += value - position;
                sprintf( command, "I%d13=%f I%d14=%f",
                         pAxis->axis, (highLimit - pAxis->enc_offset),
                         pAxis->axis, (lowLimit  - pAxis->enc_offset)  );

                printf( "Set PMAC card %d, axis %d to position %f, offset %f, old position %f\n",
                        pAxis->pDrv->card, pAxis->axis, value, pAxis->enc_offset, position );
#else
                int position = (int) floor(value*32 + 0.5);

                sprintf( command, "#%dK M%d61=%d*I%d08 M%d62=%d*I%d08",
                         pAxis->axis,
                         pAxis->axis, position, pAxis->axis, 
                         pAxis->axis, position, pAxis->axis );

                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Set card %d, axis %d to position %f\n",
                              pAxis->pDrv->card, pAxis->axis, value );

	            if ( command[0] != 0 && status == MOTOR_AXIS_OK)
    	        {
    	            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
    	        }
    	        
		        epicsThreadSleep( 0.1 );
    	        
    	        sprintf( command, "#%dJ/", pAxis->axis);

#endif
                break;
            }
            case motorAxisEncoderRatio:
            {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot set PMAC card %d, axis %d encoder ratio (%f)\n",
                              pAxis->pDrv->card, pAxis->axis, value );
                break;
            }
            case motorAxisLowLimit:
            {
                sprintf( command, "I%d14=%f", pAxis->axis, value );
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Setting PMAC card %d, axis %d low limit (%f)\n",
                              pAxis->pDrv->card, pAxis->axis, value );
                break;
            }
            case motorAxisHighLimit:
            {
                sprintf( command, "I%d13=%f", pAxis->axis, value );
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Setting PMAC card %d, axis %d high limit (%f)\n",
                              pAxis->pDrv->card, pAxis->axis, value );
                break;
            }
            case motorAxisPGain:
            {
                sprintf( command, "I%d30=%f", pAxis->axis, value );
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Setting PMAC card %d, axis %d pgain (%f)\n",
                              pAxis->pDrv->card, pAxis->axis, value );
                break;
            }
            case motorAxisIGain:
            {
                sprintf( command, "I%d33=%f", pAxis->axis, value );
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Setting PMAC card %d, axis %d igain (%f)\n",
                              pAxis->pDrv->card, pAxis->axis, value );
                break;
            }
            case motorAxisDGain:
            {
                sprintf( command, "I%d31=%f", pAxis->axis, value );
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Setting PMAC card %d, axis %d dgain (%f)\n",
                              pAxis->pDrv->card, pAxis->axis, value );
                break;
            }
            case motorAxisClosedLoop:
            {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot set PMAC card %d, axis %d closed loop (%s)\n",
                              pAxis->pDrv->card, pAxis->axis, (value!=0?"ON":"OFF") );
                break;
            }
            case motorAxisDeferMoves:
            {
            	pAxis->print( pAxis->logParam, TRACE_FLOW,
            	              "%sing Deferred Move flag on PMAC card %d\n",
            	              value != 0.0?"Sett":"Clear",pAxis->pDrv->card);
            	if (value == 0.0 && pAxis->pDrv->movesDeferred != 0) {
            		processDeferredMoves(pAxis->pDrv);
            	}
            	pAxis->pDrv->movesDeferred = (int)value;
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

            if (status == MOTOR_AXIS_OK )
            {
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
        if (acceleration != 0)
        {
            if (max_velocity != 0)
            {
                sprintf(acc_buff, "I%d20=%f ", pAxis->axis, (fabs(max_velocity/acceleration) * 1000.0));
            }
        }

        if (pAxis->pDrv->movesDeferred == 0) {
	#ifdef SIMULATE_SET_POSITION
	        sprintf( command, "%s%s#%d %s%.2f", vel_buff, acc_buff, pAxis->axis,
	                 (relative?"J^":"J="),
	                 (relative?position:position - pAxis->enc_offset) );
	#else
	        sprintf( command, "%s%s#%d %s%.2f", vel_buff, acc_buff, pAxis->axis,
	                 (relative?"J^":"J="), position );
	#endif
        } else { /* deferred moves */
        	sprintf( command, "%s%s", vel_buff, acc_buff);
        	pAxis->deferred_position = position;
        	pAxis->deferred_move = 1;
        	pAxis->deferred_relative = relative;
        }

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {

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
            motorParam->setInteger( pAxis->params, motorAxisDone, 0 );
            motorParam->callCallback( pAxis->params );
            epicsMutexUnlock( pAxis->axisMutex );
        }

	/* Signal the poller task.*/
	epicsEventSignal(pAxis->pDrv->pollEventId);

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
        char response[128];

        if (max_velocity != 0) sprintf(vel_buff, "I%d23=%f ", pAxis->axis, (forwards?1:-1)*(fabs(max_velocity) / 1000.0));
        if (acceleration != 0)
        {
            if (max_velocity != 0)
            {
                sprintf(acc_buff, "I%d20=%f ", pAxis->axis, (fabs(max_velocity/acceleration) * 1000.0));
            }
        }
        sprintf( command, "%s%s#%d HOME", vel_buff, acc_buff,  pAxis->axis );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {
                
#ifdef REMOVE_LIMITS_ON_HOME
            /* If homing onto an end-limit and home velocity is in the right direction, clear limits protection */
            int macro_station = ((pAxis->axis-1)/2)*4 + (pAxis->axis-1)%2;
            int home_type, home_flag, flag_mode, nvals, home_offset;
            int controller_type;
            double home_velocity;
            char buffer[128];

            /* Discover type of controller */
            strcpy( buffer, "cid" );
            status = motorAxisWriteRead( pAxis, buffer, sizeof(response), response, 0 );
	    nvals  = sscanf( response, "%d", &controller_type );
            if( controller_type == 603382 )
              printf("This is a Geobrick LV\n");
            else if( controller_type == 602413 )
              printf("This is a Turbo PMAC 2 Ultralite\n");
            else
              printf("Unknown controller type = %d\n", controller_type);

            if( controller_type == 603382 )
            {
              /* Read home flags and home direction from PMAC */ 
              if( pAxis->axis < 5 )
                sprintf( buffer, "I70%d2 I70%d3 i%d24 i%d23 i%d26", pAxis->axis, pAxis->axis, pAxis->axis, pAxis->axis, pAxis->axis );
              else
                sprintf( buffer, "I71%d2 I71%d3 i%d24 i%d23 i%d26", pAxis->axis-4, pAxis->axis-4, pAxis->axis, pAxis->axis, pAxis->axis );
              printf("%s\n", buffer);
              status = motorAxisWriteRead( pAxis, buffer, sizeof(response), response, 0 );
              nvals = sscanf( response, "%d %d $%x %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset );
              if( nvals != 5 )
                nvals = sscanf( response, "%d %d $%x %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset );
              printf("home_type = %d, home_flag = %d, flag_mode = %x, home_velocity = %f, home_offset = %d\n", home_type, home_flag, flag_mode, home_velocity, home_offset );
            }

            if( controller_type == 602413 )
            {
              /* Read home flags and home direction from PMAC */ 
              sprintf( buffer, "ms%d,i912 ms%d,i913 i%d24 i%d23 i%d26", macro_station, macro_station, pAxis->axis, pAxis->axis, pAxis->axis );
              status = motorAxisWriteRead( pAxis, buffer, sizeof(response), response, 0 );
              nvals = sscanf( response, "$%x $%x $%x %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset );
	      if (nvals !=5)
	        nvals = sscanf( response, "%d %d %d %lf %d", &home_type, &home_flag, &flag_mode, &home_velocity, &home_offset );
            }

            if (max_velocity != 0) home_velocity = (forwards?1:-1)*(fabs(max_velocity) / 1000.0);

            printf("status = %d, nvals = %d\n", status, nvals);
            if ( status || nvals != 5)
            {
                pAxis->print( pAxis->logParam, TRACE_ERROR,
	                      "drvPmac motorAxisHome: can not read home flags. Status: %d, nvals %d\n", status, nvals );
            }
            else if ( ( home_type <= 15 )      && 
                      ( home_type % 4 >= 2 )   &&
                      !( flag_mode & 0x20000 ) &&
                      (( home_velocity > 0 && home_flag == 1 && home_offset <= 0 ) || 
                       ( home_velocity < 0 && home_flag == 2 && home_offset >= 0 )    )   )
            {
              sprintf( buffer, " i%d24=i%d24|$20000", pAxis->axis, pAxis->axis );
              strcat( command, buffer );
              pAxis->limitsDisabled = 1;
              pAxis->print( pAxis->logParam, TRACE_FLOW,
                            "Disabling limits whilst homing PMAC card %d, axis %d, type:%d, flag:$%x, vel:%f\n",
                            pAxis->pDrv->card, pAxis->axis, home_type, home_flag, home_velocity );
            }
            else
            {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot disable limits to home PMAC card %d, axis %d, type:%x, flag:$%d, vel:%f, mode:0x%x, offset: %d\n",
                              pAxis->pDrv->card, pAxis->axis, home_type, home_flag, home_velocity, flag_mode, home_offset );
            }
#endif
            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            motorParam->setInteger( pAxis->params, motorAxisDone, 0 );
            motorParam->callCallback( pAxis->params );
            epicsMutexUnlock( pAxis->axisMutex );
        }
	/* Signal the poller task.*/
	epicsEventSignal(pAxis->pDrv->pollEventId);
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
        if (acceleration != 0)
        {
            if (velocity != 0)
            {
                sprintf(acc_buff, "I%d20=%f ", pAxis->axis, (fabs(velocity/acceleration) * 1000.0));
            }
        }

        sprintf( command, "%s%s#%d %s", vel_buff, acc_buff, pAxis->axis, (velocity < 0 ? "J-": "J+") );

        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK)
        {
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
            motorParam->setInteger( pAxis->params, motorAxisDone, 0 );
            motorParam->callCallback( pAxis->params );
            epicsMutexUnlock( pAxis->axisMutex );
        }
	/* Signal the poller task.*/
	epicsEventSignal(pAxis->pDrv->pollEventId);
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

        /* We now don't touch I19 and leave it as a safety limit */
        /* if (acceleration != 0) sprintf(acc_buff, "I%d19=%f ", pAxis->axis, (fabs(acceleration) / 1000000.0)); */

        sprintf( command, "%s#%d J/",  acc_buff, pAxis->axis );
        pAxis->deferred_move = 0;

        status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
    }
    return status;
}

static int motorAxisforceCallback(AXIS_HDL pAxis)
{
	if (pAxis == NULL)
		return (MOTOR_AXIS_ERROR);

	pAxis->print(pAxis->logParam, TRACE_FLOW, "motorAxisforceCallback: request card %d, axis %d status update\n",
	             pAxis->pDrv->card, pAxis->axis);

	motorParam->forceCallback(pAxis->params);

	return (MOTOR_AXIS_OK);
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
    int cmdStatus, done;
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
                      "drvPmacAxisGetStatus: not all status values returned. Status: %d\nCommand :%s\nResponse:%s",
                      cmdStatus, command, response );
        }
        else
        {
            int homeSignal = ((status[1] & PMAC_STATUS2_HOME_COMPLETE) != 0);
            int direction;

#ifdef SIMULATE_SET_POSITION
            if ( homeSignal && !pAxis->homeSignal )
            {
                double highLimit, lowLimit;

                /* Just finished homing - remove encoder offset */
                motorParam->getDouble( pAxis->params, motorAxisHighLimit, &highLimit );
                motorParam->getDouble( pAxis->params, motorAxisLowLimit,  &lowLimit );

                pAxis->enc_offset = 0;
                sprintf( command, "I%d13=%f I%d14=%f",
                         pAxis->axis, highLimit, pAxis->axis, lowLimit );
                cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            }
            pAxis->homeSignal = homeSignal;

            motorParam->setDouble(  pAxis->params, motorAxisPosition,      (position+error+pAxis->enc_offset) );
#else
            motorParam->setDouble(  pAxis->params, motorAxisPosition,      (position+error) );
#endif
            motorParam->setDouble(  pAxis->params, motorAxisEncoderPosn,   position );

            /* Don't set direction if velocity equals zero and was previously negative */
            motorParam->getInteger( pAxis->params, motorAxisDirection,     &direction );
            motorParam->setInteger( pAxis->params, motorAxisDirection,     ((velocity >= 0) || (velocity == 0 && direction)) );
            if(pAxis->deferred_move) {
                done = 0; 
            } else {
                done = ((status[1] & PMAC_STATUS2_IN_POSITION) != 0);
            }
            motorParam->setInteger( pAxis->params, motorAxisDone,          done );
            motorParam->setInteger( pAxis->params, motorAxisHighHardLimit, ((status[0] & PMAC_STATUS1_POS_LIMIT_SET) != 0) );
            motorParam->setInteger( pAxis->params, motorAxisHomeSignal,    homeSignal );
            motorParam->setInteger( pAxis->params, motorAxisMoving,        ((status[0] & PMAC_STATUS1_DESIRED_VELOCITY_ZERO) == 0) );
            motorParam->setInteger( pAxis->params, motorAxisLowHardLimit,  ((status[0] & PMAC_STATUS1_NEG_LIMIT_SET)!=0) );
            motorParam->callCallback( pAxis->params );           
        }

#ifdef REMOVE_LIMITS_ON_HOME
        if ( pAxis->limitsDisabled && (status[1] & PMAC_STATUS2_HOME_COMPLETE) && (status[0] & PMAC_STATUS1_DESIRED_VELOCITY_ZERO) )
        {
            /* Re-enable limits */
            sprintf( command, "i%d24=i%d24&$FDFFFF", pAxis->axis, pAxis->axis );
            cmdStatus = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
            pAxis->limitsDisabled = (cmdStatus != 0);
        }
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

static void drvPmacTask( PMACDRV_ID pDrv )
{
  int i = 0;
  int done = 0;
  int cachedDone = 1;
  int eventStatus = 0;
  int forcedFastPolls = 0;
  float timeout = 0;

  while ( 1 ) 
  {
    cachedDone = 1;

    /* Wait for an event, or a timeout. If we get an event, do a series of fast polls.*/
    eventStatus = epicsEventWaitWithTimeout(pDrv->pollEventId, timeout);
    if (eventStatus == epicsEventWaitOK) {
      forcedFastPolls = 10;
    }

    for ( i = 0; i < pDrv->nAxes; i++ )
    {
	AXIS_HDL pAxis = &(pDrv->axis[i]);		
	motorParam->getInteger( pAxis->params, motorAxisDone, &done );
	cachedDone &= done;
	drvPmacGetAxisStatus( pAxis, pDrv->pasynUser );
    }

    if (forcedFastPolls > 0) {
      timeout = pDrv->movingPollPeriod;
      forcedFastPolls--;
    } else if (!cachedDone) {
      timeout = pDrv->movingPollPeriod;
    } else {
      timeout = pDrv->idlePollPeriod;
    }
  }
}

/**
 * Set up motor controller on an Asyn port and address.
 * @param port Name of ASYN port
 * @param addr Address of ASYN port
 * @param card Numerical ID for controller
 * @param nAxes Number of axes on this controller
 * @return status
 */
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
		/* Set default polling rates.*/
		pDrv->movingPollPeriod = (double)defaultMovingPollPeriod / 1000.0;
		pDrv->idlePollPeriod = (double)defaultIdlePollPeriod / 1000.0;
		/* Create event to signal poller task with.*/
		pDrv->pollEventId = epicsEventMustCreate(epicsEventEmpty);

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
                        motorAxisWriteRead( &(pDrv->axis[i]), command, sizeof(reply), reply, 1 );
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
        int i;

        /* Do an initial poll of all status */
	for ( i = 0; i < pDrv->nAxes; i++ )
	{
            AXIS_HDL pAxis = &(pDrv->axis[i]);

            drvPmacGetAxisInitialStatus( pAxis, pDrv->pasynUser );
            drvPmacGetAxisStatus( pAxis, pDrv->pasynUser );
        }

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


/**
 * Function to set the movingPollPeriod time to use when polling 
 * the controller during a move.
 * @param card Numerical ID of the controller.
 * @param movingPollPeriod The period in miliseconds.
 * @return status
 */
int pmacSetMovingPollPeriod(int card, int movingPollPeriod)
{
  int status = 1;
  PMACDRV_ID pDrv = NULL;

  if (pFirstDrv != NULL) {
    for (pDrv = pFirstDrv; pDrv != NULL; pDrv = pDrv->pNext) {
      drvPrint(drvPrintParam, TRACE_FLOW, "** Setting moving poll period of %dms on card %d\n", movingPollPeriod, pDrv->card);
      if (card == pDrv->card) {
	pDrv->movingPollPeriod = (double)movingPollPeriod / 1000.0;
	status = 0;
      }
    }
    drvPrint(drvPrintParam, TRACE_FLOW, "** Moving poll period set to %fs\n.", pDrv->movingPollPeriod);
  }

  return status;
}


/**
 * Function to set the idlePollPeriod time to use when polling 
 * the controller when there is no motion.
 * @param card Numerical ID of the controller.
 * @param idlePollPeriod The period in miliseconds.
 * @return status
 */
int pmacSetIdlePollPeriod(int card, int idlePollPeriod)
{
  int status = 1;
  PMACDRV_ID pDrv = NULL;

  if (pFirstDrv != NULL) {
    for (pDrv = pFirstDrv; pDrv!=NULL; pDrv = pDrv->pNext) {
      drvPrint(drvPrintParam, TRACE_FLOW, "** Setting idle poll period of %dms on card %d\n", idlePollPeriod, pDrv->card);
      if (card == pDrv->card) {
	pDrv->idlePollPeriod = (double)idlePollPeriod / 1000.0;
	status = 0;
      }
    }
    drvPrint(drvPrintParam, TRACE_FLOW, "** Idle poll period set to %fs\n.", pDrv->idlePollPeriod);
  }

  return status;
}

