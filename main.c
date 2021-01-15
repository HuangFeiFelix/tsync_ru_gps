#include "main.h"
#include "log.h"
#include "net.h"
#include "manager.h"
#include "fpga_time.h"
#include "lcd_cmd_process.h"
#include "clock_rb.h"

struct root_data *g_RootData;

#define COMM_GET_SEC    3
#define COMM_SET_SEC    4
#define COMM_MODIFY_SEC 5

char *ctlEthConfig = "/mnt/network_ctl.cfg";
char *ptpEthConfig = "/mnt/network_ptp.cfg";
char *ntpEthConfig = "/mnt/network_ntp.cfg";
char *ptpConfig_s = "/mnt/ptp_s.conf";
char *ptpConfig_m = "/mnt/ptp_m.conf";

char *ntpConfig = "/mnt/ntp/ntp.conf";
char *md5Config = "/mnt/ntp/keys";
char *commSysConfig = "/mnt/comm_sys.cfg";

char *supervisionConfig = "/mnt/Config.ini";

#define PATH_PTP_DAEMON "/mnt/ptp"
#define PATH_NTP_DAEMON "/mnt/ntp/ntpd"

int remove_space(char *src,char *out)
{
    char *p = src;
    int i;
    i = strlen(src);

    

    while(i--)
    {
        if(*p == 0x20)
        {
            p++;
        }
        else
        {
            *out = *p;
            p++;
            out++;
        }


    }

	return 0;
}

int String2Bytes(unsigned char* szSrc, unsigned char* pDst, int nDstMaxLen)     
{  
    int i;
    if(szSrc == NULL)  
    {  
        return 0;  
    }
    
    int iLen = strlen((char *)szSrc);  
    if (iLen <= 0 || iLen%2 != 0 || pDst == NULL || nDstMaxLen < iLen/2)  
    {  
        return 0;  
    }  
      
    iLen /= 2;  
    //strupr((char *)szSrc);  
    for (i=0; i< iLen; i++)  
    {  
        int iVal = 0;  
        unsigned char *pSrcTemp = szSrc + i*2;  
        sscanf((char *)pSrcTemp, "%02x", &iVal);  
        pDst[i] = (unsigned char)iVal;  
    }  
      
    return iLen;  
}   


void AddData_ToSendList(struct root_data *pRoot,char devType,void *buf,short len)
{
    add_send(buf,len,&pRoot->dev[devType]);
}

char setzda[100] = {"B5 62 06 01 08 00 F0 08 00 01 00 00 00 00 08 60"};
char pubx[100] =   {"B5 62 06 01 08 00 F1 04 00 01 00 00 00 00 05 4C"};
char bdgps[200] = {"B5 62 06 3E 2C 00 00 00 20 05 00 08 10 00 00 00 01 01 01 01 03 00 00 00 01 01 03 08 10 00 01 00 01 01 05 00 03 00 00 00 01 01 06 08 0E 00 00 00 01 01 FC 01"};

void Init_ublox(struct root_data *pRootData)
{


    char txbuf[200];
    unsigned char txbufBin[200];
    int txlen;
#if 0

    txbuf[0] = 'S';
    txbuf[1] = 'T';
    txbuf[2] = 'A';
    txbuf[3] = 0x0d;
    txbuf[4] = 0x0a;
    AddData_ToSendList(pRootData,ENUM_XO,txbuf,5);




	memset(txbuf,0,sizeof(txbuf));
	memset(txbufBin,0,sizeof(txbufBin));

	remove_space(bdgps,txbuf);
	txlen = String2Bytes(txbuf,txbufBin,200);
    AddData_ToSendList(pRootData,ENUM_GPS,txbufBin,txlen);

	memset(txbuf,0,sizeof(txbuf));
	memset(txbufBin,0,sizeof(txbufBin));

	remove_space(setzda,txbuf);
	txlen = String2Bytes(txbuf,txbufBin,200);

	AddData_ToSendList(pRootData,ENUM_GPS,txbufBin,txlen);
#endif

	memset(txbuf,0,sizeof(txbuf));
	memset(txbufBin,0,sizeof(txbufBin));
	remove_space(pubx,txbuf);
	txlen = String2Bytes(txbuf,txbufBin,200);

	AddData_ToSendList(pRootData,ENUM_GPS,txbufBin,txlen);

}

void start_ptp_daemon()
{
    system("/mnt/ptp &");
}


void stop_ptp_daemon()
{
    system("pkill ptp");
}


void start_ntp_daemon()
{
    system("/mnt/ntp/ntpd -c /mnt/ntp/ntp.conf");

}

void stop_ntp_daemon()
{
    system("pkill ntp");

}

void reset_sys_daemon()
{
    system("reboot");

}

