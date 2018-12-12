/********************************************************************************************
*                           ��Ȩ����(C) 2015,
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       satellite_api.c
*    ��������:       �������ݴ���
*    ����:           �¼���
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-4-30
*    �����б�:
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-4-30
                     ����: �¼���
                     �޸�����: �´����ļ�


*********************************************************************************************/

/*----------------------------------------------*
*    ����ͷ�ļ�                                   *
*----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/timeb.h>

#include "satellite_api.h"

Uint8 g_utc_offset = 18;

//BD_100�������ݸ�ʽ
// $NVGGA,013423.00,3039.4582,N,10405.0352,E,1,9,1.55,+474.12,M,0.1*25
// $NVRMC,013423.00,A,3039.4582,N,10405.0352,E,0.035,27.500,070515,0.000,,500*3A
// $NVZDA,013423.00,07,05,2015,07,26*69


/********************************************************************************************
* ��������:    gga
* ��������:    gga���ݽ�������
* �������:    struct Satellite_Data *p_satellite_data  
               char *str                                
* �������:    
* ����ֵ:      char
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-4-30
               ����:     �¼���
               �޸�����: �����ɺ���

********************************************************************************************/
char Gga(struct Satellite_Data *p_satellite_data,char *str)
{
        int status;
        regex_t reg;
        char tmp_str[32];
        Uint32 val;
        int cflags = REG_EXTENDED;
        const size_t nmatch = 15;
        regmatch_t matchptr[15];
        //printf("gga str :%s\n",str);
        const char *pattern =
         "^\\$(\\w+),(\\w+.\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),(\\w+),(\\w+),(\\w+),(\\w+.\\w+),(\\w+.\\w+),(\\w+)";
          // $NVGGA, 013423.00,  3039.4582,   N,  10405.0352,   E,    1     ,9   ,1.55      ,+474.12 ,M,0.1*25
        regcomp(&reg, pattern, cflags);
        if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
        {
            memset(p_satellite_data->position,0,sizeof(p_satellite_data->position));
            memcpy(p_satellite_data->position, str + matchptr[3].rm_so,
                    matchptr[3].rm_eo  - matchptr[3].rm_so);
            
            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[3].rm_so,matchptr[3].rm_eo - matchptr[3].rm_so);
            
            val = atof(tmp_str) * 10000;
            p_satellite_data->latitude_d = val/1000000;
            val = val%1000000;
            p_satellite_data->latitude_f = val/10000;
            val = val%10000;
            val = val*60;
            val = val/10000;
            p_satellite_data->latitude_m = val;
            
            p_satellite_data->northorsouth = *(str + matchptr[4].rm_so);

            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[5].rm_so,matchptr[5].rm_eo - matchptr[5].rm_so);

            val = atof(tmp_str) * 10000;
            p_satellite_data->longitude_d = val/1000000;
            val = val%1000000;
            p_satellite_data->longitude_f = val/10000;
            val = val%10000;
            val = val*60;
            val = val/10000;
            p_satellite_data->longitude_m = val;

            p_satellite_data->eastorwest = *(str + matchptr[6].rm_so);

            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[8].rm_so,
                    matchptr[8].rm_eo - matchptr[8].rm_so);
            p_satellite_data->satellite_use = atoi(tmp_str);  
            //p_satellite_data->satellite_see = atoi(tmp_str);  
            p_satellite_data->satellite_position = atoi(tmp_str);


            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[10].rm_so,matchptr[10].rm_eo - matchptr[10].rm_so);
            
            p_satellite_data->height = atof(tmp_str)*100;

            //printf("==>%d %d %d %d\n",p_satellite_data->longitude_d,p_satellite_data->longitude_f,p_satellite_data->longitude_m,p_satellite_data->height);
            regfree(&reg);
            return 1;
        }
        else
        {
            regfree(&reg);
            return 0;
        }

}



