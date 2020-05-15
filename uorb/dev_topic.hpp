#pragma once

#include "vfs_api.h"
#include "advertiser.hpp"
#include "subscriber.hpp"

#define UORB_DEV_TOPIC_IOC_ADD_ADV  (0)
#define UORB_DEV_TOPIC_IOC_ADD_SUB  (1)
#define UORB_DEV_TOPIC_IOC_GET_SIZE (2)
#define UORB_DEV_TOPIC_IOC_GET_GEN  (3)
#define UORB_DEV_TOPIC_IOC_FETCH    (4)
#define UORB_DEV_TOPIC_IOC_STA      (5)
#define UORB_DEV_TOPIC_IOC_LOCK     (6)
#define UORB_DEV_TOPIC_IOC_UNLOCK   (7)

namespace UORB
{	

extern struct file_operations g_dev_topic_ops;

struct DEV_TOPIC
{
    uint32_t id;

    uint32_t size;
    uint8_t* data;
    uint32_t generation;
    ADVERTISER* first_adv;
    SUBSCRIBER* first_sub;

    bool  locked;
    void* locker;

    // used for publish rate statistic
    uint32_t last_sta_time; // ms
    uint32_t last_sta_gen;

    DEV_TOPIC(ssize_t size);

    void notify_all();
    int  insert_adv(ADVERTISER* adv);
    int  insert_sub(SUBSCRIBER* sub);
};

}
