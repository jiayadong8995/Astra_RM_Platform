/**
  *********************************************************************
  * @file      remote_task.c/h
  * @brief     该任务是读取并处理遥控数据，
	*            将遥控数据转化为期望的速度、期望的转角、期望的腿长等
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "remote_task.h"
#include "cmsis_os.h"
#include "remote_control.h"
#include "robot_def.h"
#include "message_center.h"

//控制任务周期
#define REMOTE_TIME REMOTE_TASK_PERIOD

//初始腿长
#define BEGIN_LEG_LENGTH   LEG_LENGTH_DEFAULT

//遥控器映射
#define RC_TO_ADD_LEG 0.00005f   
#define RC_TO_TURN_RATIO  RC_TO_TURN

//最大速度
#define VX_MAX  RC_VX_MAX
#define RC_TO_VX_RATIO   RC_TO_VX
//速度斜坡
#define SPEED_STEP RC_SPEED_SLOPE

typedef struct
{
    float v_set;
    float x_set;
    float x_filter;
    float turn_set;
    float total_yaw;
    float leg_set;
    float myPithR;
    uint8_t start_flag;
    uint8_t jump_flag;
    uint8_t recover_flag;
    uint8_t last_recover_flag;
} Remote_Runtime_t;

float leg_add = 0;
int leg_set_flag = 0;


static void remote_data_process(const RC_Data_t *rc_data, Remote_Runtime_t *runtime);
static void slope_following(float *target,float *set,float acc);
static void local_saturate(float *in,float min,float max);

static Publisher_t *cmd_pub;

void remote_task(void)
{	
    Remote_Runtime_t cmd_state = {0};
    Subscriber_t *rc_sub = SubRegister("rc_data", sizeof(RC_Data_t));
    Subscriber_t *ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    Subscriber_t *chassis_state_sub = SubRegister("chassis_state", sizeof(Chassis_State_t));
    Subscriber_t *leg_right_sub = SubRegister("leg_right", sizeof(Leg_Output_t));
    Subscriber_t *leg_left_sub = SubRegister("leg_left", sizeof(Leg_Output_t));
    RC_Data_t rc_msg = {0};
    INS_Data_t ins_msg = {0};
    Chassis_State_t state_msg = {0};
    Leg_Output_t right_msg = {0};
    Leg_Output_t left_msg = {0};

	cmd_state.start_flag = 0;
	cmd_state.x_filter = 0;
	cmd_state.x_set = cmd_state.x_filter;
	cmd_state.v_set = 0.0f;
	cmd_state.turn_set = 0.0f;
	cmd_state.leg_set = BEGIN_LEG_LENGTH;

	cmd_pub = PubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
	while(1)
	{	
        if (SubGetMessage(ins_sub, &ins_msg))
        {
            cmd_state.myPithR = ins_msg.pitch;
        }
        if (SubGetMessage(chassis_state_sub, &state_msg))
        {
            cmd_state.x_filter = state_msg.x_filter;
            cmd_state.total_yaw = state_msg.total_yaw;
            if (cmd_state.turn_set == 0.0f)
            {
                cmd_state.turn_set = state_msg.total_yaw;
            }
        }
        SubGetMessage(leg_right_sub, &right_msg);
        SubGetMessage(leg_left_sub, &left_msg);
        SubGetMessage(rc_sub, &rc_msg);

		remote_data_process(&rc_msg, &cmd_state);

        {
            float leg_length = (left_msg.leg_length + right_msg.leg_length) / 2.0f;
            float leg_cmd = cmd_state.leg_set - leg_length;
            if(fabsf(leg_cmd) > 0.1f){
                cmd_state.leg_set = (leg_cmd > 0.0f) ? (leg_length + 0.1f) : (leg_length - 0.1f);
            }
        }

		/* publish chassis command */
		{
			Chassis_Cmd_t cmd = {
				.vx_cmd       = cmd_state.v_set,
				.turn_cmd     = cmd_state.turn_set,
				.leg_set      = cmd_state.leg_set,
				.start_flag   = cmd_state.start_flag,
				.jump_flag    = cmd_state.jump_flag,
				.recover_flag = cmd_state.recover_flag,
			};
			PubPushMessage(cmd_pub, &cmd);
		}

		osDelay(REMOTE_TIME);
		osDelay(50);
	}
}

