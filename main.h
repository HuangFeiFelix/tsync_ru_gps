#ifndef __MAIN_H__
#define __MAIN_H__

#include "common.h"
#include "list_ctrl.h"
#include "satellite_api.h"
#include "clock_rb.h"

#define SLOT_COUNT 4
#define SLOT_TYPE_CORE 'A'
#define SLOT_TYPE_PTP  'B'
#define SLOT_TYPE_NTP  'C'

#define MAIN_SCREEN_ID          0x00
#define CLOCK_STATUS_SCREEN_ID  0x01
#define PARAM_SETING_SCREEN_ID  0x02
#define WARN_SCREEN_ID          0x03

#define ETH_CTL_SCREEN_ID       0x04
#define ETH_PTP_SCREEN_ID       0x05
#define ETH_NTP_SCREEN_ID       0x06

#define SUPER_VISION_ID			0x07

#define CLOCK_STATUS_WIDGET_ID      12
#define CLOCK_SETTING_WIDGET_ID     13
#define CLOCK_WARNNING_WIDGET_ID    14

#define	LEAP_NOWARNING	0x0	/* normal, no leap second warning */
#define	LEAP_ADDSECOND	0x1	/* last minute of day has 61 seconds */
#define	LEAP_DELSECOND	0x2	/* last minute of day has 59 seconds */
#define	LEAP_NOTINSYNC	0x3	/* overload, clock is free running */

#define STRATUM_0_PRESION -20
#define STRATUM_1_PRESION -20
#define STRATUM_2_PRESION -18
#define STRATUM_3_PRESION -15


enum
{
    ENUM_SLOT_COR_ADDR = 0x00,
    ENUM_SLOT_1_ADDR,
    ENUM_SLOT_2_ADDR,
    ENUM_SLOT_3_ADDR,
    ENUM_SLOT_4_ADDR,
    ENUM_SLOT_PC_ADDR = 0xff
};



struct ServerList
{
    Uint32 serverIp;            /** ����������ip */
    char  serverMac[6];         /** ����������mac */    
};


struct PtpSetCfg
{
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** ��� */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
    Uint8 domainFilterSwitch;            /** ��Ź��˿���0:�أ�1:�� */
    Uint32 UnicastDuration;       /** ����Э�̻��ƣ��ϻ�ʱ�� */
    Integer8 logSyncInterval;
    Integer8 logAnnounceInterval;
    
    Integer8 logMinDelayReqInterval;
    Integer8 logMinPdelayReqInterval;
   
    UInteger8 grandmasterPriority1;
    UInteger8 grandmasterPriority2;
    Uint8 validServerNum;              /** ��Ч�ķ�����ip���� */
    Uint8 currentUtcOffset;
    struct ServerList serverList[10];
};

struct PtpStatusCfg
{
    Uint8 synAlarm;
    Uint8 announceAlarm;
    Uint8 delayRespAlarm;
    Uint8 notAvailableAlarm;
    long long MeanPathDelay;
    long long TimeOffset;
    
};

struct Md5key
{
	Uint8 key_valid;		//the md5 key is valid or not:1 valid ; 2 invalid
	Uint8 key_length;		//the length of the md5 key
	Uint8 key[20];		    //the md5 key in ASCII 
	
};


struct NtpSetCfg
{
	Uint8       net_flag;			//the flag about network parameter
	Uint8       md5_flag;			//the flag about md5 authentication   bit1--1=Enable md5;0=Disable md5 ;    bit2--0= broadcast Closed;1=setted key number;  bit3--0= multicast Closed;1=setted key number;
	struct Md5key     current_key[10];    //the space to store md5 key structure
    unsigned char stratum;   //ʱ��Դ�ȼ�
    unsigned char leap;      //����
    signed char precision;   //����
    unsigned char refid[5];  //ID
	Uint32    freq_b;                   // broadcast send period   ntpd do not have
	Uint32    freq_m;                   // multicast send period   ntpd do not have
    Uint8     broadcast;                    // flag: 0 stop 1 run
    Uint8     broadcast_key_num;
	Uint8     multicast;                    // flag
    Uint8     multicast_key_num;
    Uint8     sympassive;                   // flag
	Uint8     blacklist;                    // flag
	Uint8     whitelist;                    // flag