void Init_Thread_Attr(pthread_attr_t *pThreadAttr)
{
    int ret;
    int policy;

    struct sched_param thread_param;
    
    ret = pthread_attr_init(pThreadAttr);
    if(ret != 0)
    {
        printf("pthread_attr_init error!\n");
    }
 
    ret = pthread_attr_setscope(pThreadAttr,PTHREAD_SCOPE_SYSTEM);
    if(ret != 0)
    {
        printf("PTHREAD_SCOPE_SYSTEM error!\n");
    }
 
    ret = pthread_attr_setschedpolicy(pThreadAttr,SCHED_RR);
    if(ret != 0)
    {
        printf("pthread_attr_setschedpolicy error!\n");
    }
        
    pthread_attr_getschedparam(pThreadAttr,  &thread_param);
    printf("schedparam :%d\n",thread_param.sched_priority);

    /**ÓÅÏÈ¼¶  */
    pthread_attr_getschedpolicy(pThreadAttr,&policy);
    if(policy == SCHED_FIFO)
         printf("Schedpolicy:SCHED_FIFO\n");
    if(policy==SCHED_RR)
         printf("Schedpolicy:SCHED_RR\n");
    if(policy==SCHED_OTHER)
         printf("Schedpolicy:SCHED_OTHER\n");
 
    /*param.sched_priority=30;*/
    pthread_attr_setschedparam(pThreadAttr,  &thread_param);

}

void InitSlotLocal(struct SlotList *pSlotLocal)
{
    int ret;

    Load_PtpParam_FromFile(pSlotLocal->pPtpSetcfg,ptpConfig_s);
    Load_PtpParam_FromFile(pSlotLocal->pPtpSetcfg_m,ptpConfig_m);
    Load_NtpdParam_FromFile(pSlotLocal->pNtpSetCfg,ntpConfig,md5Config);
    
}
void InitLoadRtcTime(struct root_data *pRootData)
{
    struct timeval tv;
    struct Satellite_Data *pSateData = &pRootData->satellite_data;
    struct clock_info *pClockInfo = &pRootData->clock_info;

    gettimeofday(&tv,NULL);

    pSateData->gps_utc_leaps = 18;
    pSateData->time = tv.tv_sec;
    
    tv.tv_sec += pSateData->gps_utc_leaps;
    tv.tv_sec += 19;
    tv.tv_sec += 1;
    
    SetFpgaTime(tv.tv_sec);
}

void InitSlotList(struct root_data *pRootData)
{
    int i;
    struct clock_info *pClockInfo = &pRootData->clock_info;

    struct SlotList *pSlotList = &pRootData->slot_list[ENUM_SLOT_COR_ADDR];
    pSlotList->slot_type = SLOT_TYPE_CORE;
    pSlotList->pNtpSetCfg = (struct NtpSetCfg*)malloc(sizeof(struct NtpSetCfg));
	if (pSlotList->pNtpSetCfg != NULL)
		memset(pSlotList->pNtpSetCfg, 0, sizeof(struct NtpSetCfg));
	else
		pSlotList->pNtpSetCfg = NULL;
    
    pSlotList->pPtpSetcfg = (struct PtpSetCfg*)malloc(sizeof(struct PtpSetCfg));
	if (pSlotList->pPtpSetcfg != NULL)
		memset(pSlotList->pPtpSetcfg, 0, sizeof(struct PtpSetCfg));
	else
		pSlotList->pPtpSetcfg = NULL;
    
    pSlotList->pPtpSetcfg_m = (struct PtpSetCfg*)malloc(sizeof(struct PtpSetCfg));
	if (pSlotList->pPtpSetcfg_m != NULL)
		memset(pSlotList->pPtpSetcfg_m, 0, sizeof(struct PtpSetCfg));
	else
		pSlotList->pPtpSetcfg_m = NULL;
      
    pSlotList->pPtpStatusCfg = (struct PtpStatusCfg*)malloc(sizeof(struct PtpStatusCfg));
	if (pSlotList->pPtpStatusCfg != NULL)
		memset(pSlotList->pPtpStatusCfg, 0, sizeof(struct PtpStatusCfg));
	else
		pSlotList->pPtpStatusCfg = NULL;
    
    InitSlotLocal(pSlotList);
    
    for(i = 1;i <= SLOT_COUNT;i++)
    {
        pRootData->slot_list[i].slot_type = 0;
        pRootData->slot_list[i].pNtpSetCfg = NULL;
        pRootData->slot_list[i].pPtpSetcfg = NULL;
        pRootData->slot_list[i].pPtpStatusCfg = NULL;
    }
    
    /** 1 Ê¹ÓÃOCXO */
    if(pClockInfo->clock_mode == 1)
        SetFpgaAddressVal(0x84,0x01);
    else
        SetFpgaAddressVal(0x84,0x00);

}

void InitConfigration(struct root_data *pRootData)
{
    
    strcpy(pRootData->ptp_port.ifaceName,"eth0");
    strcpy(pRootData->ntp_port.ifaceName,"eth1");
    strcpy(pRootData->comm_port.ifaceName,"eth2");
    
    Load_NetWorkParam_FromFile(ptpEthConfig,&pRootData->ptp_port);
    Load_NetWorkParam_FromFile(ntpEthConfig,&pRootData->ntp_port);
    Load_NetWorkParam_FromFile(ctlEthConfig,&pRootData->comm_port);

    Load_sys_configration(commSysConfig,pRootData);
	Load_Supervision_FromFile(supervisionConfig, &pRootData->supervision);
    
}

