/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       clock_rb.c
*    功能描述:       钟控模块，控制铷钟，状态机控制，收集鉴相值
*    作者:           Felix
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2017-7-29
*    函数列表:
                     Calman_FilterMeanPath
                     collect_ph
                     e1_det_para
                     Filter
                     gps_init
                     gps_status_handle
                     gps_status_machine
                     rb_fast_handle
                     rb_lock_handle
                     RequestRbClockStatus
                     send_clock_data
                     SetRbClockWorkMode
                     xor
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-7-29
                     作者: Felix
                     修改内容: 新创建文件


*********************************************************************************************/

#include "clock_rb.h"
#include "main.h"
#include "log.h"

char *rbCenterCfg = "/mnt/rb_center.cfg";


int Filter(int val) 
{
  int i;
  static int filter_buf[10 + 1];
  int filter_sum = 0;
   filter_buf[10] = val;
  for(i = 0; i < 10; i++)
   {
     filter_buf[i] = filter_buf[i + 1];    // 所有数据左移，低位仍掉
     filter_sum += filter_buf[i];
  } 
   return (int)(filter_sum / 10);
}

 int Save_RbSteer_ToFile(struct clock_info *pClockInfo)
{
    Uint8 line_str[200];
    Uint16 i,k;
    Uint8 text[RB_DATA_LINE][1024];
    
    struct collect_data *p_collect_data = &pClockInfo->data_1Hz;
    struct Satellite_Data *pSateData = &g_RootData->satellite_data;
    
    struct tm *tm;
    time_t seconds = time(NULL);
    
    FILE *rb_file_fd = fopen(RB_STEER_FILE,"a+");
    if(rb_file_fd == NULL)
    {
      printf("cannot find rbdata file!\n");
      return -1;
    }

    memset(text,0,sizeof(text));
    fseek(rb_file_fd,0L,SEEK_SET);
    
    i=0;
    while(fgets(text[i],sizeof(text[i]),rb_file_fd)) //41行(次)
    {
        i = (i + 1) % RB_DATA_LINE; //0~39~0
    }
    fclose(rb_file_fd);
    
    rb_file_fd = fopen(RB_STEER_FILE,"w+"); //新开
    if(rb_file_fd == NULL)
    {
      printf("cannot find rbdata file!\n");
      return -1;
    }

    for(k=0; k<RB_DATA_LINE; k++)
    {
        fputs(text[i],rb_file_fd);
        i = (i + 1) % RB_DATA_LINE; //1~39~0
    }
    
    tm = gmtime(&seconds);
    
    memset(line_str,0,sizeof(line_str));
    sprintf(line_str,"center:%8d workStatus:%d \
%2d %2d \
%2d %2d \
%04d/%02d/%02d %02d:%02d:%02d\n"
      ,pClockInfo->center,pClockInfo->workStatus
      ,p_collect_data->ph_number,p_collect_data->queue_number
      ,pClockInfo->lockCounter,pClockInfo->unlockCounter
      ,tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    fputs(line_str, rb_file_fd); //40
    
    fflush(rb_file_fd);
    fclose(rb_file_fd);

    return TRUE;
}


long int Calman_FilterMeanPath(long int val,char clean)
{
    
    /**卡尔曼滤波算法相关变量 */
    static float Km;    		 /*kalman系数*/
    static float Rm=0.1;       
    static float Qm=0.0001;    		 /*过程噪声方差*/
    static float Xk;		/*状态变量*/
    static float Xk_1=0;    /*前一状态变量*/
    static float Pk;        /*状态变量估计误差的协方差*/
    static float Pk_1=1;    /*前一状态变量估计误差的协方差*/

	Xk=Xk_1;
	Pk=Pk_1+Qm;

	Km=Pk/(Pk+Rm);
		
	Xk_1=Xk+Km*(val-Xk);
		
	Pk_1=(1-Km)*Pk;

	val=Xk_1;
     if(clean==0)
	{
		Km=0;
		Rm=0.1;
		Qm=0.0001;
		Xk=0;
		Xk_1=0;
		Pk_1=1;
		Pk=0;
	}
	return val;
}


void Init_RbClockCenter(struct clock_info *pClock_info)
{
    pClock_info->center = RB_CENTER_VALUE;
    
   
}

void Write_RbCLockCenter(struct clock_info *pClock_info)
{
    int fd = open(rbCenterCfg,O_RDWR | O_CREAT);
    write(fd,&pClock_info->center,4);
    printf("store center = %d\n",pClock_info->center);
    close(fd);
    
}

char xor(char *string,char n)
{   
    char i=0,result=0;
    for(i=0;i<n;i++)
      result^=*(string+i);
     return result; 


}
void read_clock_ver(void)
{ 
   char send_buf[10]={0xF0,0x04,0x00,0xF4};

   AddData_ToSendList(g_RootData,ENUM_RB,send_buf,4);

   printf("read ver!!!!   \r\n");

}

 
  /**
  * @brief  通过串口发送数据到铷钟
  * @param  None
  * @retval None
  */
void send_clock_data(struct clock_info *p_clock_info,int data)
{
   short i = 0;
   
   char send_buf[10]={0x2e,0x09,0x00,0x27};

#if 0
   data = data / 1000;
   data  = data * 1000;
#endif

   printf("send_clock_data center=%d\n",data);

   send_buf[4] = data>>24;
   send_buf[5] = data>>16;
   send_buf[6] = data>>8;
   send_buf[7] = data;

   send_buf[8] = xor(&send_buf[4],4);
      
   p_clock_info->time_now =time(NULL);
    
   /*调用发送函数*/
   AddData_ToSendList(g_RootData,ENUM_RB,send_buf,9);

}


  
  /**
  * @brief  GPS队列数据初始化
  * @param  None
  * @retval None
  */
void gps_init(struct clock_info *p_clock_info)
{
   struct collect_data *p_collect_data=&p_clock_info->data_1Hz;   
   p_clock_info->bInitialClear=1;
   //p_clock_info->syn_enable = 1;
   p_collect_data->ph_max=0;
   p_collect_data->ph_min=0;
   p_collect_data->ph_number=12;
   p_collect_data->queue_number=20;
   p_collect_data->add_ph=0;
   p_collect_data->ph_number_counter=0;
   p_collect_data->queue_number_counter=0;
   p_collect_data->getdata_flag=0;
   p_clock_info->lAccPhaseAll=0;

}

void m10ref_init(struct clock_info *p_clock_info)
{
   struct collect_data *p_collect_data=&p_clock_info->data_1Hz;   
   p_clock_info->bInitialClear=1;
   p_collect_data->ph_max=0;
   p_collect_data->ph_min=0;
   p_collect_data->ph_number=12;
   p_collect_data->queue_number=20;
   p_collect_data->add_ph=0;
   p_collect_data->ph_number_counter=0;
   p_collect_data->queue_number_counter=0;
   p_collect_data->getdata_flag=0;
   p_clock_info->lAccPhaseAll=0;

}
int Kalman_Filter(int val,char clean)
{
    
    /**卡尔曼滤波算法相关变量 */
    static double Km;             /*kalman系数*/
    static double Rm=0.1;       
    static double Qm=0.0001;          /*过程噪声方差*/
    static double Xk;        /*状态变量*/
    static double Xk_1=0;    /*前一状态变量*/
    static double Pk;        /*状态变量估计误差的协方差*/
    static double Pk_1=1;    /*前一状态变量估计误差的协方差*/

    Xk=Xk_1;
    Pk=Pk_1+Qm;

    Km=Pk/(Pk+Rm);
        
    Xk_1=Xk+Km*(val-Xk);
        
    Pk_1=(1-Km)*Pk;

    val=Xk_1;
     if(clean==0)
    {
        Km=0;
        Rm=0.1;
        Qm=0.0001;
        Xk=0;
        Xk_1=0;
        Pk_1=1;
        Pk=0;
    }
    return (int)val;
}

int Smooth_Filter(int val)
{
	
	static int previous = 0;;			 

	val = val + previous;
	val = val / 2;
	
	previous = val;
	
	return (int)val;
}





    /**
  * @brief  PH数据收集
  * @param  struct clock_info *   、时延(E1收集时延为0)、鉴相值(10ns)、
  * @retval None
  */
void collect_phase(struct clock_info *pClockInfo,struct collect_data *p_collect_data,int delay,int ph)
{
     int ph_temp=0;
    int temp=0;
    float sum=0;

    if(p_collect_data->clear_flag == 0)   /** 获得基准ph */
    {
        printf("get new ph_base\n");
        p_collect_data->ph_base = ph;
        p_collect_data->add_ph=0;
        pClockInfo->lPhasePrevious = 0;
        p_collect_data->queue_number_counter=0;
        p_collect_data->ph_number_counter=0;
        p_collect_data->clear_flag = 1;
        return;
    }

    ph_temp = ph - p_collect_data->ph_base;  /*延时补偿*/
    printf("collect_phase= %d count= %d ph_base= %d \n",ph,p_collect_data->queue_number_counter,p_collect_data->ph_base);

    p_collect_data->ph_number_counter++;

    if(p_collect_data->ph_number_counter >= p_collect_data->ph_number)   /*记数满一组 11*/
    {
          p_collect_data->phase_array[p_collect_data->queue_number_counter] = p_collect_data->add_ph;
          syslog(CLOG_DEBUG,"addph[%d] = %d \n", p_collect_data->queue_number_counter,p_collect_data->add_ph);

          p_collect_data->queue_number_counter++;
          p_collect_data->add_ph=0;
          p_collect_data->ph_number_counter=0;   /*本组记数清0*/
          
          if( p_collect_data->queue_number_counter >= p_collect_data->queue_number)  /*队列接收一组数据 20*/
          {
              for(temp=0; temp<p_collect_data->queue_number_counter; temp++)
              {
                  sum += p_collect_data->phase_array[temp];
              }
              p_collect_data->lAvgPhase = sum / (p_collect_data->ph_number);      /*计算平均相位值*/
              printf("sum %f  avg_current = %f \n", sum,p_collect_data->lAvgPhase);
              p_collect_data->queue_number_counter = 0;/*记数清0*/
              p_collect_data->getdata_flag = 1;
          }
    }


}
  /**
  * @brief  1Hz fast 处理
  * @param  None
  * @retval None
  */
void m10_fast_handle(struct clock_info *p_clock_info)
{   
    float everySecOffset;
        
    p_clock_info->IDetPhase= p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious;
    
    printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->lAccPhaseAll,p_clock_info->bInitialClear);


    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->OffsetAll = 0;
        p_clock_info->lAccPhaseAll = 0;
        p_clock_info->IDetPhase=0;

    
        if(p_clock_info->lPhasePrevious >2000.0 || p_clock_info->lPhasePrevious < -2000.0)
        {
            SetRbClockAlign_Once();
            p_clock_info->bInitialClear = 1;
        }
           
    }
    else
    {
        if((p_clock_info->data_1Hz.lAvgPhase>2000)||(p_clock_info->data_1Hz.lAvgPhase<-2000))
        {
             //adj
             SetRbClockAlign_Once();
             p_clock_info->bInitialClear=1;
             printf("0 adj lAvgPhase=%d \r\n",p_clock_info->data_1Hz.lAvgPhase);

        }
    
        if((p_clock_info->IDetPhase>-500)&&(p_clock_info->IDetPhase<500))  //p_clock_info->lDetDdsAdj
        {

            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter++;        

        }
        else
        {
           SetRbClockAlign_Once();
           if(p_clock_info->lockCounter>1)
                p_clock_info->lockCounter--; 
        }  
    }
    p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;

 
}


