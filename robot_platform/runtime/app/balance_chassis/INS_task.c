/**
  *********************************************************************
  * @file      ins_task.c/h
  * @brief     该任务是用mahony方法获取机体姿态，同时获取机体在绝对坐标系下的运动加速度
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "ins_task.h"
#include "controller.h"
#include "QuaternionEKF.h"
#include "bsp_PWM.h"
#include "mahony_filter.h"
#include "message_center.h"
#include "robot_def.h"

static INS_t ins_runtime;
static Publisher_t *ins_pub;

struct MAHONY_FILTER_t mahony;
Axis3f Gyro,Accel;
float gravity[3] = {0, 0, 9.81f};

uint32_t INS_DWT_Count = 0;
float ins_dt = 0.0f;
float ins_time;
int stop_time;

void INS_Init(void)
{ 
   mahony_init(&mahony,1.0f,0.0f,0.001f);
   ins_runtime.AccelLPF = 0.0089f;
}

void INS_task(void)
{
	 INS_Init();
	 ins_pub = PubRegister("ins_data", sizeof(INS_Data_t));
	 
	 while(1)
	 {  
		ins_dt = DWT_GetDeltaT(&INS_DWT_Count);
    
		mahony.dt = ins_dt;

    BMI088_Read(&BMI088);

    ins_runtime.Accel[X] = BMI088.Accel[X];
    ins_runtime.Accel[Y] = BMI088.Accel[Y];
    ins_runtime.Accel[Z] = BMI088.Accel[Z];
	Accel.x=BMI088.Accel[0];
	Accel.y=BMI088.Accel[1];
	Accel.z=BMI088.Accel[2];
    ins_runtime.Gyro[X] = BMI088.Gyro[X];
    ins_runtime.Gyro[Y] = BMI088.Gyro[Y];
    ins_runtime.Gyro[Z] = BMI088.Gyro[Z];
  	Gyro.x=BMI088.Gyro[0];
	Gyro.y=BMI088.Gyro[1];
	Gyro.z=BMI088.Gyro[2];

	mahony_input(&mahony,Gyro,Accel);
	mahony_update(&mahony);
	mahony_output(&mahony);
	RotationMatrix_update(&mahony);
				
	ins_runtime.q[0]=mahony.q0;
	ins_runtime.q[1]=mahony.q1;
	ins_runtime.q[2]=mahony.q2;
	ins_runtime.q[3]=mahony.q3;

      // 将重力从导航坐标系n转换到机体系b,随后根据加速度计数据计算运动加速度
	float gravity_b[3];
    EarthFrameToBodyFrame(gravity, gravity_b, ins_runtime.q);
    for (uint8_t i = 0; i < 3; i++) // 同样过一个低通滤波
    {
      ins_runtime.MotionAccel_b[i] = (ins_runtime.Accel[i] - gravity_b[i]) * ins_dt / (ins_runtime.AccelLPF + ins_dt) 
														+ ins_runtime.MotionAccel_b[i] * ins_runtime.AccelLPF / (ins_runtime.AccelLPF + ins_dt); 
//			ins_runtime.MotionAccel_b[i] = (ins_runtime.Accel[i] ) * dt / (ins_runtime.AccelLPF + dt) 
//														+ ins_runtime.MotionAccel_b[i] * ins_runtime.AccelLPF / (ins_runtime.AccelLPF + dt);			
	}
	BodyFrameToEarthFrame(ins_runtime.MotionAccel_b, ins_runtime.MotionAccel_n, ins_runtime.q); // 转换回导航系n
	
	//死区处理
	if(fabsf(ins_runtime.MotionAccel_n[0])<0.02f)
	{
	  ins_runtime.MotionAccel_n[0]=0.0f;	//x轴
	}
	if(fabsf(ins_runtime.MotionAccel_n[1])<0.02f)
	{
	  ins_runtime.MotionAccel_n[1]=0.0f;	//y轴
	}
	if(fabsf(ins_runtime.MotionAccel_n[2])<0.04f)
	{
	  ins_runtime.MotionAccel_n[2]=0.0f;//z轴
	}
	
	if(ins_time>3000.0f)
	{
		ins_runtime.ins_flag=1;//四元数基本收敛，加速度也基本收敛，可以开始底盘任务
		// 获取最终数据
		ins_runtime.Pitch= mahony.roll - PITCH_OFFSET;  //安装存在误差，减去初始角度
		ins_runtime.Roll= mahony.pitch - ROLL_OFFSET;
		ins_runtime.Yaw=mahony.yaw;

//ins_runtime.YawTotalAngle=ins_runtime.YawTotalAngle+ins_runtime.Gyro[2]*0.001f;
		
		if (ins_runtime.Yaw - ins_runtime.YawAngleLast > 3.1415926f)
		{
			ins_runtime.YawRoundCount--;
		}
		else if (ins_runtime.Yaw - ins_runtime.YawAngleLast < -3.1415926f)
		{
			ins_runtime.YawRoundCount++;
		}
			ins_runtime.YawTotalAngle = 6.283f* ins_runtime.YawRoundCount + ins_runtime.Yaw;
			ins_runtime.YawAngleLast = ins_runtime.Yaw;
		}
		else
		{
			ins_time++;
		}

		/* publish INS data to message bus */
		{
			INS_Data_t msg;
			msg.pitch     = ins_runtime.Pitch;
			msg.roll      = ins_runtime.Roll;
			msg.yaw_total = ins_runtime.YawTotalAngle;
			msg.gyro[0]   = ins_runtime.Gyro[0];
			msg.gyro[1]   = ins_runtime.Gyro[1];
			msg.gyro[2]   = ins_runtime.Gyro[2];
			msg.accel_b[0] = ins_runtime.MotionAccel_b[0];
			msg.accel_b[1] = ins_runtime.MotionAccel_b[1];
			msg.accel_b[2] = ins_runtime.MotionAccel_b[2];
			msg.ready     = ins_runtime.ins_flag;
			PubPushMessage(ins_pub, &msg);
		}

		osDelay(1);
	}
} 


