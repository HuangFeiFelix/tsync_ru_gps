/*
 * list_ctrl.c
 *
 *  Created on: 2021-4-21
 *      Author: administrator
 */

#include "list_ctrl.h"
#include "log.h"

 
 void dev_queue_init(struct dev_queue *p_queue)
 {
     int i;
     
     p_queue->head = 0;
     p_queue->tail = 0;
     p_queue->count = 0;
 
     for(i=0;i<QUEUE_SIZE;i++)
     {
         memset(p_queue->msg[i].r_buf,0,QUEUE_BUFFER_LEN);
         p_queue->msg[i].r_len = 0;
         p_queue->msg[i].type= 0;
         p_queue->msg[i].dev_id = 0;
     }    
 }
 
 int  dev_queue_size(const struct dev_queue *p_queue) {
 
     return p_queue->count;
 
 }
 
 Uint8 dev_queue_empty(const struct dev_queue *p_queue) {
     if(p_queue->count == 0)
         return TRUE;
     else 
         return FALSE;
 }
 
 int dev_queue_full(const struct dev_queue *p_queue) {
     if(p_queue->count >= QUEUE_SIZE)
         return TRUE;
     else
         return FALSE;
 }
 
 int dev_queue_push(struct dev_queue *p_queue, struct device *pdev, Uint8 *buffer, Uint16 len) 
 { 
     Uint8 *r_buf = NULL;
     Uint16 length = 0;
     
     if (dev_queue_full(p_queue)) 
     { 
         return FALSE; 
     }
     
     r_buf = p_queue->msg[p_queue->tail].r_buf;
     if(r_buf == NULL)
     {
         return FALSE;
     }
     if(len > QUEUE_BUFFER_LEN)
     {
         return FALSE;
     }
     memcpy(r_buf,buffer,len);
     p_queue->msg[p_queue->tail].r_len = len;
     p_queue->msg[p_queue->tail].dev_id = pdev->dev_id;
     p_queue->msg[p_queue->tail].pdev = pdev;
     p_queue->tail = (p_queue->tail+1)%QUEUE_SIZE;
     p_queue->count++;
 
     return TRUE;
 
 }
 
 int dev_queue_pop(struct dev_queue *p_queue,Uint8 *buffer,Uint16 *len,int *dev_id)
 {
     
     memcpy(buffer,p_queue->msg[p_queue->head].r_buf,p_queue->msg[p_queue->head].r_len);
     *len = p_queue->msg[p_queue->head].r_len;
     *dev_id = p_queue->msg[p_queue->head].dev_id;
        
     memset(p_queue->msg[p_queue->head].r_buf,0,QUEUE_BUFFER_LEN);
     p_queue->msg[p_queue->head].r_len = 0;
     p_queue->msg[p_queue->head].dev_id = 0;
     p_queue->head = (p_queue->head+1)%QUEUE_SIZE;
     p_queue->count--;
  
     return TRUE; 
 
 }


Uint8 add_recv(struct dev_head *dev_head,struct device *p_dev,char *data,int len)
{
    struct dev_queue *p_queue = &dev_head->recv_queue;

    if(dev_queue_push(p_queue,p_dev,data,len) == FALSE)
    {
        syslog(CLOG_DEBUG,"add_recv dev_queue_push error");
        return FALSE;
    }
    
     return TRUE;
 }
 
Uint8 add_send(struct dev_head *dev_head,struct device *p_dev,char *data,int len) 
{

      struct dev_queue *p_queue = &dev_head->send_queue;
      
      if(dev_queue_push(p_queue,p_dev,data,len) == FALSE)
      {
          syslog(CLOG_DEBUG,"add_send dev_queue_push error");
          return FALSE;
      }
      
       return TRUE;

}

 
 int add_list(struct list_head *new, struct list_head *head,pthread_rwlock_t *list_rwlock) {
     if(new == NULL)
         return FALSE;
     pthread_rwlock_wrlock(list_rwlock);
     list_add(new,head);
     pthread_rwlock_unlock(list_rwlock);
     return TRUE;
 }

 int add_tail_list(struct list_head *new, struct list_head *head,pthread_rwlock_t *list_rwlock) {
     if(new == NULL)
         return FALSE;
     pthread_rwlock_wrlock(list_rwlock);
     list_add_tail(new,head);
     pthread_rwlock_unlock(list_rwlock);
     return TRUE;
 }
 
int del_list(struct list_head *entry,pthread_rwlock_t *list_rwlock) {
     pthread_rwlock_wrlock(list_rwlock);
     list_del(entry);
     pthread_rwlock_unlock(list_rwlock);
     return TRUE;
 }
 
 int inc_data_count(struct data_head *data_head) {
     pthread_rwlock_wrlock(data_head->list_rwlock);
     data_head->count++;
     pthread_rwlock_unlock(data_head->list_rwlock);
     return TRUE;
 }
 
int dec_data_count(struct data_head *data_head) {
     pthread_rwlock_wrlock(data_head->list_rwlock);
     data_head->count--;
     pthread_rwlock_unlock(data_head->list_rwlock);
     return TRUE;
 }
 
