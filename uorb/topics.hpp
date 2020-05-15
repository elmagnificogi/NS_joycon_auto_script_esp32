#pragma once

#include "topic_data.hpp"
#include "com_types.hpp"


namespace UORB
{

struct TP_NOTIFY: public TOPIC_DATA{
    uint8_t  status;
    virtual ~TP_NOTIFY(){}
    void zero(){
        status =NOTIFY_IDLE;
    }

    void print(){
        printf(
                "\n \
               status : %d   \n \
\n",
               status 
            ); // end of printf
    } // end of print
};	

}