void InitDev(struct root_data *pRootData)
{    
    init_dev_head(&pRootData->dev_head);
    
    /**ENUM_LCD for LCD device, id =0*/
    pRootData->dev[ENUM_LCD].com_attr.com_port = ENUM_LCD; 
    pRootData->dev[ENUM_LCD].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_LCD],&pRootData->dev_head,COMM_DEVICE,ENUM_LCD);


#if 0
    pRootData->dev[ENUM_XO].com_attr.com_port = ENUM_XO;  //´®¿Ú2
    pRootData->dev[ENUM_XO].com_attr.baud_rate = 57600;
    init_add_dev(&pRootData->dev[ENUM_XO],&pRootData->dev_head,COMM_DEVICE,ENUM_XO);
#endif


    /**ENUM_GPS for gps device id=1  */
    pRootData->dev[ENUM_GPS].com_attr.com_port = ENUM_GPS;
    pRootData->dev[ENUM_GPS].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_GPS],&pRootData->dev_head,COMM_DEVICE,ENUM_GPS);
    
    /**ENUM_RB for RB clock ,device id=2  */
    pRootData->dev[ENUM_RB].com_attr.com_port = ENUM_RB;   
    pRootData->dev[ENUM_RB].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_RB],&pRootData->dev_head,COMM_DEVICE,ENUM_RB);

    //pRootData->dev[ENUM_PC_CTL].net_attr.sin_port = pRootData->ctl_sin_port;//
    //pRootData->dev[ENUM_PC_CTL].net_attr.ip = inet_addr(pRootData->ctl_ip_address);
    //init_add_dev(&pRootData->dev[ENUM_PC_CTL],&pRootData->dev_head,UDP_DEVICE,ENUM_PC_CTL);

    /**ENUM_IPC_PTP_MASTER for  PTP master, IPC communication  */
    pRootData->dev[ENUM_IPC_PTP_MASTER].net_attr.sin_port = 9900;
    pRootData->dev[ENUM_IPC_PTP_MASTER].net_attr.ip = inet_addr("127.0.0.1");

    /**ENUM_IPC_PTP_SLAVE for  PTP slave, IPC communication  */
    pRootData->dev[ENUM_IPC_PTP_SLAVE].net_attr.sin_port = 9901;
    pRootData->dev[ENUM_IPC_PTP_SLAVE].net_attr.ip = inet_addr("127.0.0.1");
    
    init_add_dev(&pRootData->dev[ENUM_IPC_PTP_MASTER],&pRootData->dev_head,INIT_DEVICE,ENUM_IPC_PTP_MASTER);
    init_add_dev(&pRootData->dev[ENUM_IPC_PTP_SLAVE],&pRootData->dev_head,UDP_DEVICE,ENUM_IPC_PTP_SLAVE);
    
    /**ENUM_IPC_NTP for  ntp, IPC communication  */
    pRootData->dev[ENUM_IPC_NTP].net_attr.sin_port = 9988;
    pRootData->dev[ENUM_IPC_NTP].net_attr.ip = inet_addr("127.0.0.1");
    init_add_dev(&pRootData->dev[ENUM_IPC_NTP],&pRootData->dev_head,UDP_DEVICE,ENUM_IPC_NTP);

}



void *DataHandle_Thread(void *arg)
{

	struct root_data *pRootData = (struct root_data *)arg;
	struct dev_head *p_dev_head = NULL;
	struct device *p_dev = NULL;
	struct data_list *p_data_list = NULL, *tmp_data_list = NULL;
	p_dev_head = (struct dev_head *)&pRootData->dev_head;

    int i;
	while(1)
    {
		sem_wait(&p_dev_head->sem[0]);
		list_for_each_entry(p_dev,&p_dev_head->list_head,list_head)	
		{
			list_for_each_entry_safe(p_data_list,tmp_data_list,&p_dev->data_head[0].list_head,list_head)
			{
				switch (p_dev->dev_id)
                {
    				case ENUM_BUS:
                        
                        #if 0
                        for(i=0;i<p_data_list->recv_lev.len;i++)
                            printf("%d ",p_data_list->recv_lev.data[i]);
                        printf("\n");           
                        ProcessUiData(pRoot,&p_data_list->recv_lev);
                        #endif
    					break;
                    case ENUM_LCD:
                         //printf("dev_id=  %d,com1\n",p_dev->dev_id);
                        ProcessLcdMessage(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);
                        break;
    				case ENUM_GPS:
                        //printf("dev_id=  %d,com1:receive:%s",p_dev->dev_id,p_data_list->recv_lev.data);
                        SatelliteHandle(&pRootData->satellite_data,p_data_list->recv_lev.data);
    					break;
    				case ENUM_RB:
                        
    					printf("dev_id=  %d,com2:receive:%s\n",p_dev->dev_id,p_data_list->recv_lev.data);
                        
                        break;
                    case ENUM_XO:
                        printf("dev_id=  %d,com2:receive:%s\n",p_dev->dev_id,p_data_list->recv_lev.data);
                        
                        break;
    				case ENUM_PC_CTL:					

                        printf("handle UDP pc_ctl_message %d\n",p_dev->dev_id);
                        //handle_pc_ctl_message(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);

                        break;
                    case ENUM_PC_DISCOVER:
                        printf("handle discovery %d\n",p_dev->dev_id);
                        //handle_discovery_message(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);
                        break;
    				case ENUM_LOCAL_DEV:
    					
    					break;
                    case ENUM_IPC_NTP:
                        //printf("ENUM_IPC_NTP %d\n",p_dev->dev_id);
						handle_ntp_data_message(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);
                        break; 
                    case ENUM_IPC_PTP_SLAVE:
                        //printf("handle_ptp_data_message dev_id=  %d,ptp recv\n",p_dev->dev_id);
                        handle_ptp_data_message(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);
                        break;
    				default:
    					break;
				}
				del_data(p_data_list, &p_dev->data_head[0], p_dev);
			}
		}
	}
	return NULL;
}
 
