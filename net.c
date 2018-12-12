/********************************************************************************************
*                           ��Ȩ����(C) 2015, 
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       net.c
*    ��������:       ����ip mac��ַ ���أ������ļ�������������
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-1-30
*    �����б�:
                     AddGateWay
                     AddIpAddr_ToMultiCast
                     BindSockDevice
                     DelGateWay
                     DisplayInterfaceStatus
                     GetAddress
                     GetGateWay
                     GetIpAddress
                     GetMacAddress
                     GetMaskAddress
                     SetGateWay
                     SetIpAddress
                     SetMacAddress
                     SetMaskAddress
                     SetMutiCastLoop
                     SetMutiCastTTl
                     SetSock_NoCheck
                     SetSock_reUse
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-1-30
                     ����: HuangFei
                     �޸�����: �´����ļ�


*********************************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <net/if.h>
#include <net/route.h>

#include <string.h>
#include <ifaddrs.h>
#include <ctype.h>

#include "net.h"

#define TRUE    1
#define FALSE   0
#define MAC_ADDRESS_LEN 6
#define PATH_PROCNET_ROUTE		"/proc/net/route"
#define IFACE_NAME_LENGTH        IF_NAMESIZE


/**********************************************************************
 * Function:      SetMacAddress
 * Description:   ����mac��ַ
 * Input:         pInterface ������ӿ����� �磺eth0��pMacAddress��mac��ַ�׵�ַ
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetMacAddress(char *pInterface,char *pMacAddress)
{
    struct ifreq ifr;
    struct sockaddr *hwaddr;
    unsigned char *hwptr;
    int sockfd;
    int i;

	/* �жϽӿ������Ƿ���ȷ */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(pMacAddress == NULL)
    {
        printf("input mac address error\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
    
     /*����socket����Ϣ*/
	if (0 > ioctl(sockfd, SIOCGIFFLAGS, &ifr))
	{
		printf("close the ethx:unknown interface\r\n");
	}
	ifr.ifr_flags &= ~IFF_UP;
    /*����Ϣд��socket�У��Ӷ��ر�eth0*/
	if (0 > ioctl(sockfd, SIOCSIFFLAGS, &ifr)) 
	{
		printf("Can't close the ethx \r\n");
        return FALSE;
	}

	/*---------------------------------------------------------------------*/
	/* set the interface eth0's  MAC address                               */
	/*---------------------------------------------------------------------*/
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	hwaddr=(struct sockaddr *)&ifr.ifr_hwaddr;
    
    /*ARPHRD_ETHER��1����ʶ���յ�ARPΪ��̫����ʽ*/
	hwaddr->sa_family = ARPHRD_ETHER;
	hwptr = hwaddr->sa_data;

	for(i=0;i<6;i++)
		*hwptr++ = (unsigned char)pMacAddress[i];
		
	if (0 > ioctl(sockfd, SIOCSIFHWADDR, &ifr))
	{
		printf("Can't set the MAC address\r\n");
	}

    /*---------------------------------------------------------------------*/
    /* open the interface eth0                                             */
    /*---------------------------------------------------------------------*/	
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
    if (0 > ioctl(sockfd,SIOCGIFFLAGS, &ifr)) 
    {
    	printf("open the ethx:unknown interface\r\n");
    }
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (0 > ioctl(sockfd, SIOCSIFFLAGS, &ifr)) 
    {
    	printf("Can't open the ethx\r\n");
    }
    
    close(sockfd);

    return TRUE;
}


/**********************************************************************
 * Function:      SetIpAddress
 * Description:   ����ip��ַ
 * Input:         pInterface ������ӿ����� �磺eth0��IpAddress��ip��ַ��
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetIpAddress(char *pInterface,unsigned int IpAddress)
{

    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(IpAddress == 0)
    {
        printf("input ip address error\n");
        return FALSE;
    }
 
    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	sinaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	sinaddr->sin_family = AF_INET;
	sinaddr->sin_addr.s_addr = IpAddress;
	if (0 > ioctl(sockfd, SIOCSIFADDR, &ifr)) 
	{
		printf("Can't set ip address\r\n");
	}

    close(sockfd);

    return TRUE;
}

/**********************************************************************
 * Function:      SetMaskAddress
 * Description:   ������������
 * Input:         pInterface ������ӿ����� �磺eth0��IpMask����������
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetMaskAddress(char *pInterface,unsigned int IpMask)
{
    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

	/* �жϽӿ������Ƿ���ȷ */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(IpMask == 0)
    {
        printf("input mask address error\n");
        return FALSE;
    }

    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	sinaddr = (struct sockaddr_in *)&ifr.ifr_netmask;
	sinaddr->sin_family = AF_INET;
	sinaddr->sin_addr.s_addr = IpMask;
	if (0 > ioctl(sockfd, SIOCSIFNETMASK, &ifr)) 
	{
		printf("Can't set subnet netmask\r\n");
	}

    close(sockfd);

    return TRUE;
}

