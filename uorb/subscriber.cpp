#include "subscriber.hpp"
#include "dev_topic.hpp"


namespace UORB
{	

SUBSCRIBER::SUBSCRIBER(){
    nxt = NULL;
    next= NULL;
    topic_size = 0;
    generation = 0;
    sema = xSemaphoreCreateBinary();
    inode=NULL;
}

int SUBSCRIBER::subscribe(const CHAR* topic_name){
    char topic_dev_path[UORB_FULL_PATH_LEN+1] = "/uorb/";
    strncat(topic_dev_path, topic_name, VFS_ENTRY_NAME_MAX);

    int ret;
    ret = open_driver(topic_dev_path, &inode);
    if(ret<0){
        printk("ERROR: Subscribe to %s failed. Could not open topic\n", topic_name);
        return ERR;
    }

    file.f_inode = inode;

    ret = inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_ADD_SUB, (unsigned long)this);
    if(ret!=OK){
        printk("ERROR: Subscribe to %s failed. Could not add sub to topic\n", topic_name);
        return ret;
    }

    topic_size = (uint32_t)inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_GET_SIZE, 0);

    return OK;
}

int SUBSCRIBER::_fetch(TOPIC_DATA* dst, uint32_t timeout, bool copy){

    if(xSemaphoreTake(sema, timeout)==pdPASS){
    	generation = (uint32_t) inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_FETCH, (unsigned long)(dst));
    	return OK;
    }else{
    	if(copy){
    		inode->u.i_ops->ioctl(&file, UORB_DEV_TOPIC_IOC_FETCH, (unsigned long)(dst));
    	}
    	return ERR;
    }
}

void SUBSCRIBER::notify(){
    xSemaphoreGive(sema);
}

}