void *DataRecv_Thread(void *arg){
	struct device *p_dev = NULL, *tmp_dev = NULL;
	struct dev_head *p_dev_head = (struct dev_head *)arg;
    short ret;
    struct   timeval   timeout;   
 
	while(1)
    {
        timeout.tv_sec=1;   
        timeout.tv_usec=0;  
		p_dev_head->tmp_set = p_dev_head->fd_set;
        
        ret = select(p_dev_head->max_fd + 1,&p_dev_head->tmp_set, NULL, NULL,&timeout);
        if(ret <0)
        {
             perror("select fail ..!!\n");
        }
        else if(ret)
        {
            list_for_each_entry_safe(p_dev,tmp_dev,&p_dev_head->list_head,list_head)    
                p_dev->recv_data(p_dev, p_dev_head);

        }
        else
        {
            printf("select time out\n");
        }
        
	}
	return NULL ;
}




void *DataSend_Thread(void *arg) {
	int fd = -1, len = 0, count = 0;
	struct device *p_dev = NULL;
	struct data_list *p_data_list = NULL, *tmp_data_list = NULL, *p_net_data =NULL;
	struct dev_head *p_dev_head = (struct dev_head *) arg;
	while(1)
    {
		sem_wait(&p_dev_head->sem[1]);
		list_for_each_entry(p_dev,&p_dev_head->list_head,list_head)		
		{
            list_for_each_entry_safe(p_data_list,tmp_data_list,&p_dev->data_head[1].list_head,list_head)
            {
                
                if(p_dev->type == UDP_DEVICE)
                {

                    len = p_data_list->send_lev.len;
                    count = sendto(p_dev->fd,p_data_list->send_lev.data,len,0,(struct sockaddr *)&p_dev->dest_addr,sizeof(struct sockaddr_in));
                    
                    del_data(p_data_list, &p_dev->data_head[1],p_dev);
                }
                else if(p_dev->type == TCP_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    count = send(p_dev->fd, p_data_list->send_lev.data, len,0);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);

                }
                else if(p_dev->type == COMM_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    //printf("send =%d %x \n",len,p_data_list->send_lev.data[0]);
                    
                    count = write(p_dev->fd, p_data_list->send_lev.data, len);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);

                }
                else if(p_dev->type == INIT_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    count = sendto(p_dev->fd,p_data_list->send_lev.data,len,0,(struct sockaddr *)&p_dev->dest_addr,sizeof(struct sockaddr_in));
                    
                    del_data(p_data_list, &p_dev->data_head[1],p_dev);

                }
                else
                {
                    len = p_data_list->send_lev.len;
                    count = write(p_dev->fd, p_data_list->send_lev.data, len);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);
                    
                }
                
            }

        }
        
	}
	return NULL;
}

static void Pps_Signal_Handle(int signum)
{

    struct root_data *pRootData = g_RootData;
    struct clock_info *pClock_info = &pRootData->clock_info;
    struct collect_data *p_collect_data = &pClock_info->data_1Hz;
    struct clock_alarm_data *pClockAlarm = &pClock_info->alarmData;
    int phaseOffset = 0;
    int ph;
    
    if(pClock_info->ref_type == REF_SATLITE 
        && pClockAlarm->alarmBd1pps == FALSE
        && pClock_info->run_times > RUN_TIME)
    {

            phaseOffset = Get_Pps_Rb_PhaseOffset();
            phaseOffset = phaseOffset * 4;
            ph = Smooth_Filter(phaseOffset);
            printf("readPhase=%d collect_phase=%d, count=%d\n",phaseOffset,ph,p_collect_data->ph_number_counter);
            collect_phase(&pClock_info->data_1Hz,0,phaseOffset);      
    }

    g_RootData->flag_usuallyRoutine = TRUE;
}


void Init_PpsDev(char *dev_name)
{
    int fd;
    int Oflags; 
       
    fd = open(dev_name, O_RDWR);
    if(fd<0)
    {
        printf("can not find ptp_dev\n");
        return;
    }

    g_RootData->dev_fd = fd;

    fcntl(fd, F_SETOWN, getpid());    
    Oflags = fcntl(fd, F_GETFL);     
    fcntl(fd, F_SETFL, Oflags | FASYNC);
    
    signal(SIGIO, Pps_Signal_Handle);
}