/**********************************************************************
 * Function:      DelGateWay
 * Description:   ɾ��ָ��������
 * Input:         pInterface ������ӿ����� �磺eth0��DelGateWay������
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char DelGateWay(char *pInterface,unsigned int DelGateWay)
{

    struct sockaddr_in *sinaddr;
    struct rtentry rt;
    struct sockaddr rtsockaddr;
    int sockfd;

    /* �жϽӿ������Ƿ���ȷ */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(DelGateWay == 0)
    {
        printf("input GateWay error!\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}


	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	memset((char *)&rtsockaddr,0,sizeof(struct sockaddr));
	sinaddr = (struct sockaddr_in *)&rtsockaddr;
	sinaddr->sin_family = AF_INET;
    
    /*INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ,�����ַ��ʵ�ϱ�ʾ��ȷ����ַ,�����е�ַ�����������ַ��*/
	sinaddr->sin_addr.s_addr = INADDR_ANY;	
    
    /*set the destination address to '0.0.0.0'*/
	rt.rt_dst = rtsockaddr;

    /*set the netmask to '0.0.0.0'*/
	rt.rt_genmask = rtsockaddr;			

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 1;
	rt.rt_dev = pInterface;

	/*delete the current default gateway*/
	sinaddr->sin_addr.s_addr = DelGateWay;
	rt.rt_gateway = rtsockaddr;

    
	if (0 > ioctl(sockfd, SIOCDELRT, &rt)) 
	{
		printf("Can't delete the current gateway\r\n");
	}

    close(sockfd);

    return TRUE;
}

unsigned char SetGateWay(char *pInterface,unsigned int ip,unsigned int gateway)
{
    unsigned char cmd[50];  
    memset(cmd,0,sizeof(cmd));
    if(gateway == 0)
    {
        printf("gateway ==0 invalid\n");
        return 0;
    }
    sprintf(cmd,"route add default gw %d.%d.%d.%d"
    ,gateway&0xff,(gateway>>8)&0xff,(gateway>>16)&0xff,(gateway>>24)&0xff);

    system(cmd);
    printf("\n");
    return 1;

}


/**********************************************************************
 * Function:      AddGateWay
 * Description:   ����ָ��������
 * Input:         pInterface ������ӿ����� �磺eth0��NewGateWay������
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char AddGateWay(char *pInterface,unsigned int NewGateWay)
{
    struct sockaddr_in *sinaddr;
    struct rtentry rt;
    struct sockaddr rtsockaddr;
    int sockfd;

    /* �жϽӿ������Ƿ���ȷ */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(NewGateWay == 0)
    {
        printf("input GateWay error!\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}


	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	memset((char *)&rtsockaddr,0,sizeof(struct sockaddr));
	sinaddr = (struct sockaddr_in *)&rtsockaddr;
	sinaddr->sin_family = AF_INET;
    
    /*INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ,�����ַ��ʵ�ϱ�ʾ��ȷ����ַ,�����е�ַ�����������ַ��*/
	sinaddr->sin_addr.s_addr = INADDR_ANY;	
    
    /*set the destination address to '0.0.0.0'*/
	rt.rt_dst = rtsockaddr;

    /*set the netmask to '0.0.0.0'*/
	rt.rt_genmask = rtsockaddr;			

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 1;
	rt.rt_dev = pInterface;

	/*set the new default gateway*/
	sinaddr->sin_addr.s_addr = NewGateWay;
	rt.rt_gateway =rtsockaddr;
	    
	if (0 > ioctl(sockfd, SIOCADDRT, &rt))
	{
		printf("Can't set the new default gateway\r\n");
	}

    close(sockfd);

    return TRUE;

}

#if 0
/**********************************************************************
 * Function:      SetGateWay
 * Description:   �������أ���ɾ���Ѿ��еģ�������µ�
 * Input:         pInterface ������ӿ����� �磺eth0��DelGateWay��ɾ�������أ�
				  NewGateWay�� ����ӵ�����
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetGateWay(char *pInterface,unsigned int DelGateWay,unsigned int NewGateWay)
{

    struct sockaddr_in *sinaddr;
    struct rtentry rt;
    struct sockaddr rtsockaddr;
    int sockfd;

    /* �жϽӿ������Ƿ���ȷ */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if((DelGateWay == 0) && (NewGateWay == 0))
    {
        printf("input GateWay error!\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}


	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	memset((char *)&rtsockaddr,0,sizeof(struct sockaddr));
	sinaddr = (struct sockaddr_in *)&rtsockaddr;
	sinaddr->sin_family = AF_INET;
    
    /*INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ,�����ַ��ʵ�ϱ�ʾ��ȷ����ַ,�����е�ַ�����������ַ��*/
	sinaddr->sin_addr.s_addr = INADDR_ANY;	
    
    /*set the destination address to '0.0.0.0'*/
	rt.rt_dst = rtsockaddr;

    /*set the netmask to '0.0.0.0'*/
	rt.rt_genmask = rtsockaddr;			

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 1;
	rt.rt_dev = pInterface;

    
	/*delete the current default gateway*/
	sinaddr->sin_addr.s_addr = DelGateWay;
	rt.rt_gateway = rtsockaddr;

    if(DelGateWay != 0)
    {
        if (0 > ioctl(sockfd, SIOCDELRT, &rt)) 
        {
            printf("Can't delete the current default gateway\r\n");
        }
    }


	/*set the new default gateway*/
	sinaddr->sin_addr.s_addr = NewGateWay;
	rt.rt_gateway =rtsockaddr;
	    
	if (0 > ioctl(sockfd, SIOCADDRT, &rt))
	{
		printf("Can't set the new default gateway\r\n");
	}

    close(sockfd);

    return TRUE;
}
#endif

/**********************************************************************
 * Function:      GetMacAddress
 * Description:   ���ָ�����ڵ�����MAC����
 * Input:         ifaceName ������ӿ����� �磺eth0��hwAddr���õ���mac��ַ
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char GetMacAddress (char* ifaceName, unsigned char* hwAddr)
{
    int ret;
    int sockfd;
    struct ifreq ifr;

    /* �жϽӿ������Ƿ���ȷ */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
        
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
    	printf("Could not open test socket");
    	return FALSE;
    }

    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name, ifaceName, IFACE_NAME_LENGTH);

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("failed to request hardware address for %s", ifaceName);
	    close(sockfd);
        return FALSE;
    }

    //int af = ifr.ifr_hwaddr.sa_family;
    //int af = ifr.ifr_addr.sa_family;
    
    memcpy(hwAddr, ifr.ifr_hwaddr.sa_data, MAC_ADDRESS_LEN);
   //memcpy(hwAddr, ifr.ifr_addr.sa_data, hwAddrSize);

	return TRUE;
    
}

