#include "dev_topic.hpp"

namespace UORB
{	

static int     dev_topic_open(struct file *filep );
static int     dev_topic_close(struct file *filep );
static ssize_t dev_topic_read(struct file *filep, char *buffer, size_t buflen);
static ssize_t dev_topic_write(struct file *filep, const char *buffer, size_t buflen);
static off_t   dev_topic_seek(struct file *filep, off_t offset, int whence);
static int     dev_topic_ioctl(struct file *filep, int option, unsigned long param);

struct file_operations g_dev_topic_ops =
{
    dev_topic_open ,
    dev_topic_close,
    dev_topic_read ,
    dev_topic_write,
    dev_topic_seek ,
    dev_topic_ioctl
};

static int     dev_topic_open(struct file *filep)
{
    return OK;
}

static int     dev_topic_close(struct file *filep)
{
    return OK;
}

static ssize_t dev_topic_read(struct file *filep, char *buffer, size_t size)
{
    // read is forbiden for this device
    return ERR;
}

static ssize_t dev_topic_write(struct file *filep, const char *buffer, size_t size)
{
    struct DEV_TOPIC * dev = (DEV_TOPIC *)(filep->f_inode->i_private);
    configASSERT(dev->size==size);

    void * writer = filep->f_priv;

    taskENTER_CRITICAL();
    // if the topic is locked by writer, then, only the write who lock the topic can really
    // do write operation, otherwise, write will dropped
    if(!dev->locked || writer==dev->locker){
    	memcpy(dev->data, buffer, size);
    	dev->generation++;
    	dev->notify_all();
    }
    taskEXIT_CRITICAL();

    return size;
}

static int dev_topic_ioctl(struct file *filep, int option, unsigned long param)
{
    struct DEV_TOPIC * dev = (DEV_TOPIC *)(filep->f_inode->i_private);
    int ret;

    switch(option){
        case UORB_DEV_TOPIC_IOC_ADD_ADV:
            {
            ADVERTISER* adv = (ADVERTISER*)param;
            return dev->insert_adv(adv);
            }
        case UORB_DEV_TOPIC_IOC_ADD_SUB:
            {
            SUBSCRIBER* sub = (SUBSCRIBER*)param;
            return dev->insert_sub(sub);
            }
        case UORB_DEV_TOPIC_IOC_GET_SIZE:
            {
            return (int)(dev->size);
            }
        case UORB_DEV_TOPIC_IOC_GET_GEN:
            {
            return (int)(dev->generation);
            }
        case UORB_DEV_TOPIC_IOC_FETCH:
            {
            taskENTER_CRITICAL();
            {
                memcpy( (uint8_t*)param, dev->data, dev->size );
                ret = (int)(dev->generation);
            }
            taskEXIT_CRITICAL();
            return ret;
            }
        case UORB_DEV_TOPIC_IOC_STA:
            {
            float * sta_rate = (float *)param;
            uint32_t now = xTaskGetTickCount();
            uint32_t gen = dev->generation;
            float rate = (float)(gen - dev->last_sta_gen)*1000.0/(now - dev->last_sta_time);
            *sta_rate = rate;
            dev->last_sta_time = now;
            dev->last_sta_gen  = gen;
            return OK;
            }
        case UORB_DEV_TOPIC_IOC_LOCK:
        	{
        	void * locker = filep->f_priv;
        	int ret = ERR;
            taskENTER_CRITICAL();
            if(!dev->locked){
            	dev->locked = true;
            	dev->locker = locker;
            	ret = OK;
            }
            taskEXIT_CRITICAL();
            return ret;
        	}
        case UORB_DEV_TOPIC_IOC_UNLOCK:
        	{
        	void * unlocker = filep->f_priv;
        	int ret = ERR;
            taskENTER_CRITICAL();
            if(!dev->locked || unlocker == dev->locker){
            	dev->locked = false;
            	dev->locker = NULL;
            	ret = OK;
            }
            taskEXIT_CRITICAL();
            return ret;
        	}
        default:
            return ERR;
    }
}

static off_t   dev_topic_seek(struct file *filep, off_t offset, int whence)
{
    return 0;
}

DEV_TOPIC::DEV_TOPIC(ssize_t size)
{
    generation    = 0;
    this->size    = size;
    data          = new U8[size];
    first_adv     = NULL;
    first_sub     = NULL;
    last_sta_time = 0;
    last_sta_gen  = 0;
    locked        = false;
    locker        = NULL;
    id =0;
}

void DEV_TOPIC::notify_all()
{
    SUBSCRIBER *cur_sub = first_sub;
    while(cur_sub!=NULL){
        cur_sub->notify();
        cur_sub = cur_sub->nxt;
    }
}

int DEV_TOPIC::insert_adv(ADVERTISER *adv)
{
    if(adv==NULL)
        return ERR;

    adv->nxt = first_adv;
    first_adv = adv;

    return OK;
}

int DEV_TOPIC::insert_sub(SUBSCRIBER *sub)
{
    if(sub==NULL)
        return ERR;

    sub->nxt = first_sub;
    first_sub = sub;

    return OK;
}

}
