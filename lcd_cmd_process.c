#include "lcd_driver.h"
#include "lcd_cmd_process.h"
#include "clock_rb.h"


static Uint16 current_screen_id = 0;//当前画面ID
static Sint32 test_value = 0;//测试值
static Uint8 update_en = 0;//更新标记

/*! 
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(Uint16 screen_id)
{
	//TODO: 添加用户代码
	current_screen_id = screen_id;//在工程配置中开启画面切换通知，记录当前画面ID

	if(current_screen_id==4)//温度曲线
	{
		Uint16 i = 0;
		Uint8 dat[100] = {0};

		//生成方波
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//添加数据到通道0

		//生成锯齿波
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//添加数据到通道1
	}
	else if(current_screen_id==9)//二维码
	{
		//二维码控件显示中文字符时，需要转换为UTF8编码，
		//通过“指令助手”，转换“广州大彩123” ，得到字符串编码如下
		Uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*! 
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(Uint8 press,Uint16 x,Uint16 y)
{
	//TODO: 添加用户代码
}

void SetTextValueInt32(Uint16 screen_id, Uint16 control_id,Sint32 value)
{
	Uint8 buffer[12] = {0};
	sprintf(buffer,"%d",value); //把整数转换为字符串
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(Uint16 screen_id, Uint16 control_id,float value)
{
	Uint8 buffer[17] = {0};//Uint8 buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//把浮点数转换为字符串(保留一位小数)
	SetTextValue(screen_id,control_id,buffer);
}

void UpdateUI()
{
	if(current_screen_id==2)//文本设置和显示
	{
		//当前电流、温度从0到100循环显示，艺术字从0.0-99.9循环显示
		SetTextValueInt32(2,5,test_value%100);//当前电流
		SetTextValueInt32(2,6,test_value%100);//温度
		SetTextValueFloat(2,7,(test_value%1000)/10.0);//艺术字

		++test_value;
	}
	else if(current_screen_id==5)//进度条和滑块控制
	{
		SetProgressValue(5,1,test_value%100);

		++test_value;
	}
	else if(current_screen_id==6)//仪表控件
	{
		SetMeterValue(6,1,test_value%360);
		SetMeterValue(6,2,test_value%360);

		++test_value;
	}
}

/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(Uint16 screen_id, Uint16 control_id, Uint8  state)
{
	//TODO: 添加用户代码
	if(screen_id==3)//按钮、图标、动画控制
	{
		if(control_id==3)//运行按钮
		{
			if(state==0)//停止运行
			{				
				AnimationPlayFrame(3,1,1);//显示停止图标
				AnimationStop(3,2);//动画停止播放
			}
			else//开始运行
			{				
				SetControlVisiable(3,1,1);//显示图标
				SetControlVisiable(3,2,1);//显示动画

				AnimationPlayFrame(3,1,0);//显示运行图标
				AnimationStart(3,2);//动画开始播放
			}		
		}
		else if(control_id==4)//复位按钮
		{
			SetControlVisiable(3,1,0);//隐藏图标
			SetControlVisiable(3,2,0);//隐藏动画
			SetButtonValue(3,3,0);//显示开始运行
		}
	}
}

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(Uint16 screen_id, Uint16 control_id, Uint8 *str)
{
	//TODO: 添加用户代码
	int32 value = 0; 

	if(screen_id==2)//画面ID2：文本设置和显示
	{
		sscanf(str,"%d",&value);//把字符串转换为整数

		if(control_id==1)//最高电压
		{
			//限定数值范围（也可以在文本控件属性中设置）
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //更新最高电压
			SetTextValueInt32(2,4,value/2);  //更新最高电压/2
		}
	}
}

/*! 
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: 添加用户代码
	if(screen_id==5&&control_id==2)//滑块控制
	{
		test_value = value;

		SetProgressValue(5,1,test_value); //更新进度条数值
	}
}

/*! 
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(Uint16 screen_id, Uint16 control_id, Uint8  item, Uint8  state)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(Uint16 screen_id, Uint16 control_id, Uint8  item)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(Uint16 screen_id, Uint16 control_id)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
void NotifyReadFlash(Uint8 status,Uint8 *_data,Uint16 length)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(Uint8 status)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
void NotifyReadRTC(Uint8 year,Uint8 month,Uint8 week,Uint8 day,Uint8 hour,Uint8 minute,Uint8 second)
{
}


void HandleWarnningState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleSettingState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleMainSreenState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleClockStatusState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleSupervisionState(struct root_data *pRootData, Uint16 screen_id, Uint16 control_id, unsigned char *data);

typedef void (*HandleWigetState)(struct root_data *,Uint16, Uint16,unsigned char *);
HandleWigetState HandleLcdEvent = HandleMainSreenState;

void HandleWarnningState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    printf("HandleWarnningState\n");
    
    switch(control_id)
    {
        case 21:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
            break;

        default:
            HandleLcdEvent = HandleWarnningState;
            break;
    }

}

void SetParamSettingScreen(struct root_data *pRootData)
{
	
	struct sockaddr_in temsock;

	if (pRootData->clock_info.ref_type == REF_SATLITE)
    {
		//参考源
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 10, 1);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 12, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 23, 0);

		//ptp配置
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 4, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 7, 0);

		//ntp配置
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 24, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 25, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 26, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 27, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 28, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 29, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 30, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 31, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 32, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 33, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 34, 0);
		
		//GPS显示
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 6, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 15, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 5, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 16, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 8, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 17, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 9, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 18, 1);
    }
	else if (pRootData->clock_info.ref_type == REF_PTP)
    {
		temsock.sin_addr.s_addr = pRootData->slot_list->pPtpSetcfg->serverList[0].serverIp;
		SetTextValue(CLOCK_STATUS_SCREEN_ID, 7, inet_ntoa(temsock.sin_addr));

		//参考源
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 10, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 12, 1);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 23, 0);

		//ptp配置
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 4, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 7, 1);

		//ntp配置
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 24, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 25, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 26, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 27, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 28, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 29, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 30, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 31, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 32, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 33, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 34, 0);

		//GPS显示
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 6, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 15, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 5, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 16, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 8, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 17, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 9, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 18, 0);
    }
	else if (pRootData->clock_info.ref_type == REF_NTP)
	{
		temsock.sin_addr.s_addr = pRootData->ntp_ref_server_ip;
		SetTextValue(CLOCK_STATUS_SCREEN_ID, 25, inet_ntoa(temsock.sin_addr));

		//参考源
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 10, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 12, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 23, 1);

		//ptp配置
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 4, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 7, 0);

		//ntp配置
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 24, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 25, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 26, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 27, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 28, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 29, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 30, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 31, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 32, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 33, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 34, 1);

		//GPS显示
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 6, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 15, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 5, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 16, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 8, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 17, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 9, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 18, 0);

		//ntp button
		if (pRootData->ntp_ref_interval == 16)
		{
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 27, 1);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 29, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 31, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 33, 0);
		}
		else if (pRootData->ntp_ref_interval == 32)
		{
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 27, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 29, 1);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 31, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 33, 0);
		}
		else if (pRootData->ntp_ref_interval == 64)
		{
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 27, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 29, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 31, 1);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 33, 0);
		}
		else if (pRootData->ntp_ref_interval == 128)
		{
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 27, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 29, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 31, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 33, 1);
		}
		else
		{
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 27, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 29, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 31, 0);
			SetButtonValue(CLOCK_STATUS_SCREEN_ID, 33, 0);
		}
	
	}

}

void HandleCtlEthState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    static int ip = 0;
    static int mask = 0;
	static int gwip = 0;
    static char set_flag = 0;

	if (ip == 0)
		ip = pRootData->comm_port.ip;
    
	if (mask == 0)
		mask = pRootData->comm_port.mask;
    
	if (gwip == 0)
		gwip = pRootData->comm_port.gwip;

    switch(control_id)
    {
        case 19:
           
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            //SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);

            if(set_flag == TRUE)
            {
				if ((pRootData->comm_port.ip != ip) || (pRootData->comm_port.mask != mask) || (pRootData->comm_port.gwip != gwip))
                {
                    pRootData->comm_port.ip = ip;
                    pRootData->comm_port.mask = mask;
					pRootData->comm_port.gwip = gwip;

                    SaveNetParamToFile(ctlEthConfig,&pRootData->comm_port);
                    usleep(2000);
                    SetCmdNetworkToEnv(&pRootData->comm_port);
                }
                set_flag = FALSE;
            }
            break;
        case 21:
            
            pRootData->lcd_sreen_id = PARAM_SETING_SCREEN_ID;
            HandleLcdEvent = HandleSettingState;
            //SetParamSettingScreen(pClockInfo->ref_type);
			break;

        case 0x04:
            ip = inet_addr(data);
			set_flag = TRUE;
            break;

        case 0x07:
            mask = inet_addr(data);
            set_flag = TRUE;
			break;

		case 16:
			gwip = inet_addr(data);
			set_flag = TRUE;
            break;
        default:
            HandleLcdEvent = HandleCtlEthState;
            break;
    }


}

void HandlePtpEthState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    static int ip = 0;
    static int mask = 0;
	static int gwip = 0;
    static char set_flag = 0;

	if (ip == 0)
		ip = pRootData->ptp_port.ip;
    
	if (mask == 0)
		mask = pRootData->ptp_port.mask;
    
	if (gwip == 0)
		gwip = pRootData->ptp_port.gwip;

    switch(control_id)
    {
        case 19:
           
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);

            if(set_flag == TRUE)
            {
				if ((pRootData->ptp_port.ip != ip) || (pRootData->ptp_port.mask != mask) || (pRootData->ptp_port.gwip != gwip))
                {
                    pRootData->ptp_port.ip = ip;
                    pRootData->ptp_port.mask = mask;
					pRootData->ptp_port.gwip = gwip;

                    SaveNetParamToFile(ptpEthConfig,&pRootData->ptp_port);
                    usleep(2000);
                    SetCmdNetworkToEnv(&pRootData->ptp_port);
                    usleep(2000);
                    stop_ptp_daemon();
                    usleep(2000);
                    start_ptp_daemon();
                }
                set_flag = FALSE;
            }
    
            break;
        case 21:
            
            pRootData->lcd_sreen_id = PARAM_SETING_SCREEN_ID;
            HandleLcdEvent = HandleSettingState;
            //SetParamSettingScreen(pClockInfo->ref_type);
			break;

        case 0x04:
            ip = inet_addr(data);
			set_flag = TRUE;
            break;

        case 0x07:
            mask = inet_addr(data);
            set_flag = TRUE;
            break;

		case 16:
			gwip = inet_addr(data);
			set_flag = TRUE;
			break;

        default:
            HandleLcdEvent = HandleCtlEthState;
            break;
    }

}


void HandleNtpEthState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{

    struct clock_info *pClockInfo = &pRootData->clock_info;
    static int ip = 0;
    static int mask = 0;
	static int gwip = 0;
    static char set_flag = 0;

	if (ip == 0)
		ip = pRootData->ntp_port.ip;
    
	if (mask == 0)
		mask = pRootData->ntp_port.mask;
    
	if (gwip == 0)
		gwip = pRootData->ntp_port.gwip;
    switch(control_id)
    {
        case 19:
           
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);

            if(set_flag == TRUE)
            {
				if ((pRootData->ntp_port.ip != ip) || (pRootData->ntp_port.mask != mask) || (pRootData->ntp_port.gwip != gwip))
                {
                    pRootData->ntp_port.ip = ip;
                    pRootData->ntp_port.mask = mask;
					pRootData->ntp_port.gwip = gwip;

                    SaveNetParamToFile(ntpEthConfig,&pRootData->ntp_port);
                    usleep(2000);
                    SetCmdNetworkToEnv(&pRootData->ntp_port);
                    usleep(2000);
                    stop_ntp_daemon();
                    usleep(2000);
                    start_ntp_daemon();
                }
                set_flag = FALSE;
            }

            break;
        case 21:
            
            pRootData->lcd_sreen_id = PARAM_SETING_SCREEN_ID;
            HandleLcdEvent = HandleSettingState;
            //SetParamSettingScreen(pClockInfo->ref_type);
			break;

        case 0x04:
            ip = inet_addr(data);
            set_flag = TRUE;
            break;

        case 0x07:
            mask = inet_addr(data);
            set_flag = TRUE;
            break;

		case 16:
			gwip = inet_addr(data);
			set_flag = TRUE;
            break;
        default:
            HandleLcdEvent = HandleCtlEthState;
            break;
    }

}


void lcdDisplayEnternet(struct NetInfor *net,Uint16 screen_id)
{
    struct sockaddr_in temsock;
    char data[20];

    temsock.sin_addr.s_addr = net->ip;
    SetTextValue(screen_id,4,inet_ntoa(temsock.sin_addr));
    temsock.sin_addr.s_addr = net->mask;
    SetTextValue(screen_id,7,inet_ntoa(temsock.sin_addr));
	temsock.sin_addr.s_addr = net->gwip;
	SetTextValue(screen_id, 16, inet_ntoa(temsock.sin_addr));

    memset(data,0,sizeof(data));
    sprintf(data,"%02x:%02x:%02x:%02x:%02x:%02x",net->mac[0],net->mac[1],net->mac[2]
                            ,net->mac[3],net->mac[4],net->mac[5]);
    SetTextValue(screen_id,10,data);
}

void lcdDisplaySuperVision(struct SuperVisionINI *net, Uint16 screen_id)
{
	struct sockaddr_in temsock;
	char data[32];

	temsock.sin_addr.s_addr = net->MonitorServerIP;
	SetTextValue(screen_id, 2, inet_ntoa(temsock.sin_addr));

	memset(data, 0, sizeof(data));
	sprintf(data, "%d", net->HttpReportCycle);
	SetTextValue(screen_id, 4, data);
}


void HandleSettingState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    printf("HandleSettingState\n");
    struct clock_info *pClockInfo = &pRootData->clock_info;

    switch(control_id)
    {
        case 21:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
            break;
        case 7:
            pRootData->lcd_sreen_id = ETH_CTL_SCREEN_ID;
            lcdDisplayEnternet(&pRootData->comm_port,ETH_CTL_SCREEN_ID);
            HandleLcdEvent = HandleCtlEthState;
            break;
        case 12:
            pRootData->lcd_sreen_id = ETH_PTP_SCREEN_ID;
            lcdDisplayEnternet(&pRootData->ptp_port,ETH_PTP_SCREEN_ID);
            HandleLcdEvent = HandlePtpEthState;
            break;
        case 15:
            pRootData->lcd_sreen_id = ETH_NTP_SCREEN_ID;
            lcdDisplayEnternet(&pRootData->ntp_port,ETH_NTP_SCREEN_ID);
            HandleLcdEvent = HandleNtpEthState;
			break;

        default:
            HandleLcdEvent = HandleSettingState;
            break;
    }
}

void HandleClockStatusState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
	static char ref_type = 0;
	static int serverip_ptp = 0;
	static int serverip_ntp = 0;
	static int ref_type_ntp = 0;
    printf("HandleClockStatusState\n");

	if (serverip_ptp == 0)
		serverip_ptp = pRootData->slot_list->pPtpSetcfg->serverList[0].serverIp;
    
	if (serverip_ntp == 0)
		serverip_ntp = pRootData->ntp_ref_server_ip;
    
	if (ref_type_ntp == 0)
		ref_type_ntp = pRootData->ntp_ref_interval;

    switch(control_id)
    {
		case 19:
			pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
			HandleLcdEvent = HandleMainSreenState;
			pRootData->clock_info.ref_type = ref_type;
			if (serverip_ptp != 0 && ref_type == REF_PTP && pRootData->slot_list->pPtpSetcfg->serverList[0].serverIp != serverip_ptp)
			{
				pRootData->slot_list->pPtpSetcfg->serverList[0].serverIp = serverip_ptp;
				Save_PtpParam_ToFile(pRootData->slot_list->pPtpSetcfg, ptpConfig_s);
				usleep(2000);
				stop_ptp_daemon();
				usleep(2000);
				start_ptp_daemon();
			}
			else if (ref_type == REF_NTP)
			{
				if (serverip_ntp != pRootData->ntp_ref_server_ip || ref_type_ntp != pRootData->ntp_ref_interval)
				{
					pRootData->ntp_ref_server_ip = serverip_ntp;
					pRootData->ntp_set_time = pRootData->clock_info.run_times;
					pRootData->ntp_ref_interval = ref_type_ntp;
				}
			}
			break;

        case 21:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            HandleLcdEvent = HandleMainSreenState;
            break;
		
		case 7:
			serverip_ptp = inet_addr(data);
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 25:
			serverip_ntp = inet_addr(data);
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 10:
			ref_type = REF_SATLITE;
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 12:
			ref_type = REF_PTP;
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 23:
			ref_type = REF_NTP;
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 27:
			ref_type_ntp = 16;
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 29:
			ref_type_ntp = 32;
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 31:
			ref_type_ntp = 64;
			HandleLcdEvent = HandleClockStatusState;
			break;

		case 33:
			ref_type_ntp = 128;
			HandleLcdEvent = HandleClockStatusState;
			break;

        default:
            HandleLcdEvent = HandleClockStatusState;
            break;
    }


}

void HandleMainSreenState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{

        printf("HandleMainSreenState\n");
        struct clock_info *pClockInfo = &pRootData->clock_info;

        switch(control_id)
        {
            case 12:
                pRootData->lcd_sreen_id = CLOCK_STATUS_SCREEN_ID;
				SetParamSettingScreen(pRootData);
                HandleLcdEvent = HandleClockStatusState;
                break;

            case 13:
                pRootData->lcd_sreen_id = PARAM_SETING_SCREEN_ID;
                HandleLcdEvent = HandleSettingState;
                break;

            case 14:
                pRootData->lcd_sreen_id = WARN_SCREEN_ID;
                HandleLcdEvent = HandleWarnningState;
                break;

			case 18:
				pRootData->lcd_sreen_id = SUPER_VISION_ID;
				lcdDisplaySuperVision(&pRootData->supervision, SUPER_VISION_ID);
				HandleLcdEvent = HandleSupervisionState;
				break;

            default:
                pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
                HandleLcdEvent = HandleMainSreenState;
                break;
        }
}

void HandleSupervisionState(struct root_data *pRootData, Uint16 screen_id, Uint16 control_id, unsigned char *data)
{
	static int ip = 0;
	static int time = -1;
	static char set_flag = FALSE;
	printf("HandleSupervisionState\n");

	if (ip == 0)
		ip = pRootData->supervision.MonitorServerIP;
	else if (time == -1)
		time = pRootData->supervision.HttpReportCycle;

	switch (control_id)
	{
		case 19:
			HandleLcdEvent = HandleMainSreenState;
			if (set_flag == TRUE)
			{
				SetTextValue(MAIN_SCREEN_ID, 4, pRootData->current_time);
				if ((ip != pRootData->supervision.MonitorServerIP) || (time != pRootData->supervision.HttpReportCycle))
				{
					pRootData->supervision.MonitorServerIP = ip;
					pRootData->supervision.HttpReportCycle = time;
					SaveSuperVisionToFile(supervisionConfig,pRootData->supervision);
					usleep(5000);
					system("pkill /mnt/TSS");
					usleep(2000);
					system("/mnt/TSS &");
				}
				set_flag = FALSE;
			}
			break;

		case 21:
			pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
			HandleLcdEvent = HandleMainSreenState;
			break;

		case 2:
			ip = inet_addr(data);
			set_flag = TRUE;
			break;

		case 4:
			if ((time = atoi(data)) < 0)
				time = 0;
			else if ((time = atoi(data)) > (60 * 60 * 24 * 365))
				time = (60 * 60 * 24 * 365);

			set_flag = TRUE;
			break;

		default:
			HandleLcdEvent = HandleClockStatusState;
			break;
	}

}


void ProcessLcdMessage(struct root_data *pRootData,char *buf,Uint16 len)
{
    struct LcdCtlMsg *msg = (struct LcdCtlMsg *)buf;
    
	Uint8 cmd_type = msg->cmd_type;             //指令类型
	Uint8 ctrl_msg = msg->ctrl_msg;             //消息的类型
	Uint8 control_type = msg->control_type;     //控件类型
	Uint16 screen_id = PTR2U16(&msg->screen_id);//画面ID
	Uint16 control_id = PTR2U16(&msg->control_id);//控件ID
	Uint32 value = PTR2U32(msg->param);             //数值

    //printf("%x %x %x %x %x\n",cmd_type,ctrl_msg,control_type,screen_id,value);
	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://触摸屏按下
	case NOTIFY_TOUCH_RELEASE://触摸屏松开
		NotifyTouchXY(buf[1],PTR2U16(buf+2),PTR2U16(buf+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://写FLASH成功
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://写FLASH失败
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://读取FLASH成功
		NotifyReadFlash(1,buf+2,len-6);//去除帧头帧尾
		break;
	case NOTIFY_READ_FLASH_FAILD://读取FLASH失败
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://读取RTC时间
		NotifyReadRTC(buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
			{
				NotifyScreen(screen_id);
			}
            else if(ctrl_msg == MSG_GET_DATA)
            {
                
                (*HandleLcdEvent)(pRootData,screen_id,control_id,msg->param);
            }
			else
			{
				switch(control_type)
				{
				case kCtrlButton: //按钮控件
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://文本控件
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //进度条控件
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //滑动条控件
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //仪表控件
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://菜单控件
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://选择控件
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://倒计时控件
					NotifyTimer(screen_id,control_id);
					break;
				default:
					break;
				}
			}			
		}
		break;
	default:
		break;
	}
}


