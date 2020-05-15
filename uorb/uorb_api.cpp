#include "uorb_api.hpp"
#include "topics.hpp"

#define MAX_TOPIC_NUM 64

int uorb_init(){

    if(UORB::register_topics()!=OK){
        return ERR;
    }

    return OK;
}

namespace UORB
{

TOPIC_DATA *topic_pool[MAX_TOPIC_NUM];

TOPIC_DATA* alloc_topic_data(uint32_t id){
    return topic_pool[id];
}

int register_topics(){
    // the id use in the REG_TOPIC Macro
    uint32_t topic_id=0;

    REG_TOPIC(TP_NOTIFY);
	

    //REG_TOPIC_MULTI(TP_SET_RELAY, 2);

    return OK;
}

}
