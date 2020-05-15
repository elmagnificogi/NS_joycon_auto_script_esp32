#pragma once

namespace UORB
{	

struct TOPIC_DATA{
    virtual void print();
    virtual void zero()=0;
    virtual ~TOPIC_DATA(){}
};	

}
