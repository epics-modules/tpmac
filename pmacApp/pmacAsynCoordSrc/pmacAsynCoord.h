#ifndef PMACASYNCOORD_H
#define PMACASYNCOORD_H

#ifdef __cplusplus
extern "C" {
#endif

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
#define CS_STATUS2_ROTARY_BUFF          (0x1<<16)
#define CS_STATUS2_IN_POSITION          (0x1<<17)
#define CS_STATUS2_FOLLOW_WARN          (0x1<<18)
#define CS_STATUS2_FOLLOW_ERR          (0x1<<19)
#define CS_STATUS2_AMP_FAULT          (0x1<<20)
#define CS_STATUS2_MOVE_IN_STACK      (0x1<<21)
#define CS_STATUS2_RUNTIME_ERR          (0x1<<22)
#define CS_STATUS2_LOOKAHEAD          (0x1<<23)

#define CS_STATUS3_LIMIT          (0x1<<1)

int pmacAsynCoordCreate( char *port, int addr, int cs, int ref, int program );
int pmacSetCoordMovingPollPeriod(int cs, int movingPollPeriod);
int pmacSetCoordIdlePollPeriod(int cs, int idlePollPeriod);
int pmacSetCoordStepsPerUnit(int ref, int axis, double stepsPerUnit);
int pmacSetDefaultCoordSteps(double defaultSteps);

#ifdef __cplusplus
}
#endif
#endif
