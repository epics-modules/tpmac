/**************************************************************************
 * pmacAsynCoord driver
 * 
 * This is a driver for Delta-Tau PMAC Coordinate Systems,
 * designed to interface with the "motor API" style interface that
 * allows drivers to be used with the standard asyn device and driver
 * support for the motor record
 * 
 * Peter Denison, Diamond Light Source
 **************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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

motorAxisDrvSET_t pmacAsynCoord = {
    15,
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
    motorAxisStop,              /**< Pointer to function to stop motion */
    motorAxisforceCallback,     /**< Pointer to function to request a poller status update */
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
    int ref;
    int cs;
    AXIS_HDL axis;
    epicsThreadId motorThread;
    epicsTimeStamp now;
    int movesDeferred;
    double movingPollPeriod;
    double idlePollPeriod;
    epicsEventId pollEventId;
    epicsMutexId controllerMutexId;    
} drvPmac_t;

/* Default polling periods (in milliseconds). */ 
static const int defaultMovingPollPeriod = 100;
static const int defaultIdlePollPeriod = 500;

typedef struct motorAxisHandle
{
    PMACDRV_ID pDrv;
    int coord_system;
    int axis;	/*> 1-based index into the coordinate system */
    int	program;
    asynUser * pasynUser;
    PARAMS params;
    motorAxisLogFunc print;
    void * logParam;
    epicsMutexId axisMutex;
    int deferred_move;
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
#define ALL_DEMAND "Q7%d..7%d"
/* Use Q81 - Q89 for motor readback positions */
#define READBACK "Q8%d"
#define ALL_READBACK "Q8%d..8%d"
/* Use 91 - 99 for motor deadbands */
#define DEADBAND "Q9%d"
#define ALL_DEADBAND "Q9%d..9%d"


static void motorAxisReportAxis( AXIS_HDL pAxis, int level )
{
	printf("axis %c:\n", NAME(pAxis));
    if (level > 0) printf( "drvPmac->axisMutex = %p\n", pAxis->axisMutex );

    if (level > 1) {
        motorParam->dump( pAxis->params );
    }
}

static void motorAxisReport( int level )
{
    PMACDRV_ID pDrv;

    for (pDrv = pFirstDrv; pDrv != NULL; pDrv = pDrv->pNext) {
        int i;
        const char *name;

        pasynManager->getPortName(pDrv->pasynUser, &name);
        printf( "Found driver for PMAC C.S. %d via %s (ref %d)\n", pDrv->cs, name, pDrv->ref );
        for ( i = 0; i < NAXES; i++ ) {
        	if (level > 0) {
        		motorAxisReportAxis( &(pDrv->axis[i]), level );
        	}
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

static AXIS_HDL motorAxisOpen( int ref, int axis, char * param )
{
    PMACDRV_ID pDrv;
    AXIS_HDL pAxis = NULL;

    for ( pDrv=pFirstDrv; pDrv != NULL && (ref != pDrv->ref); pDrv = pDrv->pNext){}

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
    if (pAxis == NULL) {
    	return MOTOR_AXIS_ERROR;
    } else {
        switch(function) {
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
        pAxis->print( pAxis->logParam, TRACE_DRIVER, "Sending to PMAC C.S. ref %d command : %s\n",pAxis->pDrv->ref, command );
    }
        
    status = pasynOctetSyncIO->writeRead( pasynUser,
                                          command, strlen(command),
                                          response, reply_buff_size,
                                          timeout,
                                          &nwrite, &nread, &eomReason );

    if ( !logGlobal && nread != 0 ) {
        pAxis->print( pAxis->logParam, TRACE_DRIVER, "Recvd from PMAC C.S. ref %d response: %s\n",pAxis->pDrv->ref, response );
    }
        
    if (status) {
        motorParam->setInteger( pAxis->params, motorAxisCommError, 1 );
        asynPrint( pasynUser,
                   ASYN_TRACE_ERROR,
                   "Read/write error to PMAC C.S. ref %d, axis %c command %s. Status=%d, Error=%s\n",
                   pAxis->pDrv->ref, NAME(pAxis), command,
                   status, pasynUser->errorMessage);
        return MOTOR_AXIS_ERROR;
    }

    motorParam->setInteger( pAxis->params, motorAxisCommError, 0 );
    return MOTOR_AXIS_OK;
}

static int processDeferredMoves(drvPmac_t *pDrv)
{
    int i;
    int status = MOTOR_AXIS_ERROR;
    char command[128], response[32];
    
    sprintf(command, "&%dB%dR", pDrv->cs, pDrv->axis[0].program);

    for (i = 0; i < NAXES; i++) {
        pDrv->axis[i].deferred_move = 0;
    }
    
    status = motorAxisWriteRead(&pDrv->axis[0], command, sizeof(response), response, 0);
    return status;
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
                int position = (int) floor(value + 0.5);
                /* Set the demand values. We don't need to do anything clever 
                 * here as a move command will set reasonable demand values for 
                 * all axis in CS */
                sprintf(command, "&%d"DEMAND"=%d",pAxis->coord_system, pAxis->axis, position);
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Set ref %d, axis %c to position %f\n",
                              pAxis->pDrv->ref, NAME(pAxis), value );
                break;
            }
            case motorAxisEncoderRatio: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot set PMAC ref %d, cs %d, axis %c encoder ratio (%f)\n",
                              pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisLowLimit: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC ref %d, cs %d, axis %c low limit (%f)\n",
                              pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisHighLimit: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC ref %d, cs %d, axis %c high limit (%f)\n",
                              pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisPGain: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC ref %d, cs %d, axis %c pgain (%f)\n",
                              pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisIGain: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC ref %d, cs %d, axis %c igain (%f)\n",
                              pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisDGain: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Ignoring PMAC ref %d, cs %d, axis %c dgain (%f)\n",
                              pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis), value );
                break;
            }
            case motorAxisClosedLoop: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "Cannot set PMAC ref %d, axis %c closed loop (%s)\n",
                              pAxis->pDrv->ref, NAME(pAxis), (value!=0?"ON":"OFF") );
                break;
            }
            case motorAxisDeferMoves: {
                pAxis->print( pAxis->logParam, TRACE_FLOW,
                              "%sing Deferred Move flag on PMAC ref %d, cs %d\n",
                              value != 0.0?"Sett":"Clear", pAxis->pDrv->ref, pAxis->coord_system);
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
	AXIS_HDL piAxis, first_axis, last_axis;
	
    if (pAxis != NULL) {
        char acc_buff[32]="";
        char vel_buff[32]="";
        char buff[128];
        char command[256];
	char commandtemp[128];
        char response[256];
	    char *response_parse = response;        
        char responsedb[256];
	    char *responsedb_parse = responsedb;        
	    
        double pos, dpos, dband;
        int i;

        if (max_velocity != 0) {
        	sprintf(vel_buff, "I%d89=%f ", CSVAR(pAxis), max_velocity);
        }
        if (acceleration != 0) {
            if (max_velocity != 0) {
                sprintf(acc_buff, "I%d87=%f ", CSVAR(pAxis),
                		(fabs(max_velocity/acceleration) * 1000.0));
            }
        }
        sprintf( command, "&%d%s%s"DEMAND"=%.2f", pAxis->coord_system, vel_buff, acc_buff, pAxis->axis, position );             

        if (pAxis->pDrv->movesDeferred) {
            pAxis->deferred_move = 1;
        }
        else if (pAxis->program != 0) {
	        /* If we aren't deferring moves and there is a program number, we want to
	         * make sure that all demand values are what they should be according to
	         * the relevant motor record */
		    first_axis = &(pAxis->pDrv->axis[0]);
		    last_axis = &(pAxis->pDrv->axis[NAXES-1]);
         
		    /* Read all the demands for this co-ordinate system in one go */
		    sprintf( buff, "&%d", first_axis->coord_system);
		    for (i = first_axis->axis; i <= last_axis->axis; i++) {
		      sprintf( commandtemp, DEMAND, i);
		      strcat(buff, commandtemp);
		    }
		    motorAxisWriteRead( first_axis, buff, sizeof(response), response, 1 );

		    /* Read all the deadbands for this co-ordinate system in one go */	
		    sprintf( buff, "&%d", first_axis->coord_system);
		    for (i = first_axis->axis; i <= last_axis->axis; i++) {
		      sprintf( commandtemp, DEADBAND, i);
		      strcat(buff, commandtemp);
		    }
		   
		    motorAxisWriteRead( first_axis, buff, sizeof(responsedb), responsedb, 1 );

		    for (i = 0; i < NAXES; i++) {
				piAxis = &(pAxis->pDrv->axis[i]);
           		dpos = strtod(response_parse, &response_parse);
           		dband = strtod(responsedb_parse, &responsedb_parse);           		
			if (piAxis->axis!=pAxis->axis) {
	            	motorParam->getDouble(  piAxis->params, motorAxisPosition, &pos );
            		if (abs(dpos-pos)>dband) {
				        sprintf( buff, " "DEMAND"=%.2f", piAxis->axis, pos );  	
				        strcat( command, buff);
				    }
			    }
		    }
	        /* If the program specified is non-zero, add a command to run the program.
	         * If program number is zero, then the move will have to be started by some
	         * external process, which is a mechanism of allowing coordinated starts to
	         * movement. */
        	sprintf(buff, "B%dR", pAxis->program);
		    strcat(command, buff);
	    }
        if (epicsMutexLock( pAxis->axisMutex ) == epicsMutexLockOK) {
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

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
/*        char response[128];*/

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
/*            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );*/
		    asynPrint(pAxis->pasynUser, ASYN_TRACE_ERROR, "motorAxisHome: not implemented for CS axes\n");
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

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char vel_buff[32]="\0";
        char command[128];
/*        char response[32];*/

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
/*            status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );*/
		    asynPrint(pAxis->pasynUser, ASYN_TRACE_ERROR, "motorAxisVelocityMove: not implemented for CS axes\n");
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

    if (pAxis != NULL) {
        char acc_buff[32]="\0";
        char command[128];
        char response[32];

        sprintf( command, "&%d%sA "DEMAND"="READBACK, pAxis->coord_system, acc_buff,
                 pAxis->axis, pAxis->axis);
        pAxis->deferred_move = 0;
        
        status = motorAxisWriteRead( pAxis, command, sizeof(response), response, 0 );
    }
    return status;
}

static int motorAxisforceCallback(AXIS_HDL pAxis)
{
       if (pAxis == NULL)
               return (MOTOR_AXIS_ERROR);

       pAxis->print(pAxis->logParam, TRACE_FLOW, "motorAxisforceCallback: request ref %d, cs %d, axis %c status update\n",
                    pAxis->pDrv->ref, pAxis->coord_system, NAME(pAxis));

       motorParam->forceCallback(pAxis->params);

       return (MOTOR_AXIS_OK);
}

static int drvPmacGetCoordStatus(AXIS_HDL pAxis, asynUser *pasynUser,
				 epicsUInt32 *status)
{
    char command[32];
    char response[128]="";
    int nvals, cmdStatus;
    int retval = 0;

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
    return retval;
}

static void drvPmacGetAxesStatus( PMACDRV_ID pDrv, epicsUInt32 *status)
{
  char command[128];
    char commandtemp[128];
    char pos_response[128];
    char *pos_parse = pos_response;
    int cmdStatus, done;
    double position, oldposition, error;
    int i;
    asynUser *pasynUser = pDrv->pasynUser;
    AXIS_HDL first_axis, last_axis, pAxis;
    int direction;
	int homeSignal;

	/* First lock all the axes */
    for (i = 0; i < NAXES; i++) {
    	pAxis = &pDrv->axis[i];
		epicsMutexLock( pAxis->axisMutex );
	}
	
    /* As yet, there is no way to get the following error of a C.S. axis - set it to zero for now */
    error = 0;
    first_axis = &pDrv->axis[0];
    last_axis = &pDrv->axis[NAXES-1];
    
    /* Read all the positions for this co-ordinate system in one go */
    sprintf( command, "&%d", first_axis->coord_system);
    for (i = first_axis->axis; i <= last_axis->axis; i++) {
      sprintf( commandtemp, READBACK, i);
      strcat(command, commandtemp);
    }

    cmdStatus = motorAxisWriteRead( first_axis, command, sizeof(pos_response), pos_response, 1 );

    /* Get the co-ordinate system status */
    drvPmacGetCoordStatus(first_axis, pDrv->pasynUser, status);

    for (i = 0; i < NAXES; i++) {
    	pAxis = &pDrv->axis[i];
/*            int homeSignal = ((status[1] & CS_STATUS2_HOME_COMPLETE) != 0);*/
/* TODO: possibly look at the aggregate of the home status of all motors in the c.s ?? */
        homeSignal = 0;
        errno = 0;
        position = strtod(pos_parse, &pos_parse);
        if (position == 0 && errno != 0) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR,
                        "drvPmacAxisGetStatus: not all status values returned. Status: %d\nCommand :%s\nResponse1:%s\n",
                        cmdStatus, command, pos_response );
        }
        motorParam->getDouble(  pAxis->params, motorAxisPosition,      &oldposition );                      
        motorParam->setDouble(  pAxis->params, motorAxisPosition,      (position+error) );
        motorParam->setDouble(  pAxis->params, motorAxisEncoderPosn,   position );
        /* Don't set direction if velocity equals zero and was previously negative */
        motorParam->getInteger( pAxis->params, motorAxisDirection,     &direction );
        motorParam->setInteger( pAxis->params, motorAxisDirection,     ((position > oldposition) || (position == oldposition && direction)) );
        motorParam->setInteger( pAxis->params, motorAxisHighHardLimit, ((status[2] & CS_STATUS3_LIMIT) != 0) );
        motorParam->setInteger( pAxis->params, motorAxisHomeSignal,    homeSignal );
        motorParam->setInteger( pAxis->params, motorAxisMoving,        ((status[1] & CS_STATUS2_IN_POSITION) == 0) );
        if (pAxis->deferred_move) {
            done = 0;
        } else {
            done = ((status[0] & CS_STATUS1_RUNNING_PROG) == 0)&&((status[1] & CS_STATUS2_IN_POSITION) != 0);
        }
        motorParam->setInteger( pAxis->params, motorAxisDone,          done);            
        motorParam->setInteger( pAxis->params, motorAxisLowHardLimit,  ((status[2] & CS_STATUS3_LIMIT)!=0) );
	/*Deal with error bits*/
	motorParam->setInteger( pAxis->params, motorAxisFollowingError,((status[1] & CS_STATUS2_FOLLOW_ERR)!=0) );
	motorParam->setInteger( pAxis->params, motorAxisProblem, ((status[1] & CS_STATUS2_AMP_FAULT) != 0) );
	motorParam->setInteger( pAxis->params, motorAxisProblem, ((status[1] & CS_STATUS2_RUNTIME_ERR) != 0) );
        motorParam->callCallback( pAxis->params );           
    }
	
	/* Now unlock all the axes */
    for (i = 0; i < NAXES; i++) {
    	pAxis = &pDrv->axis[i];
		epicsMutexUnlock( pAxis->axisMutex );
	}	
}

