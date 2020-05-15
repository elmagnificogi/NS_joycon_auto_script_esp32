#pragma once

#include <stdlib.h>
#include "dev_topic.hpp"
#include "advertiser.hpp"
#include "subscriber.hpp"
#include "uorb_def.hpp"
#include "topics.hpp"



#define REG_TOPIC_SINGLE(tp_name, tp_type) \
    do { \
        DEV_TOPIC *topic_dev  = new DEV_TOPIC(sizeof(tp_type)); \
        if(topic_dev==NULL){ \
            return -ENOMEM; \
        } \
        topic_dev->id = topic_id; \
        topic_pool[topic_id] = new tp_type(); \
        topic_id++; \
        tp_type topic_initializer; \
        memcpy(topic_dev->data, &topic_initializer, sizeof(tp_type)); \
        char path[UORB_FULL_PATH_LEN+1] = "/uorb/"; \
        strcat(path, (tp_name)); \
        int ret; \
        ret = register_driver(path, &(g_dev_topic_ops), 0666, topic_dev); \
        if(ret<0){ \
            return ret; \
        } \
    }while(0)

#define REG_TOPIC(name) REG_TOPIC_SINGLE(#name, name)

#define REG_TOPIC_MULTI(tp_type, instance) \
    do{ \
        if(instance>UORB_TP_MAX_INSTANCE){ \
            return ERR; \
        } \
        char tp_type_name[VFS_ENTRY_NAME_MAX+1] = #tp_type;\
        char tp_inst_name[VFS_ENTRY_NAME_MAX+1]; \
        char suffix[UORB_TP_SUFFIX_WIDTH+1]; \
        for(int i=0; i<instance; i++){ \
            itoa(i, suffix, 10); \
            strcpy(tp_inst_name, tp_type_name); \
            strcat(tp_inst_name, suffix); \
            REG_TOPIC_SINGLE(tp_inst_name, tp_type); \
        } \
    }while(0)

extern "C" int uorb_init();

namespace UORB
{
TOPIC_DATA* alloc_topic_data(uint32_t id);
int register_topics();

}