static void remote_data_process(const RC_Data_t *rc_data, Remote_Runtime_t *runtime)
{
	runtime->last_recover_flag = runtime->recover_flag;
	if(!rc_data->online)
	{
		runtime->start_flag = 0;
		runtime->recover_flag = 0;
		runtime->jump_flag = 0;
	}
	else if(switch_is_mid(rc_data->sw[0]))
	{
		runtime->start_flag = 1;
		if(runtime->myPithR > PITCH_RECOVER_THRESHOLD ||runtime->myPithR < -PITCH_RECOVER_THRESHOLD)//原0.28
		{
			if(rc_data->sw[1] == RC_SW_MID)
			{
			runtime->recover_flag = 0;
			}else
			{
			runtime->recover_flag = 1;
			}
		}
		
	}
	else if(switch_is_down(rc_data->sw[0]))
	{
		runtime->start_flag = 0;
		runtime->recover_flag = 0;
		runtime->jump_flag = 0;
	}
	
	
    if(runtime->start_flag == 1)
    {
		if(switch_is_mid(rc_data->sw[1]))
		{
			if(660 == rc_data->ch[3])
			{
				runtime->jump_flag = 1;
			}else
			{
				runtime->jump_flag = 0;
			}
		}
		else if(switch_is_down(rc_data->sw[1]))
		{
			runtime->jump_flag = 0;
		}
		
		runtime->leg_set =  BEGIN_LEG_LENGTH;
//		if(rc_data->sw[1] == 3)
//		{
		runtime->turn_set = runtime->turn_set - rc_data->ch[2] * RC_TO_TURN_RATIO;
//		}else{
//		runtime->turn_set = runtime->turn_set - rc_data->ch[0] * RC_TO_TURN_RATIO;
		//}
		//速度斜坡控制
		float vx_speed_cmd = -rc_data->ch[1] * RC_TO_VX_RATIO;//遥控器相反（现加负号）
		//float vx_speed_cmd = rc_data->ch[1] * RC_TO_VX;//遥控器相反（现加负号）
		slope_following(&vx_speed_cmd,&runtime->v_set,RC_SPEED_SLOPE);
		local_saturate(&runtime->v_set,-VX_MAX,VX_MAX);

		runtime->x_set = runtime->x_set + runtime->v_set * (float)REMOTE_TIME/1000.0f;

  	}
	else if(runtime->start_flag == 0)
	{
		runtime->v_set=0.0f;//清零
		runtime->x_set=runtime->x_filter;//保存
		runtime->turn_set=runtime->total_yaw;//保存
	}

	//转向限幅
	if(fabsf(runtime->turn_set - runtime->total_yaw) > 0.3f){
		runtime->turn_set = ((runtime->turn_set - runtime->total_yaw) > 0) ? (runtime->total_yaw + 0.3f) : (runtime->total_yaw - 0.3f);
	}

	local_saturate(&runtime->leg_set,LEG_LENGTH_MIN,LEG_LENGTH_MAX);//限制腿长范围

}

static void slope_following(float *target,float *set,float acc)
{
	if(*target > *set)
	{
		*set = *set + acc ;
		if(*set >= *target)
		*set = *target;
	}
	else if(*target < *set)
	{
		*set = *set - acc ;
		if(*set <= *target)
		*set = *target;
	}
}

static void local_saturate(float *in,float min,float max)
{
	if(*in < min)
	{
		*in = min;
	}
	else if(*in > max)
	{
		*in = max;
	}
}