    Uint8     blacklist_num;
    Uint8     whitelist_num;
	Uint8     blacklist_flag[16];           //��������ʶ�б�
	Uint8     whitelist_flag[16];           //��������ʶ�б�
	Uint32    blacklist_ip[16];             //������ip��ַ�б�
	Uint32    blacklist_mask[16];	        //���������������б�
	Uint32    whitelist_ip[16];             // ������ip��ַ�б�
	Uint32    whitelist_mask[16];	        // ���������������б�

};

struct SlotList
{
    char slot_type;     /**������  */
    struct NtpSetCfg *pNtpSetCfg;
    struct PtpSetCfg *pPtpSetcfg;
    struct PtpStatusCfg *pPtpStatusCfg; 
    struct PtpSetCfg *pPtpSetcfg_m;
};

//�����ص������ļ���Ϣ�ṹ��
struct SuperVisionINI
{
	Uint32 TimeSourceType;				//����ʱ��Դģ������
	Uint32 NtpServerIP;						//�ϼ�NTPServer��IP��ַ
	Uint32  HttpReportCycle;			//���ܷ������ϱ���Ϣ�����ڣ��룩
	Uint32 MonitorServerIP;					//��ܷ�����IP
	Uint32 MonitorServerPort;			//��ܷ������˿�
	Sint8 MonitorPostURL[256];
	Uint32 SystemId;						//��ϵͳ��Դ����
	Uint32 Subsystemid;						//��ϵͳID
	Sint8 SystemType[256];						//ϵͳ����
};

struct root_data{

		struct dev_head dev_head;	//�豸��Ϣͷ
        struct device dev[15];		//~{J9SCIh18PEO"~}
        int dev_fd;
        
        struct SlotList slot_list[6];

        int lcd_sreen_id;
        
        char Version[4];

        /*�ӿ��㷨*/
        struct clock_info clock_info;    
              
        /*��������*/
        struct Satellite_Data satellite_data;  

        FILE *file_fd;

        Uint8 flag_usuallyRoutine;

        struct NetInfor comm_port;
        struct NetInfor ptp_port;
        struct NetInfor ntp_port;

        int ctl_sin_port;
        char ctl_ip_address[20];

		int ntp_ref_interval; // 16 32 64
		int ntp_ref_server_ip;
		unsigned int ntp_set_time;//����ntpʱ�뵱ǰʱ��ȡģ�Ĳ��ʱ�䣬��֤ÿ�δӵ�ǰʱ�俪ʼ���ntp_ref_intervalͬ����������ͨ������ʱ�����ͬ�����
        
        char current_time[30];
        
		struct SuperVisionINI supervision;

		pthread_attr_t pattr;	//�߳�����
		pthread_t p_usual;		//�ճ�������
		pthread_t p_recv;		//���ݽ����߳�
		pthread_t p_send;		//���ݷ����߳�
		pthread_t p_handle;		//���ݴ����߳�
};

struct NtpdStatus
{
    unsigned char stratum;   //ʱ��Դ�ȼ�
    unsigned char leap;      //����
    signed char precision;   //����
    unsigned char refid[5];  //ID
};

struct PtpStatus
{
    char priority1;
    char priority2;
    char timeSource;
    char utcOffset;
    char clockClass;
    char clockAccuracy;
    char blockOutput;
    char reserve;
};

struct PtpReference
{
    
};

extern struct root_data *g_RootData;
extern char *ctlEthConfig;
extern char *ptpEthConfig;
extern char *ntpEthConfig;
extern char *ptpConfig_m;
extern char *ptpConfig_s;
extern char *ntpConfig;
extern char *md5Config;
extern char *supervisionConfig;


extern void start_ptp_daemon();
extern void stop_ptp_daemon();
extern void start_ntp_daemon();
extern void stop_ntp_daemon();

#endif