/********************************************************************************************
* ��������:    zda
* ��������:    ZDA���ݽ���
* �������:    struct Satellite_Data *p_satellite_data  
               char *str                                
* �������:    char
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-4-30
               ����:     �¼���
               �޸�����: �����ɺ���

********************************************************************************************/
char Zda(struct Satellite_Data *p_satellite_data,char *str)
{
	int status;
	regex_t reg;
	char tmp_str[32];
	int cflags = REG_EXTENDED;
	const size_t nmatch = 15;
	regmatch_t matchptr[15];
    //// $NVZDA,013423.00,07,05,2015,07,26*69
	const char *pattern ="^\\$(\\w+),(\\w+.\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
         
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[2].rm_so, 2);
	    p_satellite_data->tm.tm_hour = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[2].rm_so + 2, 2);
	    p_satellite_data->tm.tm_min = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[2].rm_so + 4, 2);
	    p_satellite_data->tm.tm_sec = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[3].rm_so,
		matchptr[3].rm_eo - matchptr[3].rm_so);
	    p_satellite_data->tm.tm_mday = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[4].rm_so,
		matchptr[4].rm_eo - matchptr[4].rm_so);
	    p_satellite_data->tm.tm_mon = atoi(tmp_str)-1;
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[5].rm_so,
		matchptr[5].rm_eo - matchptr[5].rm_so);
	    p_satellite_data->tm.tm_year = atoi(tmp_str)-1900; 

        p_satellite_data->time = mktime(&p_satellite_data->tm);
        //printf("Gps all time =%d \n",p_satellite_data->time);
        
		regfree(&reg);
		return 1;
	}
	 else 
	{
		regfree(&reg);
		return 0;
	}

}
/********************************************************************************************
* ��������:    rmc
* ��������:    GSV���ݽ�������
* �������:    struct Satellite_Data *p_satellite_data  
               char *str                                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-4-30
               ����:     �¼���
               �޸�����: �����ɺ���

********************************************************************************************/


char Rmc(struct Satellite_Data *p_satellite_data,char *str)
{
	int status;
	regex_t reg;
	char tmp_str[32];
	int cflags = REG_EXTENDED;
	const size_t nmatch = 15;
	regmatch_t matchptr[15];
    //printf("rmc str :%s\n",str);    //$NVRMC,013423.00,A,3039.4582,N,10405.0352,E                         ,0.035,     27.500,070515,,,500*3A
    const char *pattern ="^\\$(\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+)";
    regcomp(&reg, pattern, cflags);       
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        //printf("rmc str :%s\n",str);

       
        memset(tmp_str,'\0',sizeof(tmp_str));
        memcpy(p_satellite_data->satellite_mode, str + matchptr[3].rm_so,
    	matchptr[3].rm_eo - matchptr[3].rm_so);

        if(p_satellite_data->satellite_mode[0] ==  'V')
            p_satellite_data->time_enable = FALSE;
        else if(p_satellite_data->satellite_mode[0] ==  'A')
            p_satellite_data->time_enable = TRUE;
        
        //printf("rmc:%s\n",p_satellite_data->satellite_mode);
        regfree(&reg);
    	return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}

/********************************************************************************************
* ��������:    gsv
* ��������:    GSV���ݽ�������
* �������:    struct Satellite_Data *p_satellite_data  
               char *str                                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-4-30
               ����:     �¼���
               �޸�����: �����ɺ���

********************************************************************************************/
char Gsv(struct Satellite_Data *p_satellite_data,char *str)
{
	int status;
	regex_t reg;
	char tmp_str[32];
	int cflags = REG_EXTENDED;
	const size_t nmatch = 15;
	regmatch_t matchptr[15];
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+)";

    //const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        memcpy(tmp_str, str + matchptr[4].rm_so,
		matchptr[4].rm_eo - matchptr[4].rm_so);
		p_satellite_data->satellite_see = atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        //p_satellite_data->satellite_see = 0;
        regfree(&reg);
        return 0;

     }

}


char BdGpt(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];
    short leaps;
    
    /** $BDGPT,<1>,<2>, <3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        /**������  */
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[7].rm_so,
		matchptr[7].rm_eo - matchptr[7].rm_so);

        leaps = atoi(tmp_str);
		p_satellite_data->gps_utc_leaps = atoi(tmp_str);
        p_satellite_data->gps_utc_leaps = (leaps/10) *16 + (leaps%10);
        
        /**�������  */
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[8].rm_so,
		matchptr[8].rm_eo - matchptr[8].rm_so);

        if(tmp_str[1]&0x40)
            p_satellite_data->leap_enable = TRUE;
        else
            p_satellite_data->leap_enable = FALSE;

