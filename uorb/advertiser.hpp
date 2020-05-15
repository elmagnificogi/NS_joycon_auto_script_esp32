#pragma once
#include "vfs_api.h"
#include "inode_utils.h"
#include "topic_data.hpp"

namespace UORB
{	

class ADVERTISER{
    public:
    ADVERTISER();

    int advertise(const char* topic_name);
    int publish(TOPIC_DATA* data);
    int lock();
    int unlock();
    
    
    uint32_t       topic_size;
    struct inode  *inode;
    struct file    file;
    ADVERTISER*    nxt;// used  in adv list on a topic
    ADVERTISER*    next;// used  in adv cluster
};

}