void m10_lock_handle(struct clock_info *p_clock_info)
{   

    float everySecOffset;

    p_clock_info->IDetPhase = (p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious);
    
	printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->OffsetAll,p_clock_info->bInitialClear);
   

    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->OffsetAll = 0;
        p_clock_info->lAccPhaseAll = 0;
        p_clock_info->IDetPhase=0;

    }
    else
    {
        if((p_clock_info->data_1Hz.lAvgPhase>1000)||(p_clock_info->data_1Hz.lAvgPhase<-1000))
        {
             //adj
             SetRbClockAlign_Once();
             p_clock_info->bInitialClear=1;
             

        }
    
        if((p_clock_info->IDetPhase>-500)&&(p_clock_info->IDetPhase<500))  //p_clock_info->lDetDdsAdj
        {
            
            if(p_clock_info->unlockCounter>1) 
                p_clock_info->unlockCounter--;
        }
        else
        {
            SetRbClockAlign_Once();
            if(p_clock_info->unlockCounter<50)
                p_clock_info->unlockCounter++;    
        }

    }
    p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;

}



void ptp_fast_handle(struct clock_info *p_clock_info)
{   
    float everySecOffset;
    p_clock_info->IDetPhase= p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious;
    //p_clock_info->lAccPhaseAll += p_clock_info->IDetPhase;
    
	printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->lAccPhaseAll,p_clock_info->bInitialClear);

    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->lAccPhaseAll = 0;
        p_clock_info->lAccPhaseAll = 0;
	    p_clock_info->IDetPhase=0;

        if(p_clock_info->data_1Hz.lAvgPhase >300.0 || p_clock_info->data_1Hz.lAvgPhase < -300.0)
        {
            SetFpgaAdjustPhase(-(p_clock_info->data_1Hz.phase_offset));

            p_clock_info->bInitialClear = 1;
        }
		
    }
    else
    {
        if((p_clock_info->IDetPhase>-10)&&(p_clock_info->IDetPhase<10))
        {

            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter++;        

        }
		else
        {
           if(p_clock_info->lockCounter>1)
                p_clock_info->lockCounter--; 
        }  

        p_clock_info->lDetDdsAdj= PtpFastG1*p_clock_info->IDetPhase;
		printf("lDetDdsAdj=%f  \r\n", p_clock_info->lDetDdsAdj);

        if((int)p_clock_info->lDetDdsAdj >PtpFastConstraint)
            p_clock_info->lDetDdsAdj=PtpFastConstraint;
        else if ((int)p_clock_info->lDetDdsAdj<-PtpFastConstraint)
            p_clock_info->lDetDdsAdj=-PtpFastConstraint;
    
        if((p_clock_info->data_1Hz.lAvgPhase>300.0)||(p_clock_info->data_1Hz.lAvgPhase<-300.0))
        {
          	 //adj
			 SetFpgaAdjustPhase(-p_clock_info->data_1Hz.phase_offset);
      		 p_clock_info->bInitialClear=1;
			 printf("0 adj lAvgPhase=%d \r\n",p_clock_info->data_1Hz.lAvgPhase);

        }


    }
    p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;
    if(p_clock_info->lDetDdsAdj!=0)
    {
      p_clock_info->adjflag=0x01;
      p_clock_info->center = p_clock_info->center - (int)p_clock_info->lDetDdsAdj;    //计算中心值
      printf("lDetDdsAdj= %f   lockCounter= %d\r\n",p_clock_info->lDetDdsAdj,p_clock_info->lockCounter);
      send_clock_data(p_clock_info,p_clock_info->center);  /*添加写钟控值*/

    } 
}
  