static void drvPmacGetAxisInitialStatus( AXIS_HDL pAxis, asynUser * pasynUser )
{
/*    char command[32];
    char response[128];
    int cmdStatus;
    double low_limit, high_limit, pgain, igain, dgain;
    int nvals;*/

    return;
}



static void drvPmacTask( PMACDRV_ID pDrv )
{
  int i = 0;
  int done = 0;
  int eventStatus = 0;
  float timeout = 0.0;
  epicsUInt32 status[3];

  while ( 1 ) 
  {
	/* if it's time for an idle poll or the motor is moving */
	drvPmacGetAxesStatus( pDrv, status);    
    /* Wait for an event, or a timeout. If we get an event, force an update.*/
    if (epicsMutexLock(pDrv->controllerMutexId) == epicsMutexLockOK) {
      timeout = pDrv->idlePollPeriod;
    }
    else {
      drvPrint(drvPrintParam, TRACE_ERROR, "drvPmacTask: Failed to get controllerMutexId lock.\n");
    }
    /* Get axis status */
    for ( i = 0; i < NAXES; i++ )
    {
      AXIS_HDL pAxis = &(pDrv->axis[i]);
      /* get the cached done status */
      epicsMutexLock( pAxis->axisMutex );
      motorParam->getInteger( pAxis->params, motorAxisDone, &done );
      epicsMutexUnlock( pAxis->axisMutex );
      if ((eventStatus == epicsEventWaitOK) || (done == 0))
      {
      	/* If we got an event, then one motor is moving, so force an update for all */      
        timeout = pDrv->movingPollPeriod;      	
        break;
      }
    }
    epicsMutexUnlock(pDrv->controllerMutexId);
    eventStatus = epicsEventWaitWithTimeout(pDrv->pollEventId, timeout);    
  }
}


