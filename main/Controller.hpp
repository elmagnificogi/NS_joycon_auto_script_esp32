#pragma once

#include "com_types.hpp"

namespace controller
{	

class CONTROLLER{
    public:
    CONTROLLER();

    virtual int run_script();
    virtual int run_cmd();
    virtual int pair();
    virtual uint16_t get_version();
    virtual int set_version();
    virtual MAC get_mac_addr();
    virtual int set_mac_addr();
    virtual SKIN get_color();
    virtual int set_color();
    virtual int get_battery();    
    virtual int set_battery();
    
    CONTROLLER_STATUS status;
    MAC mac;
    SKIN color;
    CONTROLLER_BUTTON button;
    CONTROLLER_OTHER_CMD other_cmd;
    uint16_t version;
};

}