void ptp_lock_handle(struct clock_info *p_clock_info)
{   

    p_clock_info->IDetPhase = (p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious);
    
    printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->lAccPhaseAll,p_clock_info->bInitialClear);

    if(p_clock_info->lAccPhaseAll>GpsAccConstraint)
       p_clock_info->lAccPhaseAll=GpsAccConstraint;
    else if((p_clock_info->lAccPhaseAll<-GpsAccConstraint))  
       p_clock_info->lAccPhaseAll=-GpsAccConstraint;


    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->lAccPhaseAll=0;
        p_clock_info->IDetPhase=0;
        p_clock_info->lAccPhaseAll = 0;
        
        if(p_clock_info->data_1Hz.lAvgPhase >200.0 || p_clock_info->data_1Hz.lAvgPhase < -2000.0)
        {
            SetFpgaAdjustPhase(-(p_clock_info->data_1Hz.phase_offset));

            p_clock_info->bInitialClear = 1;
        }
        
    }
    else
    {
        p_clock_info->lDetDdsAdj=PtpLockG1* p_clock_info->IDetPhase;
        //p_clock_info->lDetDdsAdj+=p_clock_info->data_1Hz.lAvgPhase/GpsLockG2;
        
        printf("IDetPhase=%f lAvgPhase=%f \r\n ",p_clock_info->IDetPhase,p_clock_info->data_1Hz.lAvgPhase);


        if((p_clock_info->data_1Hz.lAvgPhase>200.0)||(p_clock_info->data_1Hz.lAvgPhase<-2000.0))
        {
             //adj
             SetFpgaAdjustPhase(-p_clock_info->data_1Hz.phase_offset);
             p_clock_info->bInitialClear=1;
             printf("0 adj lAvgPhase=%d \r\n",p_clock_info->data_1Hz.lAvgPhase);

             if(p_clock_info->unlockCounter<50)
                p_clock_info->unlockCounter++;

        }
        else
        {

            printf("lock adj lAvgPhase=%f \r\n",p_clock_info->data_1Hz.lAvgPhase);

            if(p_clock_info->unlockCounter>1) 
                p_clock_info->unlockCounter--;

        }

        if(p_clock_info->lDetDdsAdj<-PtpLockConstraint) 
           p_clock_info->lDetDdsAdj=-PtpLockConstraint;
        else if(p_clock_info->lDetDdsAdj>PtpLockConstraint)
           p_clock_info->lDetDdsAdj=PtpLockConstraint;
    }
    p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;
    if(p_clock_info->lDetDdsAdj!=0)
    {
      p_clock_info->adjflag=0x01;
      p_clock_info->center = p_clock_info->center - (int)p_clock_info->lDetDdsAdj;    //计算中心值
      printf("lDetDdsAdj: %f  unlockCounter=%d\r\n",p_clock_info->lDetDdsAdj,p_clock_info->unlockCounter);
      send_clock_data(p_clock_info,p_clock_info->center);  /*添加写钟控值*/
    }
    
}


  
  void rb_fast_handle(struct clock_info *p_clock_info)
  {
      float everySecOffset;
      struct collect_data *p_collect_data = &p_clock_info->data_1Hz;
      
      if(p_clock_info->bInitialClear==1)  /*有调相动作*/
      {
              p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;
              printf("1-lPhasePrevious get new. lPhasePrevious %f\n",p_clock_info->lPhasePrevious);
              p_clock_info->bInitialClear=0;
              return;
      }
      else
      {
          p_clock_info->IDetPhase = p_clock_info->data_1Hz.lAvgPhase - p_clock_info->lPhasePrevious;
          printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f \r\n"
              ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious,p_clock_info->IDetPhase);
  
          if(fabs(p_clock_info->data_1Hz.lAvgPhase) > 1000.0)   //10000.0
          {
              //adj
          p_clock_info->syn_enable=1;
          //p_RbData->bInitialClear=1;
          printf("0_adj _lAvgPhase=%f \r\n",p_clock_info->data_1Hz.lAvgPhase);
          }
  
          if(fabs(p_clock_info->IDetPhase) < 30.0)
          {
                  if(p_clock_info->lockCounter<10)
                  {
                         p_clock_info->lockCounter++; //>=2
                  }
                  p_clock_info->SmoothStage++;
                  if((p_clock_info->SmoothStage == 3) && (p_clock_info->lockCounter < 3))
                      p_clock_info->lockCounter = 3;
                  
          }
          
          else if(fabs(p_clock_info->IDetPhase) <= 65.0)  //p_clock_info->lDetDdsAdj
          {
                  if(fabs(p_clock_info->data_1Hz.lAvgPhase) > 1000.0)  //
                  {
                      // fpga_write_byte(ALINE_1HZ_REG,0x55); /*进行对齐*/
                       p_clock_info->syn_enable=1;
                       //p_RbData->bInitialClear=1;
                  }
  
                   if(p_clock_info->lockCounter<10)
                   {
                          p_clock_info->lockCounter++; //>=2
                   }
                   p_clock_info->SmoothStage = 0;
          }
          else
          {
             if(p_clock_info->lockCounter>1)
             {
                     //p_clock_info->minus_flag=0;
                   p_clock_info->lockCounter--;
             }
             p_clock_info->SmoothStage = 0;
          }
          
          p_clock_info->lDetDdsAdj= GpsFastG1 * p_clock_info->IDetPhase / p_clock_info->lockCounter;
          printf("lDetDdsAdj=%f  \r\n", p_clock_info->lDetDdsAdj);
      }
  
      p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;
      
      if((int)p_clock_info->lDetDdsAdj>GpsFastConstraint)
          p_clock_info->lDetDdsAdj=GpsFastConstraint;
      else if ((int)p_clock_info->lDetDdsAdj<-GpsFastConstraint)
          p_clock_info->lDetDdsAdj=-GpsFastConstraint;
      
      if(p_clock_info->lDetDdsAdj != 0.0)
      {
          //p_clock_info->adjflag=0x01;
          printf("lockCounter= %d ",p_clock_info->lockCounter);
          send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);  /*写钟控值*/
           Save_RbSteer_ToFile(p_clock_info);
          //p_RbData->flag=1;
      }
      //log_debug("2 lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f\r\n",p_RbData->data.lAvgPhase,p_RbData->Previousph,p_RbData->IDetPhase);
  }


  /**
  * @brief  1Hz lock 处理
  * @param  None
  * @retval None
  */
  void rb_lock_handle(struct clock_info *p_clock_info)
  {
      struct collect_data *p_collect_data = &p_clock_info->data_1Hz;
    //  p_clock_info->IDetPhase= p_clock_info->IDetPhase/2+(p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious)/2;
    //  p_clock_info->lAccPhaseAll+=p_clock_info->data_1Hz.lAvgPhase;
  
      p_clock_info->IDetPhase=p_clock_info->data_1Hz.lAvgPhase - p_clock_info->lPhasePrevious;//h
      p_clock_info->lAccPhaseAll+=p_clock_info->IDetPhase;
  
      printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f   lAccPhaseAll= %f \r\n"
                ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious,p_clock_info->IDetPhase,p_clock_info->lAccPhaseAll);
  
      if(p_clock_info->lAccPhaseAll>GpsAccConstraint)
          p_clock_info->lAccPhaseAll=GpsAccConstraint;
      else if((p_clock_info->lAccPhaseAll<-GpsAccConstraint))
          p_clock_info->lAccPhaseAll=-GpsAccConstraint;
  
      if(p_clock_info->bInitialClear==1)  /*有调相动作*/
      {
          p_clock_info->lDetDdsAdj=0;
          p_clock_info->lAccPhaseAll=0;
          p_clock_info->IDetPhase=0;
          p_clock_info->bInitialClear=0;
      }
      else        // GpsLockG1* IDetPhase+lAvgPhase/GpsLockG2+lAccPhaseAll/GpsLockG3
      {
          p_clock_info->lDetDdsAdj=GpsLockG1 * p_clock_info->IDetPhase * p_clock_info->unlockCounter;   /** 放后面 */
          //p_RbData->lDetDdsAdj+=p_RbData->data.lAvgPhase/GpsLockG2;
          //p_RbData->lDetDdsAdj+=p_RbData->lAccPhaseAll/GpsLockG3;
          //printf("IDetPhase=%f lAvgPhase=%f  lAccPhaseAll=%f  \r\n ",p_RbData->IDetPhase,p_RbData->data.lAvgPhase,p_RbData->lAccPhaseAll);
  
          if(fabs(p_clock_info->data_1Hz.lAvgPhase) > 900.0)     //2000hg700
          {
              if(fabs(p_clock_info->IDetPhase) < 100.0)
              {
               //fpga_write_byte(ALINE_1HZ_REG,0x55);
                  //p_RbData->syn_1Hz=1;
                  //p_RbData->bInitialClear=1;
                  printf("lock adj lAvgPhase=%f \r\n",p_clock_info->data_1Hz.lAvgPhase);
               
                  if((fabs(p_clock_info->IDetPhase) < 40.0) && (p_clock_info->unlockCounter>1))
                        p_clock_info->unlockCounter--;
              }
              else if(p_clock_info->unlockCounter<25)
                  p_clock_info->unlockCounter++;
          }
          else if(p_clock_info->unlockCounter>1)
              p_clock_info->unlockCounter--;
  
          if(p_clock_info->lDetDdsAdj<-GpsLockConstraint)
              p_clock_info->lDetDdsAdj=-GpsLockConstraint;
          else if(p_clock_info->lDetDdsAdj>GpsLockConstraint)
              p_clock_info->lDetDdsAdj=GpsLockConstraint;
  
      }
      
      p_clock_info->lPhasePrevious=p_clock_info->data_1Hz.lAvgPhase;
      
      if(p_clock_info->lDetDdsAdj != 0.0)
      {
          //p_clock_info->adjflag=0x01;
          printf("unlockCounter=%d ",p_clock_info->unlockCounter);
          send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);  /*写钟控值*/
          Save_RbSteer_ToFile(p_clock_info);
      }
  }



  /**
  * @brief  1Hz状态机处理 (每秒调用一次,配置参考源为1PPS时，调用)
  * @param  None
  * @retval None
  */