void MaintainCoreTime(struct root_data *pRoot,TimeInternal *ptptTime)
{        
    struct clock_info *pClockInfo = &pRoot->clock_info;
    struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
    time_t current_time;
    struct tm *tm;
    struct timeval tv;
	
    static Uint32 previousBusSec = 0;
    Uint32 ntp_second_from_1970 =0;
    Uint32 Time = 0;
    static short  secErrorCnt = 0;
    static short  ntpsecErrorCnt = 0;
    static unsigned short modify_internal = 0;

    Uint32 fpga_ntp_second_from_1970 =0;
    struct Satellite_Data *pSateData = &pRoot->satellite_data;       

    int fd = pRoot->dev_fd;

    if((pClockInfo->ref_type ==  REF_SATLITE))
    {
        //printf("PPS TIme:%d, leap:%d\n",pSateData->time,pSateData->gps_utc_leaps);
        if(pClockInfo->pps_work == 0)
        {
            printf("GPS TIme:%d, leap:%d\n",pSateData->time+1,pSateData->gps_utc_leaps);
            
            Time = pSateData->time + pSateData->gps_utc_leaps;
            Time += 19;
            Time += 1;/**+1 ~{1mJ>51G0CkJ}~}  */

        }
        else if(pClockInfo->pps_work == 1)
        {
            
            printf("PPS TIme:%d, leap:%d\n",ptptTime->seconds,pSateData->gps_utc_leaps);
            Time= ptptTime->seconds;
        }
        else
        {
            Time= ptptTime->seconds;
        }
    }
    else if(pClockInfo->ref_type == REF_PTP)
    {
        Time= ptptTime->seconds;

    }
    else if(pClockInfo->ref_type == REF_NTP)
    {
		Time= ptptTime->seconds;

    }
	else
		Time= ptptTime->seconds;
	
   ntp_second_from_1970 = ioctl(fd,COMM_GET_SEC,0);
   current_time = ntp_second_from_1970;
   current_time += 28800;
   tm = gmtime(&current_time);
   memset(pRoot->current_time,0,sizeof(pRoot->current_time));
   sprintf(pRoot->current_time,"%d-%d-%d %02d:%02d:%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
   printf("%s\n",pRoot->current_time);

    if(abs(Time - previousBusSec) != 1)
    {
        printf("-------1------\n");
        previousBusSec = Time;
        return;
    }
    previousBusSec = Time;

    if(ptptTime->seconds != Time)
    {
        printf("------2-------\n");
        secErrorCnt++;
        if(secErrorCnt>5)
        {
            syslog(CLOG_DEBUG,"setFpgaTime=%d\n",Time);
            SetFpgaTime(Time);
            secErrorCnt = 0;
        }
    }
    else if(ptptTime->seconds == Time)
        secErrorCnt = 0;

    fpga_ntp_second_from_1970 = Time - pSateData->gps_utc_leaps;
    fpga_ntp_second_from_1970 -= 19;
    
    printf("ntp_second_from_1970=%d,fpga_ntp_second_from_1970=%d\n"
        ,ntp_second_from_1970
        ,fpga_ntp_second_from_1970);

    /**ntp do not == fgpa time (hardware time from RB clock)  */
    if(ntp_second_from_1970 != fpga_ntp_second_from_1970)
    {
        ntpsecErrorCnt++;
        if(ntpsecErrorCnt>5)
        {
            printf("------5-------\n");
            ioctl(fd,COMM_SET_SEC,fpga_ntp_second_from_1970);
            ioctl(fd,COMM_MODIFY_SEC,1);
            syslog(CLOG_DEBUG,"set systemtime-5-");

            ntpsecErrorCnt = 0;
        }

    }
    else if(ntp_second_from_1970 == fpga_ntp_second_from_1970)
    {
        modify_internal++;
        printf("------6-------\n");
        if(modify_internal%4)
            ioctl(fd,COMM_MODIFY_SEC,1);
        ntpsecErrorCnt = 0;

    }

            
}

void display_alarm_to_lcd(struct root_data *pRootData)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct clock_alarm_data *pClockAlarm = &pClockInfo->alarmData;
    if(pClockAlarm->alarmBd1pps == TRUE)
        SetTextValue(WARN_SCREEN_ID,4,"YES");
    else
        SetTextValue(WARN_SCREEN_ID,4,"NO");

    if(pClockAlarm->alarmPtp == TRUE)
        SetTextValue(WARN_SCREEN_ID,5,"YES");
    else
        SetTextValue(WARN_SCREEN_ID,5,"NO");

    if(pClockAlarm->alarmDisk == TRUE)
        SetTextValue(WARN_SCREEN_ID,6,"YES");
    else
        SetTextValue(WARN_SCREEN_ID,6,"NO");

	if (pClockAlarm->alarmNtp == TRUE)
		SetTextValue(WARN_SCREEN_ID, 9, "YES");
	else
		SetTextValue(WARN_SCREEN_ID, 9, "NO");

}