/**********************************************************************
 * Function:      GetIpAddress
 * Description:   ���ָ���˿ڵ�ip��ַ
 * Input:         ifaceName ������ӿ����� �磺eth0��IpAddress���õ���ip��ַ
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char GetIpAddress(char* ifaceName,unsigned int *IpAddress)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    /* �жϽӿ������Ƿ���ȷ */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }

    if(getifaddrs(&ifaddr) == -1)
    {
	    printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return FALSE;
    }
    
    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
        return FALSE;
	}

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
    	if(!strcmp(ifaceName, ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET))
        {
    		memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name)-1);
            if (0 > ioctl(sockfd, SIOCGIFADDR, &ifr)) 
	        {
		        printf("Can't get ip address\r\n");
                return FALSE;
	        }
            sinaddr = (struct sockaddr_in *)&ifr.ifr_addr;
            *IpAddress = sinaddr->sin_addr.s_addr;
            freeifaddrs(ifaddr);
            close(sockfd);
            return TRUE;

    	}

    }

    printf("input interface no found\n");

    freeifaddrs(ifaddr);
    
    return FALSE;
}

/**********************************************************************
 * Function:      GetMaskAddress
 * Description:   ���ָ���˿ڵ���������
 * Input:         pInterface ������ӿ����� �磺eth0��MaskAddress���õ�����������
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char GetMaskAddress(char* ifaceName,unsigned int *MaskAddress)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    /* �жϽӿ������Ƿ���ȷ */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }

    if(getifaddrs(&ifaddr) == -1)
    {
	    printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return FALSE;
    }

    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
        return FALSE;
	}


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
    	if(!strcmp(ifaceName, ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET))
        {
    		memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name)-1);
            if (0 > ioctl(sockfd, SIOCGIFNETMASK, &ifr)) 
	        {
		        printf("Can't get ip address\r\n");
                return FALSE;
	        }
            sinaddr = (struct sockaddr_in *)&ifr.ifr_netmask;
            *MaskAddress = sinaddr->sin_addr.s_addr;
            freeifaddrs(ifaddr);
            close(sockfd);
            return TRUE;
    	}
    }

    printf("input interface no found\n");
    freeifaddrs(ifaddr);
    
    return FALSE;
}