void ptp_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    
    if(p_clock_info->ref_change_flag == TRUE)  /*同类参考源变化丢弃当前数据*/
    {
      p_clock_info->bInitialClear=1; /*标记*/
      //p_clock_info->syn_enable = 1;
      p_clock_info->data_1Hz.add_ph=0;
      p_clock_info->data_1Hz.ph_number_counter=0;
      p_clock_info->data_1Hz.queue_number_counter=0;
      p_clock_info->ref_change_flag = FALSE;
    }


    //printf("workStatus= %d \r\n",p_clock_info->workStatus);
    switch(p_clock_info->workStatus)
    {
        case FREE:
        {
             if(ref_status==1)   /*参考源正常*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }
        }
        break;
        case FAST:
        {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus=HOLD; 
            else if((ref_status==1)&&(p_clock_info->lockCounter>=4))
            {
               p_clock_info->workStatus=LOCK;
               p_clock_info->unlockCounter=0;
             }
            p_clock_info->fast_times++;/*快捕用时*/
        }       
            break;
        case LOCK:  
         {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus= HOLD;
            else if ((ref_status==1)&&(p_clock_info->unlockCounter>=3))
            {
               p_clock_info->workStatus= FAST;
               p_clock_info->lockCounter=0;
            }
            p_clock_info->lock_times++;/*锁定用时*/
            if((ref_status == 1) && ((p_clock_info->lock_times%86400) == 0))
            {
                Write_RbCLockCenter(p_clock_info);
            }
         }
            break;
        case HOLD:  
        {
            p_clock_info->hold_times++;/*锁定用时*/
           if(p_clock_info->hold_times>=86400) /*保持时间大于1天, 无参考源 进入自由运行*/
           {
                p_clock_info->workStatus= FREE;
           }else if((p_clock_info->hold_times<86400)&&(ref_status==1)&&(p_clock_info->lockCounter>=4))
           {
                p_clock_info->workStatus=LOCK;
           }
           else if((p_clock_info->hold_times<86400)&&(ref_status==1)&&(p_clock_info->lockCounter<4))
           {
               p_clock_info->workStatus=FAST;

           }
           /*保持时间小于1于，参考源有效，进入锁定*/
        }   
            break;
            
        default:    
            
            break;
     }

    if(status!=p_clock_info->workStatus)    /*工作状态变化*/
    {
        printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        syslog(CLOG_DEBUG,"workStatus change  workStatus= %d\n",p_clock_info->workStatus);

        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          gps_init(p_clock_info);
           /**快捕状态下，每10个收集一组，收集20组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=30;
          p_clock_info->lockCounter=0;
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          gps_init(p_clock_info);

          /**锁定状态下，没10个收集一组，收集50组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=50;    //50  20h
          p_clock_info->lock_times=0;

         break;
        case HOLD:   
            p_clock_info->hold_times=0;
            p_clock_info->unlockCounter=0;
            p_clock_info->lockCounter=0;
            break;
        default :
            break;
        
        }
        status=p_clock_info->workStatus;  /*记录当前工作状态*/
    }

}