void display_clockstate_to_lcd(struct root_data *pRootData)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct Satellite_Data *p_satellite_data = &pRootData->satellite_data;

    char szbuf[50];

	if (pClockInfo->ref_type == REF_SATLITE)
	{
		switch (pClockInfo->workStatus)
		{
		case 0:
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 15, "FREE");
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 17, "NO");
			break;
		case 1:
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 15, "FAST");
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 17, "NO");
			break;
		case 2:
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 15, "LOCK");
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 17, "YES");
			break;
		case 3:
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 15, "HOLD");
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 17, "NO");
			break;
		default:
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 15, "ERROR");
			SetTextValue(CLOCK_STATUS_SCREEN_ID, 17, "NO");
			break;
		}

		memset(szbuf, 0, sizeof(szbuf));
		sprintf(szbuf, "%d", p_satellite_data->satellite_use);
		SetTextValue(CLOCK_STATUS_SCREEN_ID, 16, szbuf);

		memset(szbuf, 0, sizeof(szbuf));
		sprintf(szbuf, "%d", p_satellite_data->gps_utc_leaps);
		SetTextValue(CLOCK_STATUS_SCREEN_ID, 18, szbuf);
	}
	//else
		//SetTextValue(CLOCK_STATUS_SCREEN_ID, 12, "PTP");


	/*
    if(pClockInfo->ref_type == REF_SATLITE)
        SetTextValue(CLOCK_STATUS_SCREEN_ID,12,"GPS/BD");
    else
        SetTextValue(CLOCK_STATUS_SCREEN_ID,12,"PTP");

	
    switch(pClockInfo->workStatus)
    {
        case 0:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"FREE");
            SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"NO");
            break;
        case 1:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"FAST");
            SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"NO");
            break;
        case 2:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"LOCK");
            SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"YES");
            break;
        case 3:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"HOLD");
            SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"NO");
            break;
        default:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"ERROR");
            SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"NO");
            break;
    }

	
    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",p_satellite_data->satellite_see);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,15,szbuf);

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",p_satellite_data->satellite_use);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,16,szbuf);

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",p_satellite_data->gps_utc_leaps);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,18,szbuf);

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%c %d`%d'%d'' %c %d`%d'%d''"
        ,p_satellite_data->eastorwest,p_satellite_data->longitude_d,p_satellite_data->longitude_f,p_satellite_data->longitude_m
        ,p_satellite_data->northorsouth,p_satellite_data->latitude_d,p_satellite_data->latitude_f,p_satellite_data->latitude_m);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,19,szbuf);
	*/
    
}



void display_lcd_running_status(struct root_data *pRootData)
{
	char szbuf[100];
	int pRun_Day = 0;
	int pRun_Hour = 0;
	int pRun_Minute = 0;
	int pRun_Seconds = 0;

    switch(pRootData->lcd_sreen_id)
    {
        
        case MAIN_SCREEN_ID:
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
			memset(szbuf, 0, sizeof(szbuf));
			
			pRun_Day = pRootData->clock_info.run_times / (60 * 60 * 24);
			pRun_Hour = (pRootData->clock_info.run_times % (60 * 60 * 24)) / (60 * 60);
			pRun_Minute = (pRootData->clock_info.run_times % (60 * 60)) / 60;
			pRun_Seconds = pRootData->clock_info.run_times % 60;
			sprintf(szbuf, "%d:%s%d:%s%d:%s%d",
				pRun_Day,
				pRun_Hour < 10 ? "0" : "", pRun_Hour,
				pRun_Minute < 10 ? "0" : "", pRun_Minute,
				pRun_Seconds < 10 ? "0" : "", pRun_Seconds);
			SetTextValue(MAIN_SCREEN_ID, 16, szbuf);

            break;
        case CLOCK_STATUS_SCREEN_ID:
            display_clockstate_to_lcd(pRootData);
            break;
        case WARN_SCREEN_ID:
            display_alarm_to_lcd(pRootData);

            break;

        case ETH_CTL_SCREEN_ID:       
        case ETH_PTP_SCREEN_ID:      
        case ETH_NTP_SCREEN_ID:
            break;
        default:
            break;
    }
    
}



void display_alarm_information(struct clock_alarm_data *pClockAlarm)
{
   
    printf("alarmBd1pps=%d alarmPtp=%d alarmNtp=%d alarmVcxo100M=%d\n"
        ,pClockAlarm->alarmBd1pps,pClockAlarm->alarmPtp,pClockAlarm->alarmNtp,pClockAlarm->alarmVcxo100M);
    printf("alarmRb10M=%d alarmXo10M=%d alarmVcxoLock=%d\n"
        ,pClockAlarm->alarmRb10M,pClockAlarm->alarmXo10M,pClockAlarm->alarmVcxoLock);
    printf("alarmDisk=%d\n"
        ,pClockAlarm->alarmDisk);
}

