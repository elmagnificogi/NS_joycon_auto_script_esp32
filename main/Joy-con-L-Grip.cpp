#include "Controller.hpp"

namespace controller
{	

JOY_CON_GRIP::JOY_CON_GRIP(){
}

uint16_t JOY_CON_GRIP:: get_version(){
        return version;
};

int JOY_CON_GRIP:: set_version(){
};

MAC JOY_CON_GRIP:: get_mac_addr(){
    return mac;
};

int JOY_CON_GRIP:: set_mac_addr();

SKIN JOY_CON_GRIP:: get_color(){
    return color;
};

int JOY_CON_GRIP:: set_color();

int JOY_CON_GRIP:: get_battery(){
    
};

int JOY_CON_GRIP:: set_battery();

}
