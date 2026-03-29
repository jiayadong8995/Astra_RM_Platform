#ifndef PLATFORM_CONTROL_INTERNAL_BALANCE_PARAMS_H
#define PLATFORM_CONTROL_INTERNAL_BALANCE_PARAMS_H

#define LINK_L1     0.15f
#define LINK_L2     0.272f
#define LINK_L3     0.272f
#define LINK_L4     0.15f
#define LINK_L5     0.15f

#define WHEEL_RADIUS        0.0675f
#define WHEEL_GEAR_RATIO    15.0f

#define JOINT0_OFFSET   (-0.024f)
#define JOINT1_OFFSET   (-0.0531f)
#define JOINT2_OFFSET   (-0.018f)
#define JOINT3_OFFSET   (+0.074f)

#define THETA_OFFSET_R  (-0.05f)
#define THETA_OFFSET_L  (+0.05f)

#define LEG_LENGTH_MIN  0.14f
#define LEG_LENGTH_MAX  0.35f
#define LEG_LENGTH_DEFAULT  0.18f

#define JOINT_TORQUE_MAX    2.8f
#define WHEEL_TORQUE_MAX    2.0f

#define M3508_TORQUE_TO_CURRENT     3458.84f
#define M3508_RPM_TO_RADS           (2.0f * 3.1415926f / 60.0f / WHEEL_GEAR_RATIO)

#define WHEEL_TORQUE_RATIO  1.0f
#define TURN_TORQUE_RATIO   0.8f

#define LEG_GRAVITY_COMP    10.4f

#define LEG_PID_KP      220.0f
#define LEG_PID_KI      0.02f
#define LEG_PID_KD      4200.0f
#define LEG_PID_MAX_OUT     60.0f
#define LEG_PID_MAX_IOUT    15.0f

#define TP_PID_KP       9.0f
#define TP_PID_KI       0.0f
#define TP_PID_KD       1.6f
#define TP_PID_MAX_OUT      2.0f
#define TP_PID_MAX_IOUT     0.0f

#define TURN_PID_KP     4.0f
#define TURN_PID_KI     0.0f
#define TURN_PID_KD     0.4f
#define TURN_PID_MAX_OUT    1.0f
#define TURN_PID_MAX_IOUT   0.0f

#define ROLL_PID_KP     2.0f
#define ROLL_PID_KI     0.0f
#define ROLL_PID_KD     1.0f
#define ROLL_PID_MAX_OUT    80.0f
#define ROLL_PID_MAX_IOUT   0.0f

#define PITCH_FALL_THRESHOLD    0.15f
#define PITCH_RECOVER_THRESHOLD 0.20f

#define FN_GROUND_THRESHOLD     10.0f
#define FN_WHEEL_GRAVITY        6.0f

#define RC_VX_MAX           6.5f
#define RC_TO_VX            (RC_VX_MAX / 660.0f)
#define RC_TO_TURN          0.0002f
#define RC_SPEED_SLOPE      0.1f

#define LQR_K_MATRIX { \
    -12.4527f, -1.0044f, -12.0831f, -9.3426f, 13.4064f, 2.0690f, \
     19.2682f,  2.0679f,  26.8886f, 17.6150f, 33.3348f, 3.2053f  \
}

#endif
