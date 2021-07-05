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

struct dev_head{
	int count;						//���豸����
	pthread_rwlock_t dev_rwlock;	//�豸���������д��
	struct {						//select����ʹ�ò���
		fd_set fd_set;				//fd_set
		fd_set tmp_set;				//tmp_set
		int max_fd;					//�ļ����������ֵ
		sem_t sem[2];				//��0�ݣ����ݽ��գ���1�ݣ����ݷ���
	};
	struct list_head list_head;		//�豸����ͷ
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

struct device {
	int dev_id;							//�豸ID��
	int fd;								//�豸�򿪺���ļ�������
	int type;							//�豸���� 1:�����豸��2:�����豸,3:һ���豸
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






static inline int add_list(struct list_head *new, struct list_head *head,pthread_rwlock_t *list_rwlock) {
	if(new == NULL)
		return FALSE;
	pthread_rwlock_wrlock(list_rwlock);
	list_add(new,head);
	pthread_rwlock_unlock(list_rwlock);
	return TRUE;
}

static inline int add_tail_list(struct list_head *new, struct list_head *head,pthread_rwlock_t *list_rwlock) {
	if(new == NULL)
		return FALSE;
	pthread_rwlock_wrlock(list_rwlock);
	list_add_tail(new,head);
	pthread_rwlock_unlock(list_rwlock);
	return TRUE;
}

static inline int del_list(struct list_head *entry,pthread_rwlock_t *list_rwlock) {
	pthread_rwlock_wrlock(list_rwlock);
	list_del(entry);
	pthread_rwlock_unlock(list_rwlock);
	return TRUE;
}

static inline int inc_data_count(struct data_head *data_head) {
	pthread_rwlock_wrlock(data_head->list_rwlock);
	data_head->count++;
	pthread_rwlock_unlock(data_head->list_rwlock);
	return TRUE;
}

static inline int dec_data_count(struct data_head *data_head) {
	pthread_rwlock_wrlock(data_head->list_rwlock);
	data_head->count--;
	pthread_rwlock_unlock(data_head->list_rwlock);
	return TRUE;
}

static inline int malloc_new(void **p_data,int len) {
	void * new_data;
	new_data = (void *)malloc(len);
	if(new_data == NULL)
		return FALSE;
	bzero(new_data,len);
	*p_data = new_data;
	return TRUE;
}

static inline int m_data_head(struct data_head **data_head) {
	struct data_head * new_head;
	new_head = (struct data_head *)malloc(sizeof(struct data_head));
	if(new_head == NULL)
		return FALSE;
	bzero(new_head,sizeof(struct data_head));
	*data_head = new_head;
	return TRUE;
}

static inline int m_data_list(struct data_list **data_list) {
	struct data_list * new_list;
	new_list = (struct data_list *)malloc(sizeof(struct data_list));
	if(new_list == NULL)
		return FALSE;
	bzero(new_list,sizeof(struct data_head));
	*data_list = new_list;
	return TRUE;
}

static inline void init_data_head(struct data_head *new_head,int type,pthread_rwlock_t *list_rwlock) {
	new_head->type = type;
	new_head->list_rwlock = list_rwlock;
    new_head->count = 0;
	INIT_LIST_HEAD(&new_head->list_head);
}

static inline struct data_list *init_data_list(struct data_list **data_list) {
	struct data_list *new_list;
	if(!m_data_list(&new_list))
		return NULL;
	bzero(new_list,sizeof(struct data_list));
	INIT_LIST_HEAD(&new_list->list_head);
	*data_list = new_list;
	return new_list;
}

static inline struct data_list *find_list(int fd,struct data_head *data_head) {
	struct data_list *p_data_list;
	list_for_each_entry(p_data_list,&data_head->list_head,list_head) {
		if (p_data_list->net_lev.fd == fd)
			return p_data_list;
	}
	return NULL;
}

static inline struct device *find_dev(int dev_id,struct dev_head *dev_head) {
	struct device *p_dev;
	list_for_each_entry(p_dev,&dev_head->list_head,list_head) {
		if (p_dev->dev_id == dev_id)
			return p_dev;
	}
	return NULL;
}

static inline void *malloc_data(char **list_data,char *src_data,int len) {
	*list_data = (char *)malloc(len*sizeof(char));
	if (*list_data == NULL)
		return NULL;
    memset(*list_data,0,len);
	memcpy(*list_data,src_data,len);
	return *list_data;
}

static inline struct data_list *add_net_data(int fd,struct sockaddr_in *dev_sockaddr,struct data_head *data_head) {
	struct data_list *new_list;
	struct net_lev *p_net_lev;
	if(data_head == NULL)
		return NULL;
	if(!init_data_list(&new_list))
		return NULL;
	p_net_lev = &new_list->net_lev;
	memcpy(&p_net_lev->dev_sockaddr,dev_sockaddr,sizeof(struct sockaddr_in));
	p_net_lev->fd= fd;
	add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
	inc_data_count(data_head);
	return new_list;
}

static inline struct data_list *add_recv_data(char *data,int fd,int len,struct data_head *data_head)	//���ӽ������ݵ�Ԫ��������������
{
	struct data_list *new_list;
	struct recv_lev *p_recv_lev;
	if(data_head == NULL)
		return NULL;
	time_t time_now;
	if(!init_data_list(&new_list))
		return NULL;
	p_recv_lev = &new_list->recv_lev;

        malloc_data(&p_recv_lev->data,data,len);
	//p_recv_lev->recv_time = time_now;
	p_recv_lev->fd = fd;
	p_recv_lev->len = len;
	p_recv_lev->deal_time = 0;
	add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
	inc_data_count(data_head);
	return new_list;
}

static inline struct data_list *add_send_data(char *data,int len,struct data_head *data_head) {
	int *p_int = NULL;
	struct list_head *p_list = NULL;
	struct data_list *new_list;
	struct send_lev *p_send_lev;
	if(data_head == NULL)
		return NULL;
	time_t time_now;
	if(!init_data_list(&new_list))
		return NULL;
	p_send_lev = &new_list->send_lev;

        malloc_data(&p_send_lev->data,data,len);
	p_send_lev->len = len;
	//p_send_lev->add_time = time_now;
	p_send_lev->send_time = 0;

	add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
	inc_data_count(data_head);
	return new_list;
}

static inline struct data_list *add_net(int fd,struct sockaddr_in *dev_sockaddr,struct device *p_dev) {
	struct data_list *p_data_list = NULL;
	p_data_list = add_net_data(fd,dev_sockaddr,&p_dev->data_head[2]);
	return p_data_list;
}

static inline struct data_list *add_recv(char *data,int fd,int len,struct device *p_dev) {
	struct data_list *p_data_list = NULL;
	p_data_list = add_recv_data(data,fd,len,&p_dev->data_head[0]);
	sem_post(p_dev->p_sem[0]);
	return p_data_list;
}

static inline struct data_list *add_send(char *data,int len,struct device *p_dev) {
	struct data_list *p_data_list = NULL;
        p_data_list = add_send_data(data,len,&p_dev->data_head[1]);

    // if malloc fail return NULL
    if(p_data_list == NULL)
        return NULL;
    
	sem_post(p_dev->p_sem[1]);
	return p_data_list;
}

static inline int del_data_list(struct data_list *data_list,struct data_head *data_head) {
	del_list(&data_list->list_head,data_head->list_rwlock);
	switch(data_head->type) {
	case 1:
		free(data_list->recv_lev.data);
		break;
	case 2:
		free(data_list->send_lev.data);
		break;
	case 3:
		break;
	default :
		return FALSE;
		break;
	}
	free(data_list);
	dec_data_count(data_head);
	return TRUE;
}

static inline int del_data(struct data_list *data_list,struct data_head *data_head,struct device *p_dev) {
	del_list(&data_list->list_head,data_head->list_rwlock);
	switch(data_head->type) {
	//Bug20180108:��Ϊcase���������½������ݵ������޷��������գ������ڴ�ʹ����һֱ���ǡ�
	
	case 0: /**����  */
		//sem_trywait(p_dev->p_sem[0]);
		free(data_list->recv_lev.data);
        free(data_list);
	    dec_data_count(data_head);
		break;
	case 1: /**����  */
		//sem_trywait(p_dev->p_sem[1]);
		free(data_list->send_lev.data);
        free(data_list);
	    dec_data_count(data_head);
		break;
	case 3:
		break;
	default :
		return FALSE;
	}
	
	return TRUE;
}

static inline int del_data_head(struct data_head *data_head) {
	free(data_head);
	return TRUE;
}

#endif /* LIST_CTRL_H_ */