void updatePtpStatus(struct root_data *pRootData,Uint16 nTimeCnt)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct Satellite_Data *pSateData = &pRootData->satellite_data;     
    struct PtpStatus mPtpStatus;

    if(nTimeCnt%10 == 0)
    {
        memset(&mPtpStatus,0,sizeof(mPtpStatus));
        switch(pClockInfo->workStatus)
        {
            case 0:

                mPtpStatus.blockOutput = 0;
                mPtpStatus.blockOutput = 0;
                mPtpStatus.priority1 = 127;
                mPtpStatus.priority2 = 127;
                mPtpStatus.timeSource = 0xa0;
                mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                mPtpStatus.clockClass = 248;
                mPtpStatus.clockAccuracy = 0x23;
                break;
            case 1:


                mPtpStatus.blockOutput = 0;
                mPtpStatus.priority1 = 127;
                mPtpStatus.priority2 = 127;
                mPtpStatus.timeSource = 0x20;
                mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                mPtpStatus.clockClass = 7;
                mPtpStatus.clockAccuracy = 0x21;

                break;
            case 2:


                mPtpStatus.blockOutput = 0;
                mPtpStatus.priority1 = 0;
                mPtpStatus.priority2 = 127;
                mPtpStatus.timeSource = 0x20;
                mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                mPtpStatus.clockClass = 6;
                mPtpStatus.clockAccuracy = 0x20;

                break;
            case 3:

                mPtpStatus.blockOutput = 0;
                mPtpStatus.priority1 = 127;
                mPtpStatus.priority2 = 127;
                mPtpStatus.timeSource = 0xa0;
                mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                mPtpStatus.clockClass = 248;
                mPtpStatus.clockAccuracy = 0x22;
                break;
           default:
                break;
        }

        AddData_ToSendList(pRootData,ENUM_IPC_PTP_MASTER,(void *)&mPtpStatus,sizeof(mPtpStatus));

    }
     
}

void *ThreadUsuallyProcess(void *arg)
{
    

    Uint32 UtcTime;
    TimeInternal timeTmp;
	pid_t status;
	char ntp_cmd[512];

    static Uint32 nTimeCnt = 0;
    struct root_data *pRootData = g_RootData;
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct clock_alarm_data *pClockAlarm = &pClockInfo->alarmData;
    struct Satellite_Data *pSateData = &pRootData->satellite_data;   
	struct sockaddr_in temsock;

	struct SuperVisionINI pSuperVision = pRootData->supervision;

    while(1)
    {
        /**1pps from kernel signal*/
        if(pRootData->flag_usuallyRoutine)
        {
            
            printf("========================1PPS Start ============================\n");

            /**Get hardware time from fpga  */
            GetFpgaPpsTime(&timeTmp); 

            /**main core time handle */
            MaintainCoreTime(pRootData,&timeTmp);

            CollectAlarm(pRootData);
            Display_SatelliteData(&pRootData->satellite_data);
            display_alarm_information(pClockAlarm);

            if(pClockInfo->clock_mode == 1)
                ClockStateProcess_OCXO(pClockInfo);
            else
                ClockStateProcess_RB(pClockInfo);

            display_lcd_running_status(pRootData);   
			           
            /**Running LED Handle */
            Control_LedRun(nTimeCnt%2);

            if(nTimeCnt%300 == 0)
                system("hwclock -w");
            
            if(pClockInfo->ref_type == REF_SATLITE)
            {
                if(pClockAlarm->alarmBd1pps == 1 || pClockAlarm->alarmDisk == 1)
                    Control_LedAlarm(0x01);
                else
                    Control_LedAlarm(0x00);
            }
            else if(pClockInfo->ref_type == REF_PTP)
            {
                if(pClockAlarm->alarmPtp == 1 || pClockAlarm->alarmDisk == 1)
                    Control_LedAlarm(0x01);
                else
                    Control_LedAlarm(0x00);
            }
			else if(pClockInfo->ref_type == REF_NTP)
			{
				  if(pClockAlarm->alarmNtp == 1 || pClockAlarm->alarmDisk == 1)
					  Control_LedAlarm(0x01);
				  else
					  Control_LedAlarm(0x00);
			}

            if(pClockAlarm->alarmBd1pps == 1)
                Control_LedSatStatus(0x00);
            else
                Control_LedSatStatus(0x01);

            if(pClockInfo->workStatus == LOCK)
                Control_LedLockStatus(0x01);
            else if(pClockInfo->workStatus == HOLD)
                Control_LedLockStatus(0x10);
            else/** fast and free */
                Control_LedLockStatus(0x00);

            updatePtpStatus(pRootData,nTimeCnt);
            
			if(pClockInfo->ref_type == REF_NTP)
			{
				if ((pRootData->ntp_ref_interval != 0) && (nTimeCnt % pRootData->ntp_ref_interval == 0))
				{
					memset(ntp_cmd, 0, 512);
					temsock.sin_addr.s_addr = pRootData->ntp_ref_server_ip;
                    //Bug_20180202 ntpdate path.
					sprintf(ntp_cmd, "/mnt/ntpdate %s &", inet_ntoa(temsock.sin_addr));
									
					status = system(ntp_cmd);
					
				}
				
			}
			
			nTimeCnt++;
            
            pRootData->flag_usuallyRoutine = FALSE;
        }

        /**Check whether REF time data is connected full */
        if(pClockInfo->clock_mode == 1)
            ClockHandleProcess_OCXO(pClockInfo);
        else
            ClockHandleProcess(pClockInfo);

        usleep(1000);
    }

    
}