/**********************************************************************
 * Function:      GetAddress
 * Description:   ���ָ�����ڵ�sockdaddr��ַ
 * Input:         ifaceName ������ӿ����� �磺eth0��addr����ַ
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char GetAddress(char* ifaceName,struct sockaddr* addr)
{
    struct ifaddrs *ifaddr, *ifa;

    /* �жϽӿ������Ƿ���ȷ */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }

    if(getifaddrs(&ifaddr) == -1)
    {
	    printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return FALSE;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {

    	if(!strcmp(ifaceName, ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET))
        {
    		memcpy(addr, ifa->ifa_addr, sizeof(struct sockaddr));
            freeifaddrs(ifaddr);
            return TRUE;

    	}

    }

    printf("input interface no found\n");
    freeifaddrs(ifaddr);
    return FALSE;
}


/**********************************************************************
 * Function:      GetGateWay
 * Description:   �õ�Ĭ������
 * Input:         pInterface ������ӿ����� �磺eth0��pGateWay���õ�������
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char GetGateWay(char *pInterface,unsigned int *pGateWay)
{
    short i = 0;
 	char buff[1024], iface[16];
	char gate_addr[128], net_addr[128];
	char mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;
    
 	char *sp , *bp;    
	unsigned int val=0;

    /* �жϽӿ������Ƿ���ȷ */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(pGateWay == NULL)
    {
        printf("input mac address error\n");
        return FALSE;
    }

    FILE *fp = fopen(PATH_PROCNET_ROUTE, "r");

    while (fgets(buff, 1023, fp))
    {
             num = sscanf(buff, "%16s %128s %128s %X %d %d %d %128s %d %d %d\n",
             iface, net_addr, gate_addr,
             &iflags, &refcnt, &use, &metric, mask_addr,
             &mss, &window, &irtt);
             if (num < 10 || !(iflags & RTF_UP))
                 continue;
             if(strncmp(iface,pInterface,4)!=0)
                 continue;
             if(strncmp(net_addr,"00000000",8)!=0)
                 continue;
             if(strncmp(mask_addr,"00000000",8)!=0)
                 continue;
             bp = (char *) &val;
             
             sp = gate_addr;
             for (i = 0; i < 4; i++) 
             {
                 *sp = toupper(*sp);
                 if ((*sp >= 'A') && (*sp <= 'F'))
                     bp[i] |= (int) (*sp - 'A') + 10;
                 else if ((*sp >= '0') && (*sp <= '9'))
                     bp[i] |= (int) (*sp - '0');
                 else
                 {
                     printf("Can't convert the gateway string to integer.\n\r");
                     fclose(fp);
                     return 0;
                 }
                 bp[i] <<= 4;
                 sp++;
                 *sp = toupper(*sp);
     
                 if ((*sp >= 'A') && (*sp <= 'F'))
                         bp[i] |= (int) (*sp - 'A') + 10;
                 else if ((*sp >= '0') && (*sp <= '9'))
                         bp[i] |= (int) (*sp - '0');
                 else
                 {
                     printf("Can't convert the gateway string to integer.\n");
                     fclose(fp);
                     return 0;
                 }
                 sp++;
             }
    }

    if(val == 0)
    {
        printf("Don't find default gateway = %d\n",val);
        fclose(fp);
        return FALSE;
    }
    else
    {
        *pGateWay = htonl(val);
    }


    fclose(fp);
    return TRUE;
}


