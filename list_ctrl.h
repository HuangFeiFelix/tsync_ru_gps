/*
 * list_ctrl.h
 *
 *  Created on: 2014-2-23
 *      Author: administrator
 */

#ifndef LIST_CTRL_H_
#define LIST_CTRL_H_

#include <stdio.h>
#include <string.h>

#include "list.h"

#define MAX_COM_NUM			5

#define UART_BUF_SIZE 	    1024
#define UART_LEN 			1

#define PROTOCOL 0
#define MAX_LINK 10

#define DATA_BITS 8
#define PARITY 'N'
#define STOP_BITS 1
#define BUFFER_SIZE 			1500

#define QUEUE_SIZE  50   /**message queue, que size  20  */
#define QUEUE_BUFFER_LEN 1500


struct uart_buf {
	int cur;
	int flags;
	char buf[UART_BUF_SIZE];
};




struct net_lev{
	int fd;							//客户端文件描述符
	struct sockaddr_in dev_sockaddr;//套接字
};

struct recv_lev{
	char *data;						//数据存放地址
	int fd; 						//数据来源
	int len;						//数据长度
	time_t recv_time,deal_time;		//recv_time 数据创建时间 deal_time 数据处理时间
};

struct send_lev{
	char *data;						//数据存放地址
	int len;						//数据长度

	union {
		int fd;						//发送文件描述符
		struct list_head *fd_head;	//发送文件描述符列向量头指针
	};
	time_t add_time,send_time;		//recv_time 数据创建时间,deal_time 数据处理时间
};

struct data_list {
	union {
		struct send_lev send_lev;	//发送链表
		struct recv_lev recv_lev;	//接收链表
		struct net_lev net_lev;		//网络链表

	};
	struct list_head list_head;		//行向量链表用于链接数据
};

struct data_head {
	int type;						//类型 0：接收数据，1：发送数据，2：文件描述符
	int count;						//链表中数据数目
	pthread_rwlock_t *list_rwlock;	//链表操作读写锁
	struct list_head list_head;		//数据链表
};




struct net_attr{
	int domain;						//通信域，套接定域描述
	int sock_type;					//套接字类型
	int protocol;					//制定和种协议，这个参数通常为0
	short int sin_family;			//地址家族，一般是AF_INET,代表TCP/IP协议族
	int sin_port;					//端口号
	int ip;				            //ip地址
	int max_link;					//最大连接数
	char buf[BUFFER_SIZE];   		//网口接收缓冲
};

struct com_attr{
	int com_port;					//串口端口号
	int baud_rate;					//波特率
	int data_bits;					//数据位
	char parity;					//奇偶校验位
	int stop_bits;					//停止位
	struct uart_buf uart_buf;		//串口接收缓冲
};

struct int_attr{
	char path[64];					//设备路径
	char buf[BUFFER_SIZE];			//数据接收缓冲
};

struct dev_msg{
    Uint8  r_buf[QUEUE_BUFFER_LEN];
    Uint8  type;                    
    Uint8  dev_id;                  
    struct device *pdev;      
    Uint32 r_len;
};


struct dev_queue{
	struct dev_msg msg[QUEUE_SIZE];
	Uint16 head; 
	Uint16 tail;
    Uint16 count;
};

struct dev_head{
	int count;						//总设备数量

    //select函数使用参数
    fd_set fd_set;				//fd_set
    fd_set tmp_set;				//tmp_set
    int max_fd;					//文件描述符最大值
    sem_t sem[2];				//［0］：数据接收，［1］：数据发送

    struct dev_queue recv_queue;
    struct dev_queue send_queue;
    struct list_head list_head;         //设备链表

};


struct device {
	int dev_id;							//设备ID号
	int fd;								//设备打开后的文件描述符
	int type;							//设备类型 1:网口设备，2:串中设备,3:一般设备
    int valid;
    union {								//设备属性类型
		struct net_attr net_attr;		//网络参数
		struct com_attr com_attr;		//串口参数
		struct int_attr int_attr;		//中断设备
	};
    struct sockaddr_in  dest_addr;
    int sock_len;
    
	sem_t *p_sem[2];					//［0］：数据接收，［1］：数据发送
	struct data_head data_head[3];		//数据 ［0］：接收数据，［1］：发送数据，［2］：文件描述符
	int (*open_dev)(struct device *pdev,struct dev_head *dev_head);	//设备打开操作函数
	int (*close_dev)(struct device *pdev,struct dev_head *dev_head);//设备关闭函数
	int (*send_data)(struct device *pdev,struct dev_head *dev_head);//设备中数据发送函数
	int (*recv_data)(struct device *pdev,struct dev_head *dev_head);//设备中数据接收函数
	struct list_head list_head;			//设备链表
};




Uint8 add_recv(struct dev_head *dev_head,struct device *p_dev,char *data,int len);
Uint8 add_send(struct dev_head *dev_head,struct device *p_dev,char *data,int len);

#endif /* LIST_CTRL_H_ */