/**
 * @brief          Transform 3dvector from BodyFrame to EarthFrame
 * @param[1]       vector in BodyFrame
 * @param[2]       vector in EarthFrame
 * @param[3]       quaternion
 */
void BodyFrameToEarthFrame(const float *vecBF, float *vecEF, float *q)
{
    vecEF[0] = 2.0f * ((0.5f - q[2] * q[2] - q[3] * q[3]) * vecBF[0] +
                       (q[1] * q[2] - q[0] * q[3]) * vecBF[1] +
                       (q[1] * q[3] + q[0] * q[2]) * vecBF[2]);

    vecEF[1] = 2.0f * ((q[1] * q[2] + q[0] * q[3]) * vecBF[0] +
                       (0.5f - q[1] * q[1] - q[3] * q[3]) * vecBF[1] +
                       (q[2] * q[3] - q[0] * q[1]) * vecBF[2]);

    vecEF[2] = 2.0f * ((q[1] * q[3] - q[0] * q[2]) * vecBF[0] +
                       (q[2] * q[3] + q[0] * q[1]) * vecBF[1] +
                       (0.5f - q[1] * q[1] - q[2] * q[2]) * vecBF[2]);
}

/**
 * @brief          Transform 3dvector from EarthFrame to BodyFrame
 * @param[1]       vector in EarthFrame
 * @param[2]       vector in BodyFrame
 * @param[3]       quaternion
 */
void EarthFrameToBodyFrame(const float *vecEF, float *vecBF, float *q)
{
    vecBF[0] = 2.0f * ((0.5f - q[2] * q[2] - q[3] * q[3]) * vecEF[0] +
                       (q[1] * q[2] + q[0] * q[3]) * vecEF[1] +
                       (q[1] * q[3] - q[0] * q[2]) * vecEF[2]);

    vecBF[1] = 2.0f * ((q[1] * q[2] - q[0] * q[3]) * vecEF[0] +
                       (0.5f - q[1] * q[1] - q[3] * q[3]) * vecEF[1] +
                       (q[2] * q[3] + q[0] * q[1]) * vecEF[2]);

    vecBF[2] = 2.0f * ((q[1] * q[3] + q[0] * q[2]) * vecEF[0] +
                       (q[2] * q[3] - q[0] * q[1]) * vecEF[1] +
                       (0.5f - q[1] * q[1] - q[2] * q[2]) * vecEF[2]);
}