/**********************************************************************
 * Function:      SetSock_reUse
 * Description:   ���������׽����ظ�ʹ��
 * Input:         sockfd�������׽���
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetSock_reUse(int sockfd)
{
    int reuse = 1;
    if(-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)))
    {
		printf("setsockopt fail!\n");
        return FALSE;
	}

    return TRUE;
}

/**********************************************************************
 * Function:      SetSock_NoCheck
 * Description:   �����׽������ݰ������
 * Input:         sockfd�������׽���
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetSock_NoCheck(int sockfd)
{
    int nocheck = 1;
    if(0 != setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, &nocheck, sizeof(int)))
    {
        printf("setsockopt NO_CHECK error!\n");
        return FALSE;
    }

    return TRUE;
}

/**********************************************************************
 * Function:      SetMutiCastLoop
 * Description:   ���ûػ��ಥ
 * Input:         sockfd��socket�׽��֣�val��0 �رգ�1 ��
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetMutiCastLoop(int sockfd,int val)
{
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(int))< 0)
    {
        printf("failed to set multicast LOOP value \n");
        return FALSE;
    }

    return TRUE;
}

/**********************************************************************
 * Function:      SetMutiCastTTl
 * Description:   ���öಥ��ַ�����ݰ�������ʱ��
 * Input:         sockfd��socket�׽��֣�ttl������ʱ��
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char SetMutiCastTTl(int sockfd,int ttl)
{
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl))< 0)
    {
        printf("failed to set multicast TTL value \n");
        return FALSE;
    }
    return TRUE;
}

unsigned char BindSockDevice(char*ifaceNmae,int sockfd)
{
    struct ifreq ifreg;
    memcpy(ifreg.ifr_name, ifaceNmae,IFNAMSIZ);
    if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&ifreg, sizeof(ifreg)) < 0)
    {
        printf("bindSockDevice error\n");
        return FALSE;
    }

    return TRUE;
}

/**********************************************************************
 * Function:      AddIpAddr_ToMultiCast
 * Description:   ��ָ��ip��ַ�����ಥ��
 * Input:         sockfd��socket�׽��֣�ip��ip��ַ��multi_ip���ಥ��ַ��ttl������ʱ��
 * Return:        �ɹ� 1��ʧ�� 0
 * Others:        
**********************************************************************/
unsigned char AddIpAddr_ToMultiCast(int sockfd,unsigned int ip,unsigned int multi_ip,int ttl)
{
    struct ip_mreq imr;
    
    imr.imr_multiaddr.s_addr = multi_ip;
    imr.imr_interface.s_addr = ip;

    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &imr.imr_interface.s_addr, sizeof(struct in_addr)) < 0)
    {
        printf("failed to set  multicast send interface\n");
        return FALSE;
    }
    
    if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr))< 0)
    {
        printf("failed to add  multicast member\n");
        return FALSE;
    }
    if(SetMutiCastTTl(sockfd,ttl) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}

/**********************************************************************
 * Function:      DisplayInterfaceStatus
 * Description:   ��ʾ��ǰ�ӿ�״̬��Ϣ
 * Input:         ifaceName ������ӿ����� �磺eth0
 * Return:        void
 * Others:        
**********************************************************************/
void DisplayInterfaceStatus(char* ifaceName)
{
    struct ifaddrs *ifaddr, *ifa;
    unsigned int flags;

    
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return;
    }
    
    if(getifaddrs(&ifaddr) == -1)
    {
    	printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {

    	if(!strcmp(ifaceName, ifa->ifa_name))
        {
    	    flags = ifa->ifa_flags;
    	    freeifaddrs(ifaddr);
    	}

    }
    if(flags & IFF_UP)
    {
        printf("Interface %s is up \n",ifaceName);
    }
    if(flags & IFF_RUNNING)
    {
        printf("Interface %s is running ",ifaceName);
    }

}