#if 0
        if((tmp_str[1]&0x07) == 0x04)
            p_satellite_data->time_enable = 0;
        else if((tmp_str[1]&0x07) == 0x05)
            p_satellite_data->time_enable = 1;
#endif

        /**����״̬  */
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[9].rm_so,
		matchptr[9].rm_eo - matchptr[9].rm_so);
		p_satellite_data->antenna = atoi(tmp_str);

        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[11].rm_so,
		matchptr[11].rm_eo - matchptr[11].rm_so);
        p_satellite_data->satellite_use = atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

     }

}


char BdLps(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];

    /** $BDLPS,<1>,<2>, <3>,<4>,<5>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);

    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[2].rm_so,
        matchptr[2].rm_eo - matchptr[2].rm_so);
        p_satellite_data->leap58or60= atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}

char BdVer(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];

    /** $BDGPT,<1>,<2>, <3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}

char Pubx(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];

    /** $BDGPT,<1>,<2>, <3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[7].rm_so,
        matchptr[7].rm_eo - matchptr[7].rm_so);
        if(tmp_str[2] == 'D')
            p_satellite_data->gps_utc_leaps = 18;
        else
            p_satellite_data->gps_utc_leaps = atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}


char BdGsa(struct Satellite_Data *p_satellite_data,char *str)
{
	return -1;
}

/********************************************************************************************
* ��������:    satellite_handle
* ��������:    vcom�������ݴ���
* �������:    struct Satellite_Data *p_satellite_data  
               char *str                                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-4-30
               ����:     �¼���
               �޸�����: �����ɺ���

********************************************************************************************/
char SatelliteHandle(struct Satellite_Data *p_satellite_data,char *str)
{
    char *p;
    char temp[256];
    char ret;
    
    memcpy(temp,str,strlen(str));

    if((NULL != strstr(temp,"$GBZDA"))
        ||(NULL != strstr(temp,"$GNZDA"))
        || (NULL != strstr(temp,"$GPZDA")))
    {
        /**���ʱ��  */
        ret = Zda(p_satellite_data,str); 
        if(!ret)
            p_satellite_data->time++;

    }
    else if((NULL != strstr(temp,"$GBGGA"))
        || (NULL != strstr(temp,"$GNGGA"))
        || (NULL != strstr(temp,"$GPGGA")))
    {
        /**���  λ��*/
        Gga(p_satellite_data,str);    
    }
    else if((NULL != strstr(temp,"$GBRMC"))
        || (NULL != strstr(temp,"$GNRMC"))
        || (NULL != strstr(temp,"$GPRMC")))
    {
        /**��������ģʽ  */
        Rmc(p_satellite_data,str);   
    }
    else if(NULL != strstr(temp,"$GPGSV"))
    {
        Gsv(p_satellite_data,str);
    }
    else if(NULL != strstr(temp,"$GPGPT"))
    {
        /**������������ʹ�ܣ�����״̬  */
        BdGpt(p_satellite_data,str);
       
    }
    else if(NULL != strstr(temp,"$BDLPS"))
    {
        BdLps(p_satellite_data,str);
       
    }
    else if(NULL != strstr(temp,"$BDVER"))
    {
        BdVer(p_satellite_data,str);
    }
    else if(NULL != strstr(temp,"$GPTXT"))
    {
        
    }
    else if(NULL != strstr(temp,"$GPGSA"))
    {
        BdGsa(p_satellite_data,str);
    }
    else if(NULL != strstr(temp,"$PUBX"))
    {
        Pubx(p_satellite_data,str);
    }
    else
    {
        //printf("-----------------\n");
       //p_satellite_data->gps_utc_leaps = g_utc_offset;

    }
	return 0;
}

void Display_SatelliteData(struct Satellite_Data *p_satellite_data)
{
    printf("satellite_position=%d satellite_see=%d gps_utc_leaps=%d time_enable=%d antenna=%d\n",
        p_satellite_data->satellite_position,p_satellite_data->satellite_see
        ,p_satellite_data->gps_utc_leaps,p_satellite_data->time_enable,p_satellite_data->antenna);
}