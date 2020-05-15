#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "vfs_api.h"
#include "inode_utils.h"
#include "topic_data.hpp"

namespace UORB
{	

class SUBSCRIBER{
    public:
    SUBSCRIBER();

    int         subscribe(const CHAR* topic_name);
    int         try_fetch(TOPIC_DATA* dst)           {return _fetch(dst, 0, true);};
    int         try_fetch(TOPIC_DATA* dst, bool copy){return _fetch(dst, 0, copy);};
    int         fetch(TOPIC_DATA* dst)                  {return _fetch(dst, portMAX_DELAY, false);};
    int         fetch(TOPIC_DATA* dst, uint32_t timeout){return _fetch(dst, timeout, false);}
    int         _fetch(TOPIC_DATA* dst, uint32_t timeout, bool copy);
    void        notify();

    uint32_t         generation;
    uint32_t         topic_size;
    struct inode    *inode;
    struct file      file;
    SUBSCRIBER*      nxt; // used in blocking list of a topic
    SUBSCRIBER*      next; // used in sub_cluster
    xSemaphoreHandle sema;

};

}