void ntp_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    
    if(p_clock_info->ref_change_flag == TRUE)  /*同类参考源变化丢弃当前数据*/
    {
      p_clock_info->bInitialClear=1; /*标记*/
      //p_clock_info->syn_enable = 1;
      p_clock_info->data_1Hz.add_ph=0;
      p_clock_info->data_1Hz.ph_number_counter=0;
      p_clock_info->data_1Hz.queue_number_counter=0;
      p_clock_info->ref_change_flag = FALSE;
    }


	//printf("workStatus= %d \r\n",p_clock_info->workStatus);
    switch(p_clock_info->workStatus)
    {
		case FREE:
        {
             if(ref_status==1)   /*参考源正常*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }
        }
      	    break;
     	case FAST:
        {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus=HOLD; 
            else if((ref_status==1)&&(p_clock_info->fast_times>=300))
			{
               p_clock_info->workStatus=LOCK;
			   p_clock_info->unlockCounter=0;
			 }
            p_clock_info->fast_times++;/*快捕用时*/
        }
       		break;
     	case LOCK:	
	     {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus= HOLD;
             p_clock_info->lock_times++;/*锁定用时*/
             
             if(p_clock_info->lock_times % 3600 == 0)
             {
                 system("hwclock -w");

             }
         }
      		break;
     	case HOLD:	
        {
            p_clock_info->hold_times++;/*锁定用时*/
           if(p_clock_info->hold_times>=86400) /*保持时间大于1天, 无参考源 进入自由运行*/
           {
                p_clock_info->workStatus= FREE;
           }
           else if((p_clock_info->hold_times<86400)&&(ref_status==1))
           {
               p_clock_info->workStatus=FAST;

           }
           /*保持时间小于1于，参考源有效，进入锁定*/
        }	
      		break;
            
    	default:    
			
     		break;
     }

    if(status!=p_clock_info->workStatus)    /*工作状态变化*/
    {
		printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        syslog(CLOG_DEBUG,"workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          gps_init(p_clock_info);
           /**快捕状态下，每10个收集一组，收集20组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=30;
          p_clock_info->lockCounter=0;
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          gps_init(p_clock_info);

          /**锁定状态下，没10个收集一组，收集50组  */
          p_clock_info->data_1Hz.ph_number=15;
          p_clock_info->data_1Hz.queue_number=50;	 //50  20h
          p_clock_info->lock_times=0;

         break;
        case HOLD:   
            p_clock_info->hold_times=0;
            p_clock_info->unlockCounter=0;
            p_clock_info->lockCounter=0;
            break;
        default :
            break;
        
        }
        status=p_clock_info->workStatus;  /*记录当前工作状态*/
    }

}