static int print_usage(void) {
    printf("Usage: -i ethname(eth0, eth1)[options]\n");
    printf("\
Options:\n\
    -p Server port.\n\
    -d Running in deamon.\n\
    -l Log level.\n\
    -h Show usage.\n\
    -V Show version and exit.\n"); 
	return 0;
}

int daemonize(void)
{
    int pid, i;

    switch(fork())
    {
        /* fork error */
        case -1:
            perror("fork()");
            exit(1);

            /* child process */
        case 0:
            /* obtain a new process group */
            if( (pid = setsid()) < 0)
            {
                perror("setsid()");
                exit(1);
            }

            /* close all descriptors */
            for (i=getdtablesize();i>=0;--i) close(i);

            i = open("/dev/null", O_RDWR); /* open stdin */
            dup(i); /* stdout */
            dup(i); /* stderr */

            umask(027);
            chdir("/"); /* chdir to /tmp ? */

            return pid;

            /* parent process */
        default:
            exit(0);
    }
}

void IntExit(int a)
{
    system("pkill ptp");
    system("pkill ntpd");
    exit(-1);
}

void Wait_For_EnvConfig()
{
	struct timeval timeout;
	fd_set fs_read;
    
    int ret;
    int wait_cnt = 3;

    printf("\r\nWatting for Env Init finish... :   ");
    fflush(stdout);
    
    while(wait_cnt)
    {
        timeout.tv_sec = 1;									
	    timeout.tv_usec = 0;
        FD_ZERO(&fs_read);
        FD_SET(0,&fs_read);

        ret = select(1,&fs_read,NULL,NULL,&timeout);

        printf("\b\b%d ",wait_cnt);
        fflush(stdout);
        wait_cnt--;

    }

    printf("\n");
}

int main(int argc,char *argv[])
{
    int ret,err;
    pthread_attr_t threadAttr;
    pthread_t Id_ThreadUsually;
    int c;
    int val;
    printf("Hardware Version V1.00\n");
    printf("SoftWare Version V0.15\n");
    
    g_RootData = (struct root_data *)malloc(sizeof(struct root_data));
	if (g_RootData == NULL)
	{
		printf("malloc root_para error\n");
		return -1;
	}

	bzero(g_RootData,sizeof(struct root_data));
    g_RootData->ctl_sin_port = 20170;
    
    while((c = getopt(argc, argv, "dVhp:l:f:")) != -1)
    {
        switch(c)
        {
            case 'd':
                daemonize();
                break;
            case 'p':
              
                printf("the port is %s\n", optarg);
                g_RootData->ctl_sin_port = atoi(optarg);
                break;
            case 'V':

                break;
            case 'l':
                break;

            case 'h':
            default:
                print_usage();
                exit(0);
        }
    }
    InitConfigration(g_RootData);
    Init_RbClockCenter(&g_RootData->clock_info);
    Init_FpgaCore();
    Init_Thread_Attr(&g_RootData->pattr);
    Init_PpsDev("/dev/ptp_dev");
    InitDev(g_RootData);
    InitSlotList(g_RootData);    
    signal(SIGINT, IntExit);
    Wait_For_EnvConfig();

    syslog_init();

    start_ntp_daemon();
    start_ptp_daemon();

    InitLoadRtcTime(g_RootData);

    syslog(CLOG_DEBUG,"tsync deamon starting running\n");
    
    /** ThreadUsuallyProcess thread for ususal handle */
	err = pthread_create(&g_RootData->p_usual,&g_RootData->pattr,(void *)ThreadUsuallyProcess,(void *)g_RootData);	
	if(err == 0)
	{
		printf("dailymanage_pthread succes!\n");
	}
	else
	{
		printf("dailymanage_pthread error!\n");
		return FALSE;
	}

    /** DataRecv_Thread thread for recvdata handle */
	err = pthread_create(&g_RootData->p_recv,&g_RootData->pattr,(void *)DataRecv_Thread,(void *)&g_RootData->dev_head);	
	if(err == 0)
	{
		printf("datarecv_pthread succes!\n");
	}
	else
	{
		printf("datarecv_pthread error!\n");
		return FALSE;
	}

    /** DataSend_Thread thread for senddata handle */
	err = pthread_create(&g_RootData->p_send,&g_RootData->pattr,(void *)DataSend_Thread,(void *)&g_RootData->dev_head);
    if(err == 0)
    {
		printf("datasend_pthread succes!\n");
	}
    else
	{
		printf("datasend_pthread error!\n");
		return FALSE;
	}

    /** DataSend_Thread thread for data process handle */
	err = pthread_create(&g_RootData->p_handle,&g_RootData->pattr,(void *)DataHandle_Thread,(void *)g_RootData);
    if(err == 0)
    {
		printf("datahandle_pthread succes!\n");
	}else
	{
		printf("datahandle_pthread error!\n");
		return FALSE;
	}

	Init_ublox(g_RootData);

    pthread_join(g_RootData->p_usual,NULL);
    pthread_join(g_RootData->p_recv,NULL);
    pthread_join(g_RootData->p_send,NULL);
    pthread_join(g_RootData->p_handle,NULL);
    

	return TRUE;
}

