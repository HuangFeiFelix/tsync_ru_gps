#include "lcd_driver.h"
#include "lcd_cmd_process.h"
#include "clock_rb.h"


static Uint16 current_screen_id = 0;//��ǰ����ID
static Sint32 test_value = 0;//����ֵ
static Uint8 update_en = 0;//���±��

/*! 
 *  \brief  �����л�֪ͨ
 *  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
 *  \param screen_id ��ǰ����ID
 */
void NotifyScreen(Uint16 screen_id)
{
	//TODO: ����û�����
	current_screen_id = screen_id;//�ڹ��������п��������л�֪ͨ����¼��ǰ����ID

	if(current_screen_id==4)//�¶�����
	{
		Uint16 i = 0;
		Uint8 dat[100] = {0};

		//���ɷ���
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//������ݵ�ͨ��0

		//���ɾ�ݲ�
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//������ݵ�ͨ��1
	}
	else if(current_screen_id==9)//��ά��
	{
		//��ά��ؼ���ʾ�����ַ�ʱ����Ҫת��ΪUTF8���룬
		//ͨ����ָ�����֡���ת�������ݴ��123�� ���õ��ַ�����������
		Uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*! 
 *  \brief  ���������¼���Ӧ
 *  \param press 1���´�������3�ɿ�������
 *  \param x x����
 *  \param y y����
 */
void NotifyTouchXY(Uint8 press,Uint16 x,Uint16 y)
{
	//TODO: ����û�����
}

void SetTextValueInt32(Uint16 screen_id, Uint16 control_id,Sint32 value)
{
	Uint8 buffer[12] = {0};
	sprintf(buffer,"%d",value); //������ת��Ϊ�ַ���
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(Uint16 screen_id, Uint16 control_id,float value)
{
	Uint8 buffer[17] = {0};//Uint8 buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//�Ѹ�����ת��Ϊ�ַ���(����һλС��)
	SetTextValue(screen_id,control_id,buffer);
}

void UpdateUI()
{
	if(current_screen_id==2)//�ı����ú���ʾ
	{
		//��ǰ�������¶ȴ�0��100ѭ����ʾ�������ִ�0.0-99.9ѭ����ʾ
		SetTextValueInt32(2,5,test_value%100);//��ǰ����
		SetTextValueInt32(2,6,test_value%100);//�¶�
		SetTextValueFloat(2,7,(test_value%1000)/10.0);//������

		++test_value;
	}
	else if(current_screen_id==5)//�������ͻ������
	{
		SetProgressValue(5,1,test_value%100);

		++test_value;
	}
	else if(current_screen_id==6)//�Ǳ�ؼ�
	{
		SetMeterValue(6,1,test_value%360);
		SetMeterValue(6,2,test_value%360);

		++test_value;
	}
}

/*! 
 *  \brief  ��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton(Uint16 screen_id, Uint16 control_id, Uint8  state)
{
	//TODO: ����û�����
	if(screen_id==3)//��ť��ͼ�ꡢ��������
	{
		if(control_id==3)//���а�ť
		{
			if(state==0)//ֹͣ����
			{				
				AnimationPlayFrame(3,1,1);//��ʾֹͣͼ��
				AnimationStop(3,2);//����ֹͣ����
			}
			else//��ʼ����
			{				
				SetControlVisiable(3,1,1);//��ʾͼ��
				SetControlVisiable(3,2,1);//��ʾ����

				AnimationPlayFrame(3,1,0);//��ʾ����ͼ��
				AnimationStart(3,2);//������ʼ����
			}		
		}
		else if(control_id==4)//��λ��ť
		{
			SetControlVisiable(3,1,0);//����ͼ��
			SetControlVisiable(3,2,0);//���ض���
			SetButtonValue(3,3,0);//��ʾ��ʼ����
		}
	}
}

/*! 
 *  \brief  �ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText(Uint16 screen_id, Uint16 control_id, Uint8 *str)
{
	//TODO: ����û�����
	int32 value = 0; 

	if(screen_id==2)//����ID2���ı����ú���ʾ
	{
		sscanf(str,"%d",&value);//���ַ���ת��Ϊ����

		if(control_id==1)//��ߵ�ѹ
		{
			//�޶���ֵ��Χ��Ҳ�������ı��ؼ����������ã�
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //������ߵ�ѹ
			SetTextValueInt32(2,4,value/2);  //������ߵ�ѹ/2
		}
	}
}

/*! 
 *  \brief  �������ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyProgress(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  �������ؼ�֪ͨ
 *  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifySlider(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: ����û�����
	if(screen_id==5&&control_id==2)//�������
	{
		test_value = value;

		SetProgressValue(5,1,test_value); //���½�������ֵ
	}
}

/*! 
 *  \brief  �Ǳ�ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyMeter(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  �˵��ؼ�֪ͨ
 *  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item �˵�������
 *  \param state ��ť״̬��0�ɿ���1����
 */
void NotifyMenu(Uint16 screen_id, Uint16 control_id, Uint8  item, Uint8  state)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ѡ��ؼ�֪ͨ
 *  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item ��ǰѡ��
 */
void NotifySelector(Uint16 screen_id, Uint16 control_id, Uint8  item)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ʱ����ʱ֪ͨ����
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 */
void NotifyTimer(Uint16 screen_id, Uint16 control_id)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ȡ�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 *  \param _data ��������
 *  \param length ���ݳ���
 */
void NotifyReadFlash(Uint8 status,Uint8 *_data,Uint16 length)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  д�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 */
void NotifyWriteFlash(Uint8 status)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
 *  \param year �꣨BCD��
 *  \param month �£�BCD��
 *  \param week ���ڣ�BCD��
 *  \param day �գ�BCD��
 *  \param hour ʱ��BCD��
 *  \param minute �֣�BCD��
 *  \param second �루BCD��
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
		//�ο�Դ
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 10, 1);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 12, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 23, 0);

		//ptp����
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 4, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 7, 0);

		//ntp����
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
		
		//GPS��ʾ
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

		//�ο�Դ
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 10, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 12, 1);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 23, 0);

		//ptp����
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 4, 1);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 7, 1);

		//ntp����
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

		//GPS��ʾ
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

		//�ο�Դ
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 10, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 12, 0);
		SetButtonValue(CLOCK_STATUS_SCREEN_ID, 23, 1);

		//ptp����
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 4, 0);
		SetControlVisiable(CLOCK_STATUS_SCREEN_ID, 7, 0);

		//ntp����
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

		//GPS��ʾ
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
	else if (mask == 0)
		mask = pRootData->comm_port.mask;
	else if (gwip == 0)
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
	else if (mask == 0)
		mask = pRootData->ptp_port.mask;
	else if (gwip == 0)
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
	else if (mask == 0)
		mask = pRootData->ntp_port.mask;
	else if (gwip == 0)
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
	else if (serverip_ntp == 0)
		serverip_ntp = pRootData->ntp_ref_server_ip;
	else if (ref_type_ntp == 0)
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
    
	Uint8 cmd_type = msg->cmd_type;             //ָ������
	Uint8 ctrl_msg = msg->ctrl_msg;             //��Ϣ������
	Uint8 control_type = msg->control_type;     //�ؼ�����
	Uint16 screen_id = PTR2U16(&msg->screen_id);//����ID
	Uint16 control_id = PTR2U16(&msg->control_id);//�ؼ�ID
	Uint32 value = PTR2U32(msg->param);             //��ֵ

    //printf("%x %x %x %x %x\n",cmd_type,ctrl_msg,control_type,screen_id,value);
	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://����������
	case NOTIFY_TOUCH_RELEASE://�������ɿ�
		NotifyTouchXY(buf[1],PTR2U16(buf+2),PTR2U16(buf+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://дFLASH�ɹ�
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://дFLASHʧ��
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://��ȡFLASH�ɹ�
		NotifyReadFlash(1,buf+2,len-6);//ȥ��֡ͷ֡β
		break;
	case NOTIFY_READ_FLASH_FAILD://��ȡFLASHʧ��
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://��ȡRTCʱ��
		NotifyReadRTC(buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//����ID�仯֪ͨ
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
				case kCtrlButton: //��ť�ؼ�
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://�ı��ؼ�
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //�������ؼ�
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //�������ؼ�
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //�Ǳ�ؼ�
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://�˵��ؼ�
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://ѡ��ؼ�
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://����ʱ�ؼ�
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