/**
 * Test if a connected pasynUser matches a given port and address
 * 
 */
int asynMatch(asynUser *pasynUser, char *checkport, int checkaddr)
{
	const char *port;
	int addr;
	
    pasynManager->getPortName(pasynUser, &port);
    pasynManager->getAddr(pasynUser, &addr);
    if (strncmp(port, checkport, strlen(port)) != 0 || addr != checkaddr) {
    	return 0;
    }
    return 1;
}

/**
 * Create a driver instance to communicate with a given coordiante system
 * 
 * @param port The Asyn port used to communicate with the PMAC card
 * @param addr The Asyn address of the PMAC (usually 0)
 * @param cs The co-ordinate system to connect to
 * @param ref A unique reference, used by the higher layer software to reference this C.S.
 * @param program The PMAC program number to run to move the C.S.
 */ 
int pmacAsynCoordCreate( char *port, int addr, int cs, int ref, int program )
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
          pDrv != NULL && (pDrv->ref != ref) && (!asynMatch(pDrv->pasynUser, port, addr) || pDrv->cs != cs);
          pDrv = pDrv->pNext )
    {
        ppLast = &(pDrv->pNext);
    }

    if ( pDrv == NULL) {
        drvPrint( drvPrintParam, TRACE_FLOW,
               "Creating PMAC co-ordinate system driver on port %s, address %d, cs: %d (ref = %d)\n",
               port, addr, cs, ref );

        pDrv = (PMACDRV_ID) calloc( 1, sizeof(drvPmac_t) );

        if (pDrv != NULL) {
            pDrv->axis = (AXIS_HDL) calloc( NAXES, sizeof( motorAxis ) );

            if (pDrv->axis != NULL ) {
                pDrv->cs = cs;
                pDrv->ref = ref;
		/* Set default polling rates.*/
		pDrv->movingPollPeriod = (double)defaultMovingPollPeriod / 1000.0;
		pDrv->idlePollPeriod = (double)defaultIdlePollPeriod / 1000.0;
		/* Create event to signal poller task with.*/
		pDrv->pollEventId = epicsEventMustCreate(epicsEventEmpty);
		/* Create mutex ID for controller.*/
		if ((pDrv->controllerMutexId = epicsMutexCreate()) == NULL) {
		  drvPrint( drvPrintParam, TRACE_ERROR, "pmacAsynMotorCreate: Could not create controllerMutexId.\n");
		}

                status = motorAxisAsynConnect( port, addr, &(pDrv->pasynUser), "\006", "\r" );

                for (i=0; i<NAXES && status == MOTOR_AXIS_OK; i++ ) {
                    if ((pDrv->axis[i].params = motorParam->create( 0, MOTOR_AXIS_NUM_PARAMS )) != NULL &&
                        (pDrv->axis[i].axisMutex = epicsMutexCreate( )) != NULL ) {
                        pDrv->axis[i].pDrv = pDrv;
                        pDrv->axis[i].coord_system = cs;
                        pDrv->axis[i].program = program;
                        pDrv->axis[i].axis = i + 1;
                        pDrv->axis[i].logParam  = pDrv->pasynUser;
                        pDrv->axis[i].pasynUser = pDrv->pasynUser;

                        asynPrint( pDrv->pasynUser, ASYN_TRACE_FLOW, "Created motor for ref %d, C.S. %d, signal %d OK\n", ref, cs, i );
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
        drvPrint( drvPrintParam, TRACE_ERROR, "C.S. reference %d already exists\n", ref );
        status = MOTOR_AXIS_ERROR;
    }

    if (status == MOTOR_AXIS_OK) {
        epicsUInt32 status[3];

        /* Do an initial poll of all status */
        drvPmacGetAxesStatus( pDrv, status );
		
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


/**
 * Function to set the movingPollPeriod time to use when polling 
 * the controller during a move.
 * @param ref Numerical ID of the coordinate system.
 * @param movingPollPeriod The period in miliseconds.
 * @return status
 */
int pmacSetCoordMovingPollPeriod(int ref, int movingPollPeriod)
{
  int status = 1;
  PMACDRV_ID pDrv = NULL;

  if (pFirstDrv != NULL) {
    for (pDrv = pFirstDrv; pDrv != NULL; pDrv = pDrv->pNext) {
      drvPrint(drvPrintParam, TRACE_FLOW, "** Setting moving poll period of %dms on ref %d\n", movingPollPeriod, pDrv->ref);
      if (ref == pDrv->ref) {
        if (epicsMutexLock( pDrv->controllerMutexId ) == epicsMutexLockOK) {
	      pDrv->movingPollPeriod = (double)movingPollPeriod / 1000.0;
	      epicsMutexUnlock( pDrv->controllerMutexId );
	      status = 0;
	    } else {
	      drvPrint(drvPrintParam, TRACE_ERROR, "pmacSetCoordMovingPollPeriod: could not access pDrv to set polling period.\n" );
        } 
      }
    }
  }

  return status;
}


/**
 * Function to set the idlePollPeriod time to use when polling 
 * the controller when there is no motion.
 * @param ref Numerical ID of the coordinate system.
 * @param idlePollPeriod The period in miliseconds.
 * @return status
 */
int pmacSetCoordIdlePollPeriod(int ref, int idlePollPeriod)
{
  int status = 1;
  PMACDRV_ID pDrv = NULL;

  if (pFirstDrv != NULL) {
    for (pDrv = pFirstDrv; pDrv!=NULL; pDrv = pDrv->pNext) {
      drvPrint(drvPrintParam, TRACE_FLOW, "** Setting idle poll period of %dms on ref %d\n", idlePollPeriod, pDrv->ref);
      if (ref == pDrv->ref) {
    	if (epicsMutexLock( pDrv->controllerMutexId ) == epicsMutexLockOK) {
    	pDrv->idlePollPeriod = (double)idlePollPeriod / 1000.0;
    	epicsMutexUnlock( pDrv->controllerMutexId );
    	status = 0;
    	} else {
    	    drvPrint(drvPrintParam, TRACE_ERROR, "pmacSetMovingPollPeriod: could not access pDrv to set polling period.\n" );
    	}
      }
    }
  }

  return status;
}
