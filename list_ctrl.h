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
	int fd;							//�ͻ����ļ�������
	struct sockaddr_in dev_sockaddr;//�׽���
};

struct recv_lev{
	char *data;						//���ݴ�ŵ�ַ
	int fd; 						//������Դ
	int len;						//���ݳ���
	time_t recv_time,deal_time;		//recv_time ���ݴ���ʱ�� deal_time ���ݴ���ʱ��
};

struct send_lev{
	char *data;						//���ݴ�ŵ�ַ
	int len;						//���ݳ���

	union {
		int fd;						//�����ļ�������
		struct list_head *fd_head;	//�����ļ�������������ͷָ��
	};
	time_t add_time,send_time;		//recv_time ���ݴ���ʱ��,deal_time ���ݴ���ʱ��
};

struct data_list {
	union {
		struct send_lev send_lev;	//��������
		struct recv_lev recv_lev;	//��������
		struct net_lev net_lev;		//��������

	};
	struct list_head list_head;		//����������������������
};

struct data_head {
	int type;						//���� 0���������ݣ�1���������ݣ�2���ļ�������
	int count;						//������������Ŀ
	pthread_rwlock_t *list_rwlock;	//���������д��
	struct list_head list_head;		//��������
};




struct net_attr{
	int domain;						//ͨ�����׽Ӷ�������
	int sock_type;					//�׽�������
	int protocol;					//�ƶ�����Э�飬�������ͨ��Ϊ0
	short int sin_family;			//��ַ���壬һ����AF_INET,����TCP/IPЭ����
	int sin_port;					//�˿ں�
	int ip;				            //ip��ַ
	int max_link;					//���������
	char buf[BUFFER_SIZE];   		//���ڽ��ջ���
};

struct com_attr{
	int com_port;					//���ڶ˿ں�
	int baud_rate;					//������
	int data_bits;					//����λ
	char parity;					//��żУ��λ
	int stop_bits;					//ֹͣλ
	struct uart_buf uart_buf;		//���ڽ��ջ���
};

struct int_attr{
	char path[64];					//�豸·��
	char buf[BUFFER_SIZE];			//���ݽ��ջ���
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
	int count;						//���豸����

    //select����ʹ�ò���
    fd_set fd_set;				//fd_set
    fd_set tmp_set;				//tmp_set
    int max_fd;					//�ļ����������ֵ
    sem_t sem[2];				//��0�ݣ����ݽ��գ���1�ݣ����ݷ���

    struct dev_queue recv_queue;
    struct dev_queue send_queue;
    struct list_head list_head;         //�豸����

};


struct device {
	int dev_id;							//�豸ID��
	int fd;								//�豸�򿪺���ļ�������
	int type;							//�豸���� 1:�����豸��2:�����豸,3:һ���豸
    int valid;
    union {								//�豸��������
		struct net_attr net_attr;		//�������
		struct com_attr com_attr;		//���ڲ���
		struct int_attr int_attr;		//�ж��豸
	};
    struct sockaddr_in  dest_addr;
    int sock_len;
    
	sem_t *p_sem[2];					//��0�ݣ����ݽ��գ���1�ݣ����ݷ���
	struct data_head data_head[3];		//���� ��0�ݣ��������ݣ���1�ݣ��������ݣ���2�ݣ��ļ�������
	int (*open_dev)(struct device *pdev,struct dev_head *dev_head);	//�豸�򿪲�������
	int (*close_dev)(struct device *pdev,struct dev_head *dev_head);//�豸�رպ���
	int (*send_data)(struct device *pdev,struct dev_head *dev_head);//�豸�����ݷ��ͺ���
	int (*recv_data)(struct device *pdev,struct dev_head *dev_head);//�豸�����ݽ��պ���
	struct list_head list_head;			//�豸����
};




Uint8 add_recv(struct dev_head *dev_head,struct device *p_dev,char *data,int len);
Uint8 add_send(struct dev_head *dev_head,struct device *p_dev,char *data,int len);

#endif /* LIST_CTRL_H_ */