int malloc_new(void **p_data,int len) {
     void * new_data;
     new_data = (void *)malloc(len);
     if(new_data == NULL)
         return FALSE;
     bzero(new_data,len);
     *p_data = new_data;
     return TRUE;
 }
 
 int m_data_head(struct data_head **data_head) {
     struct data_head * new_head;
     new_head = (struct data_head *)malloc(sizeof(struct data_head));
     if(new_head == NULL)
         return FALSE;
     bzero(new_head,sizeof(struct data_head));
     *data_head = new_head;
     return TRUE;
 }
 
 int m_data_list(struct data_list **data_list) {
     struct data_list * new_list;
     new_list = (struct data_list *)malloc(sizeof(struct data_list));
     if(new_list == NULL)
         return FALSE;
     bzero(new_list,sizeof(struct data_head));
     *data_list = new_list;
     return TRUE;
 }
 
 void init_data_head(struct data_head *new_head,int type,pthread_rwlock_t *list_rwlock) {
     new_head->type = type;
     new_head->count = 0;
 }
 
 struct data_list *init_data_list(struct data_list **data_list) {
     struct data_list *new_list;
     if(!m_data_list(&new_list))
         return NULL;
     bzero(new_list,sizeof(struct data_list));
     INIT_LIST_HEAD(&new_list->list_head);
     *data_list = new_list;
     return new_list;
 }
 
struct data_list *find_list(int fd,struct data_head *data_head) {
     struct data_list *p_data_list;
     list_for_each_entry(p_data_list,&data_head->list_head,list_head) {
         if (p_data_list->net_lev.fd == fd)
             return p_data_list;
     }
     return NULL;
 }
 
struct device *find_dev(int dev_id,struct dev_head *dev_head) {
     struct device *p_dev;
     list_for_each_entry(p_dev,&dev_head->list_head,list_head) {
         if (p_dev->dev_id == dev_id)
             return p_dev;
     }
     return NULL;
 }
 
 void *malloc_data(char **list_data,char *src_data,int len) {
     *list_data = (char *)malloc(len*sizeof(char));
     if (*list_data == NULL)
         return NULL;
     memset(*list_data,0,len);
     memcpy(*list_data,src_data,len);
     return *list_data;
 }
 
 struct data_list *add_net_data(int fd,struct sockaddr_in *dev_sockaddr,struct data_head *data_head) {
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
 
struct data_list *add_recv_data(char *data,int fd,int len,struct data_head *data_head)    //增加接收数据单元到行向量链表中
 {
     struct data_list *new_list;
     struct recv_lev *p_recv_lev;
     if(data_head == NULL)
         return NULL;
     time_t time_now;
     if(!init_data_list(&new_list))
         return NULL;
     p_recv_lev = &new_list->recv_lev;
 
     if(NULL == malloc_data(&p_recv_lev->data,data,len))
     {
         return NULL;
     }
     //p_recv_lev->recv_time = time_now;
     p_recv_lev->fd = fd;
     p_recv_lev->len = len;
     p_recv_lev->deal_time = 0;
     add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
     inc_data_count(data_head);
     return new_list;
 }
 
struct data_list *add_send_data(char *data,int len,int type,struct data_head *data_head) {
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
 
     if(NULL == malloc_data(&p_send_lev->data,data,len))
     {
         return NULL;
     }
     
     p_send_lev->len = len;
     //p_send_lev->add_time = time_now;
     p_send_lev->send_time = 0;
 
     add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
     inc_data_count(data_head);
     return new_list;
 }
 
 struct data_list *add_net(int fd,struct sockaddr_in *dev_sockaddr,struct device *p_dev) {
     struct data_list *p_data_list = NULL;
     p_data_list = add_net_data(fd,dev_sockaddr,&p_dev->data_head[2]);
     return p_data_list;
 }
 

#if 0
struct data_list *add_send(char *data,int len,struct device *p_dev) {
     struct data_list *p_data_list = NULL;
     p_data_list = add_send_data(data,len,0,&p_dev->data_head[1]);
 
     // if malloc fail return NULL
     if(p_data_list == NULL)
         return NULL;
     
     sem_post(p_dev->p_sem[1]);
     return p_data_list;
 }
 
#endif
 int del_data_list(struct data_list *data_list,struct data_head *data_head) {
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

 int del_data(struct data_list *data_list,struct data_head *data_head,struct device *p_dev) {
     del_list(&data_list->list_head,data_head->list_rwlock);
     switch(data_head->type) {
     //Bug20180108:因为case条件错误导致接收数据的链表无法正常回收，导致内存使用率一直增涨。
     
     case 0: /**接收  */
         //sem_trywait(p_dev->p_sem[0]);
         if(data_list->recv_lev.data != NULL)
         {
             free(data_list->recv_lev.data);
             data_list->recv_lev.data = NULL;
 
         }
         if(data_list != NULL)
         {
             free(data_list);
         }
         dec_data_count(data_head);
         break;
     case 1: /**发送  */
         //sem_trywait(p_dev->p_sem[1]);
         
         if(data_list->send_lev.data != NULL)
         {
             free(data_list->send_lev.data);
             data_list->send_lev.data = NULL;
 
         }
         if(data_list != NULL)
         {
             free(data_list);
         }
         dec_data_count(data_head);
         break;
     case 2:
         break;
     case 3:
         break;
     default :
         return FALSE;
     }
     
     return TRUE;
 }
 
int del_data_head(struct data_head *data_head) {
     free(data_head);
     return TRUE;
 }

