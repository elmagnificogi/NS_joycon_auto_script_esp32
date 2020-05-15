#include "advertiser.hpp"
#include "dev_topic.hpp"

namespace UORB
{	

ADVERTISER::ADVERTISER(){
    nxt = NULL;
    next= NULL;
    topic_size = 0;
    inode = NULL;
}

int ADVERTISER::advertise(const char* topic_name){
    char topic_dev_path[UORB_FULL_PATH_LEN+1] = "/uorb/";
    strncat(topic_dev_path, topic_name, VFS_ENTRY_NAME_MAX);

    int ret;
    ret = open_driver(topic_dev_path, &inode);
    if(ret<0){
        printk("ERROR: Advertise to %s failed. Could not open topic\n", topic_name);
        return ERR;
    }

    file.f_inode = inode;
    file.f_priv  = this;

    ret = inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_ADD_ADV, (unsigned long)this);
    if(ret!=OK){
        printk("ERROR: Advertise to %s failed. Could not add adv to topic\n", topic_name);
        return ret;
    }

    topic_size = inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_GET_SIZE, 0);

    return OK;
}

int ADVERTISER::lock()
{
	return inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_LOCK, 0);
}

int ADVERTISER::unlock()
{
	return inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_UNLOCK, 0);
}

int ADVERTISER::publish(TOPIC_DATA* data){
    return inode->u.i_ops->write(&file, (const char *)data, topic_size);
}

}