void gps_status_machine(struct clock_info *p_clock_info,char ref_status)
{
    static char status = 0, tofast_cnt = 0;
    static Uint8 core_handletoauto_flag = 0, status_grand = 0;
    
    if(p_clock_info->ref_change_flag == TRUE)  /*同类参考源变化 丢弃当前数据*/
    {
        p_clock_info->syn_enable = 1;
        p_clock_info->bInitialClear=1; /*标记*/
        p_clock_info->data_1Hz.add_ph=0;
        p_clock_info->data_1Hz.ph_number_counter=0;
        p_clock_info->data_1Hz.queue_number_counter=0;
        p_clock_info->ref_change_flag = FALSE;
    }

    //printf("workStatus= %d \r\n",p_clock_info->workStatus);
    switch(p_clock_info->workStatus)
    {
        case FREE:
        {
            if(ref_status==REF_VALID)   /*参考源正常*/
            {
                tofast_cnt++;
               /** 进快捕保护 201026 */
                if((100 == tofast_cnt) && (g_RootData->satellite_data.satellite_see > 4))
                {
                    p_clock_info->workStatus=FAST;  /*参考源正常下等100s */
                    tofast_cnt = 0;
                }
                else if(g_RootData->satellite_data.satellite_see <= 4) //重复了 进不来
                    tofast_cnt = 0;
            }
            else /*参考源告警*/
            {
                tofast_cnt = 0;
            }
            p_clock_info->free_times++;
        }
            break;
        case FAST:
        {
            if(ref_status==REF_INVALID) /*参考源告警*/
            {
                p_clock_info->workStatus=FREE;
                p_clock_info->lockCounter = 1;
            }
            else if((ref_status==REF_VALID) && (p_clock_info->lockCounter>=LOCK_NUM))
            {
                p_clock_info->workStatus=LOCK;
                p_clock_info->unlockCounter=1;
            }
            p_clock_info->fast_times++;/*快捕用时*/

            //tofast_cnt = 0;
        }
            break;
        case LOCK:
        {
            if(ref_status==REF_INVALID) /*参考源告警*/
            {
                p_clock_info->workStatus= HOLD;
                p_clock_info->lockCounter = 2;   /** 若锁定中断(拔线)，不用跟太久 */
            }
            else if ((ref_status==REF_VALID) && (p_clock_info->unlockCounter>=UNLOCK_NUM))
            {
                p_clock_info->workStatus= FAST;
                //p_clock_info->lockCounter=1;
            }
            p_clock_info->lock_times++;/*锁定用时*/
        }
            break;
        case HOLD:
        {
            p_clock_info->hold_times++;/*保持用时*/
            if(0 != g_RootData->satellite_data.core_calibrate_flag)   /** 手动校时下，一直保持 20200918 */
            {
                //p_clock_info->workStatus=HOLD;
                core_handletoauto_flag = 1;
                break;
            }
               /** 下述自动下才进来: */
            if((p_clock_info->hold_times>=HOLD_DAYTIME) && (0 == core_handletoauto_flag))    /** 自动下的真保持:长于7天 */
            {
                
                p_clock_info->workStatus=FREE; /*保持时间大于1天，进入自由运行*/
                //core_handletoauto_flag = 0;
            }
            else if(ref_status==REF_VALID)
            {
                if((p_clock_info->hold_times<HOLD_DAYTIME) && (0 == core_handletoauto_flag))   /** 自动下的真保持:短于7天, 参考源恢复正常，进入快捕 */
                {
                    p_clock_info->workStatus=FAST;
                }
                else if(1 == core_handletoauto_flag)  /** 手动切入自动 */
                {
                    if(status == FREE)   /** 返回自由运行 */
                    {
                        p_clock_info->workStatus=status;
                        p_clock_info->free_times=0;
                    }
                    else if(status == FAST)   /** 返回快捕 */
                    {
                        p_clock_info->workStatus=status;
                        p_clock_info->fast_times=0;
                    }
                    else if((status == LOCK) || (status == HOLD))   /** 手动前为锁定/保持，退回快捕 */
                    {
                        p_clock_info->workStatus=FAST;
                    }
                    core_handletoauto_flag = 0;
                }
            }
            else if((ref_status==REF_INVALID) && (1 == core_handletoauto_flag))/** 手动切入自动，参考源告警 */
            {
                if(status == FREE)   /** 返回自由运行 */
                {
                    p_clock_info->workStatus=status;
                    p_clock_info->free_times=0;
                }
                else if(status == FAST)   /** 退回自由运行 */
                {
                    p_clock_info->workStatus=FREE;
                }
                else if((status == LOCK) || (status == HOLD))
                {
                    if(p_clock_info->hold_times>=HOLD_DAYTIME)
                    {
                        p_clock_info->workStatus=FREE;   /** 退回自由运行 */
                    }
                    else
                    {
                        p_clock_info->workStatus=HOLD;   /** 退回或维持保持 */
                        status = HOLD;   /** 保持的计数不变 */
                    }
                }
                core_handletoauto_flag = 0;
            }
        }
            break;
        default:
            break;
     }

    if((status != p_clock_info->workStatus) && (0 == g_RootData->satellite_data.core_calibrate_flag))    /* 自动下，工作状态变化 */
    {
        printf("workStatus change %d-->%d\r\n",status,p_clock_info->workStatus);
        printf("lockCounter %d unlockCounter %d\r\n",p_clock_info->lockCounter,p_clock_info->unlockCounter);

        switch(p_clock_info->workStatus)
        {
            case FREE:
                p_clock_info->free_times=0;
                break;
            case FAST:
//                gps_init(p_clock_info);
                /**快捕状态下，每10个收集一组，收集20组  */
                //p_clock_info->lockCounter=1;
                tofast_cnt = 0;
                p_clock_info->data_1Hz.ph_number=9;
                p_clock_info->data_1Hz.queue_number=15; //20
                p_clock_info->fast_times=0;
                p_clock_info->data_1Hz.getdata_flag=0;

                if(status == FREE)
                {
                    p_clock_info->lockCounter=1;
                    //p_clock_info->minus_flag=1; //显示假自由运行
                    p_clock_info->data_1Hz.freetofast_flag++;   /** 仅第一次的free->fast做对齐操作 */
                    p_clock_info->data_1Hz.clear_flag = 0;   /** 重新采ph 从lock过来不动 */
                    p_clock_info->bInitialClear = 1;   /** 重新算均值 */
                }
                else if(status == LOCK)
                {
                    //p_clock_info->minus_flag=0; //避免显示假自由运行
                    p_clock_info->lockCounter=2; //为1要告警 210223
                    p_clock_info->bInitialClear = 1;   /** 重新算均值 210223 */
                }
                else if(status == HOLD)
                {
                    p_clock_info->lockCounter = 2;   /** 确保不告警 */
                    p_clock_info->data_1Hz.tofast_60s_flag = 1;
                    p_clock_info->data_1Hz.clear_flag = 0;   /** 重新采ph */
                    p_clock_info->bInitialClear = 1;   /** 重新算均值 */
                }
                break;
            case LOCK:
//                gps_init(p_clock_info);
                /**锁定状态下，每10个收集一组，收集50组  */
                p_clock_info->unlockCounter=1;
                p_clock_info->data_1Hz.ph_number=12;
                p_clock_info->data_1Hz.queue_number=15;         //50  20h定长
                p_clock_info->lock_times=0;
                if(status == FAST)
                {
                    p_clock_info->data_1Hz.tofast_60s_flag = 1;
                }
                break;
            case HOLD:
                p_clock_info->unlockCounter=1;
                p_clock_info->lockCounter=2;
                p_clock_info->hold_times=0;
                break;
            default :
                break;
        }
        
        status = p_clock_info->workStatus;  /*记录当前工作状态*/
    }
}


