#ifndef __LCD_CMD_PROCESS_H__
#define __LCD_CMD_PROCESS_H__
#include "common.h"
#include "main.h"

#define NOTIFY_TOUCH_PRESS      0X01             //����������֪ͨ
#define NOTIFY_TOUCH_RELEASE  0X03              //�������ɿ�֪ͨ
#define NOTIFY_WRITE_FLASH_OK  0X0C              //дFLASH�ɹ�
#define NOTIFY_WRITE_FLASH_FAILD  0X0D          //дFLASHʧ��
#define NOTIFY_READ_FLASH_OK  0X0B              //��FLASH�ɹ�
#define NOTIFY_READ_FLASH_FAILD  0X0F           //��FLASHʧ��
#define NOTIFY_MENU                        0X14  //�˵��¼�֪ͨ
#define NOTIFY_TIMER                       0X43  //��ʱ����ʱ֪ͨ
#define NOTIFY_CONTROL                0XB1      //�ؼ�����֪ͨ
#define NOTIFY_READ_RTC               0XF7       //��ȡRTCʱ��
#define MSG_GET_CURRENT_SCREEN 0X01              //����ID�仯֪ͨ
#define MSG_GET_DATA            0X11             //�ؼ�����֪ͨ

#define PTR2U16(PTR) ((((Uint8 *)(PTR))[0]<<8)|((Uint8 *)(PTR))[1])  //�ӻ�����ȡ16λ����
#define PTR2U32(PTR) ((((Uint8 *)(PTR))[0]<<24)|(((Uint8 *)(PTR))[1]<<16)|(((Uint8 *)(PTR))[2]<<8)|((Uint8 *)(PTR))[3])  //�ӻ�����ȡ32λ����




enum CtrlType
{
	kCtrlUnknown=0x0,
	kCtrlButton=0x10,  //��ť
	kCtrlText,  //�ı�
	kCtrlProgress,  //������
	kCtrlSlider,    //������
	kCtrlMeter,  //�Ǳ�
	kCtrlDropList, //�����б�
	kCtrlAnimation, //����
	kCtrlRTC, //ʱ����ʾ
	kCtrlGraph, //����ͼ�ؼ�
	kCtrlTable, //���ؼ�
	kCtrlMenu,//�˵��ؼ�
	kCtrlSelector,//ѡ��ؼ�
	kCtrlQRCode,//��ά��
};

#pragma pack(push)
#pragma pack(1)	//���ֽڶ���

struct LcdCtlMsg
{
	Uint8    cmd_head;  //֡ͷ

	Uint8    cmd_type;  //��������(UPDATE_CONTROL)	
	Uint8    ctrl_msg;   //CtrlMsgType-ָʾ��Ϣ������
	Uint16   screen_id;  //������Ϣ�Ļ���ID
	Uint16   control_id;  //������Ϣ�Ŀؼ�ID
	Uint8    control_type; //�ؼ�����

	Uint8    param[256];//�ɱ䳤�Ȳ��������256���ֽ�

	Uint8  cmd_tail[4];   //֡β
};


void ProcessLcdMessage(struct root_data *pRootData,char *buf,Uint16 len);

void NotifyScreen(Uint16 screen_id);

void NotifyTouchXY(Uint8 press,Uint16 x,Uint16 y);

void NotifyButton(Uint16 screen_id, Uint16 control_id, Uint8  state);

void NotifyText(Uint16 screen_id, Uint16 control_id, Uint8 *str);

void NotifyProgress(Uint16 screen_id, Uint16 control_id, Uint32 value);

void NotifySlider(Uint16 screen_id, Uint16 control_id, Uint32 value);

void NotifyMeter(Uint16 screen_id, Uint16 control_id, Uint32 value);

void NotifyMenu(Uint16 screen_id, Uint16 control_id, Uint8  item, Uint8  state);

void NotifySelector(Uint16 screen_id, Uint16 control_id, Uint8  item);

void NotifyTimer(Uint16 screen_id, Uint16 control_id);

void NotifyReadFlash(Uint8 status,Uint8 *_data,Uint16 length);

void NotifyWriteFlash(Uint8 status);

void NotifyReadRTC(Uint8 year,Uint8 month,Uint8 week,Uint8 day,Uint8 hour,Uint8 minute,Uint8 second);

extern void update_lcd_display(struct root_data *pRootData);


#endif