void m10_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    if(p_clock_info->ref_change_flag == TRUE)  /*同类参考源变化丢弃当前数据*/
    {
      p_clock_info->bInitialClear=1; /*标记*/
      //p_clock_info->syn_enable = 1;
      p_clock_info->data_1Hz.add_ph=0;
      p_clock_info->data_1Hz.ph_number_counter=0;
      p_clock_info->data_1Hz.queue_number_counter=0;
      p_clock_info->ref_change_flag = FALSE;
    }


	//printf("workStatus= %d \r\n",p_clock_info->workStatus);
    switch(p_clock_info->workStatus)
    {
		case FREE:
        {
             if(ref_status==1)   /*参考源正常*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }

        }
      	    break;
     	case FAST:
        {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus=HOLD; 
            else if((ref_status==1)&&(p_clock_info->lockCounter>=4))
			{
               p_clock_info->workStatus=LOCK;
			   p_clock_info->unlockCounter=0;
			 }
            p_clock_info->fast_times++;/*快捕用时*/
        }		
       		break;
     	case LOCK:	
	     {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus= HOLD;
            else if ((ref_status==1)&&(p_clock_info->unlockCounter>=3))
			{
               p_clock_info->workStatus= FAST;
			   p_clock_info->lockCounter=0;
			}
            p_clock_info->lock_times++;/*锁定用时*/
            if((ref_status == 1) && ((p_clock_info->lock_times%86400) == 0))
            {
                

            }
         }
      		break;
     	case HOLD:	
        {
            p_clock_info->hold_times++;/*锁定用时*/
           if(p_clock_info->hold_times>=86400) /*保持时间大于1天, 无参考源 进入自由运行*/
           {
                p_clock_info->workStatus= FREE;
           }else if((p_clock_info->hold_times<86400)&&(ref_status==1)&&(p_clock_info->lockCounter>=4))
           {
                p_clock_info->workStatus=LOCK;
           }
           else if((p_clock_info->hold_times<86400)&&(ref_status==1)&&(p_clock_info->lockCounter<4))
           {
               p_clock_info->workStatus=FAST;

           }
           /*保持时间小于1于，参考源有效，进入锁定*/
        }	
      		break;
            
    	default:    
			
     		break;
     }

    if(status!=p_clock_info->workStatus)    /*工作状态变化*/
    {
		printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        syslog(CLOG_DEBUG,"workStatus change  workStatus= %d\n",p_clock_info->workStatus);

        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          gps_init(p_clock_info);
           /**快捕状态下，每10个收集一组，收集20组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=30;
          p_clock_info->lockCounter=0;
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          gps_init(p_clock_info);

          /**锁定状态下，没10个收集一组，收集50组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=50;	 //50  20h
          p_clock_info->lock_times=0;

         SetRbClockAlign_Once();
         p_clock_info->bInitialClear=1; 

         break;
        case HOLD:   
            p_clock_info->hold_times=0;
            p_clock_info->unlockCounter=0;
            p_clock_info->lockCounter=0;
            break;
        default :
            break;
        
        }
        status=p_clock_info->workStatus;  /*记录当前工作状态*/
    }

}
  /**
  * @brief  收到一组数据后调用不同的处理函数 ，配置为参考源为1PPS时调用。
  * @param  None
  * @retval None
  */

void gps_status_handle(struct clock_info *p_clock_info)
{   
  	switch(p_clock_info->workStatus)
    { 
		case FREE:
        	printf("free_handle\r\n");	
      	    break;
     	case FAST:
            printf("fast_handle\r\n");
            rb_fast_handle(p_clock_info);		
       		break;
     	case LOCK:	
            printf("lock_handle\r\n");
            rb_lock_handle(p_clock_info);	
      		break;
     	case HOLD:
            printf("hold_handle\r\n");		
      		break;           
    	default:    
			
     		break;
     }

}


void m10_status_handle(struct clock_info *p_clock_info)
{   
  	switch(p_clock_info->workStatus)
    { 
		case FREE:
        	printf("free_handle\r\n");	
      	    break;
     	case FAST:
            printf("fast_handle\r\n");
            m10_fast_handle(p_clock_info);		
       		break;
     	case LOCK:	
            printf("lock_handle\r\n");
            m10_lock_handle(p_clock_info);	
      		break;
     	case HOLD:
            printf("hold_handle\r\n");		
      		break;           
    	default:    
			
     		break;
     }

}

void ptp_status_handle(struct clock_info *p_clock_info)
{   
  	switch(p_clock_info->workStatus)
    { 
		case FREE:
        	printf("free_handle\r\n");	
      	    break;
     	case FAST:
            printf("fast_handle\r\n");
            ptp_fast_handle(p_clock_info);		
       		break;
     	case LOCK:	
            printf("lock_handle\r\n");
            ptp_lock_handle(p_clock_info);	
      		break;
     	case HOLD:
            printf("hold_handle\r\n");		
      		break;           
    	default:    
			
     		break;
     }

}


void ClockStateProcess_RB(struct clock_info *pClockInfo)
{
    struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
    Uint8 ref_status;
    static short tmp_ref = 0;

    if(pClockInfo->run_times == 30)
    {
        send_clock_data(pClockInfo,RB_CENTER_VALUE);
		if(pAlarmData->alarmBd1pps==FALSE)
        	SetRbClockAlign_Once();
        pClockInfo->bInitialClear = 1;
        
    }
        
    printf("runtime=%d ref_type=%d workStatus= %d\r\n",pClockInfo->run_times,pClockInfo->ref_type,pClockInfo->workStatus);
    /**参考源1pps，1pps和10m无收入告警，等待4分钟，等待小铷钟稳定  */
    /*选择参考源 1PPS输入不告警*/

    if((pClockInfo->ref_type == REF_SATLITE)&&(pAlarmData->alarmBd1pps==FALSE)&&(pClockInfo->run_times==RUN_TIME))
    {
        SetRbClockAlign_Once();
        pClockInfo->bInitialClear = 1;
        pClockInfo->lockCounter=0;
        
        pClockInfo->data_1Hz.add_ph=0;
        pClockInfo->data_1Hz.ph_number_counter=0;
        pClockInfo->data_1Hz.queue_number_counter=0;
        pClockInfo->data_1Hz.start_flag = 1;

    }
    if((pClockInfo->ref_type == REF_PTP)&&(pAlarmData->alarmPtp == FALSE)&&(pClockInfo->run_times == RUN_TIME))
    {
         pClockInfo->lockCounter=0;
         
         pClockInfo->data_1Hz.add_ph=0;
         pClockInfo->data_1Hz.ph_number_counter=0;
         pClockInfo->data_1Hz.queue_number_counter=0;
         pClockInfo->data_1Hz.start_flag = 1;
         
         printf("-------- REF_PTP start control rb\r\n");

    }
	if((pClockInfo->ref_type == REF_NTP)&&(pAlarmData->alarmNtp== FALSE)&&(pClockInfo->run_times == RUN_TIME))
    {
         pClockInfo->lockCounter=0;
         
         pClockInfo->data_1Hz.add_ph=0;
         pClockInfo->data_1Hz.ph_number_counter=0;
         pClockInfo->data_1Hz.queue_number_counter=0;
         pClockInfo->data_1Hz.start_flag = 1;
         
         printf("--------REF_NTP start control rb\r\n");

    }
    if(tmp_ref != pClockInfo->ref_type)
    {
        pClockInfo->ref_change_flag = TRUE;
        tmp_ref = pClockInfo->ref_type;
    }

    if(pClockInfo->ref_type == REF_PTP)
    {
        if(pAlarmData->alarmPtp == TRUE || (pAlarmData->alarmDisk == TRUE))
            ref_status = 0;   
        else 
            ref_status = 1;
        
        ptp_status_machine(pClockInfo,ref_status);
    }
    else if(pClockInfo->ref_type == REF_SATLITE)
    {
        if((pAlarmData->alarmBd1pps == TRUE) || (pAlarmData->alarmDisk == TRUE))
            ref_status = 0;   
        else 
            ref_status = 1;
        
        gps_status_machine(pClockInfo,ref_status);

    }
	else if((pClockInfo->ref_type == REF_NTP)&&(pAlarmData->alarmNtp== FALSE))
	{

		 if((pAlarmData->alarmNtp == TRUE) || (pAlarmData->alarmDisk == TRUE))
			 ref_status = 0;   
		 else 
			 ref_status = 1;

		 ntp_status_machine(pClockInfo,ref_status);

	}
    pClockInfo->run_times++;
}

void ClockHandleProcess(struct clock_info *pClockInfo)
{
    struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
    
    if((pClockInfo->data_1Hz.getdata_flag==1)
        &&(pAlarmData->alarmBd1pps == FALSE)
        &&(pAlarmData->alarmDisk == FALSE)
        &&(pClockInfo->ref_type==REF_SATLITE)&&(pClockInfo->run_times > RUN_TIME)) 
    {
         pClockInfo->data_1Hz.getdata_flag=0;
    
        /**收集够一组数据 */
        gps_status_handle(pClockInfo);
               
        system("hwclock -w");
    }
    else if((pClockInfo->data_1Hz.getdata_flag==1)
        &&(pAlarmData->alarmPtp == FALSE)
        &&(pAlarmData->alarmDisk == FALSE)
        &&(pClockInfo->ref_type== REF_PTP)&&(pClockInfo->run_times > RUN_TIME))
    {
        pClockInfo->data_1Hz.getdata_flag=0;
        
        ptp_status_handle(pClockInfo);
            
        system("hwclock -w");
    }
}

void ClockStateProcess_OCXO(struct clock_info *pClockInfo)
{
     struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
        Uint8 ref_status;
        static short tmp_ref = 0;
    
        if(pClockInfo->run_times == 20 && (pAlarmData->alarmBd1pps==FALSE))
        {
            SetRbClockAlign_Once();
            pClockInfo->bInitialClear = 1;
            printf("---------ref 1hz 1Hz_set center\r\n");
        }
    
        printf("runtime=%d ref_type=%d workStatus= %d\r\n",pClockInfo->run_times,pClockInfo->ref_type,pClockInfo->workStatus);
        /**参考源1pps，1pps和10m无收入告警，等待4分钟，等待小铷钟稳定  */
        /*选择参考源 1PPS输入不告警*/
    
        if((pClockInfo->ref_type == REF_SATLITE)&&(pAlarmData->alarmBd1pps==FALSE)&&(pClockInfo->run_times == RUN_TIME))
        {
            SetRbClockAlign_Once();
            pClockInfo->bInitialClear = 1;
            pClockInfo->lockCounter=0;
            
            pClockInfo->data_1Hz.add_ph=0;
            pClockInfo->data_1Hz.ph_number_counter=0;
            pClockInfo->data_1Hz.queue_number_counter=0;
            pClockInfo->data_1Hz.start_flag = 1;
            
        }

        if(pClockInfo->ref_type == REF_SATLITE)
        {
            if((pAlarmData->alarmBd1pps == TRUE) || (pAlarmData->alarmDisk == TRUE))
                ref_status = 0;   
            else 
                ref_status = 1;
            
            m10_status_machine(pClockInfo,ref_status);
			
        }
        
		if((pClockInfo->ref_type == REF_NTP)&&(pAlarmData->alarmNtp== FALSE)&&(pClockInfo->run_times > RUN_TIME))
		{

			 if((pAlarmData->alarmNtp == TRUE) || (pAlarmData->alarmDisk == TRUE))
				 ref_status = 0;   
			 else 
				 ref_status = 1;

			 ntp_status_machine(pClockInfo,ref_status);
		
		}
		if((pClockInfo->ref_type == REF_PTP)&&(pAlarmData->alarmPtp == FALSE)&&(pClockInfo->run_times > RUN_TIME))
		{

			 if((pAlarmData->alarmNtp == TRUE) || (pAlarmData->alarmDisk == TRUE))
				 ref_status = 0;   
			 else 
				 ref_status = 1;

			 ntp_status_machine(pClockInfo,ref_status);
		
		}
        pClockInfo->run_times++;
        
}

void ClockHandleProcess_OCXO(struct clock_info *p_clock_info)
{
    struct clock_alarm_data *pAlarmData = &p_clock_info->alarmData;
    

    if((p_clock_info->data_1Hz.getdata_flag==1)
        &&(pAlarmData->alarmBd1pps == FALSE)
        &&(pAlarmData->alarmDisk == FALSE)
        &&(p_clock_info->ref_type==REF_SATLITE)&&(p_clock_info->run_times>RUN_TIME)) 
    {

        
         p_clock_info->data_1Hz.getdata_flag=0;
          
               /**收集够一组数据 */
         m10_status_handle(p_clock_info);
         system("hwclock -w");
    }
	else if((p_clock_info->data_1Hz.getdata_flag==1)
        &&(pAlarmData->alarmPtp == FALSE)
        &&(pAlarmData->alarmDisk == FALSE)
        &&(p_clock_info->ref_type== REF_PTP)&&(p_clock_info->run_times > RUN_TIME))
    {
        p_clock_info->data_1Hz.getdata_flag=0;
        
        ptp_status_handle(p_clock_info);
        system("hwclock -w");
    }

    

}

