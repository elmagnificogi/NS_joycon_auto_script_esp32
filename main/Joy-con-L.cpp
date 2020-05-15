#include "Controller.hpp"

namespace controller
{	

JOY_CON_L::JOY_CON_L(){
}

uint16_t JOY_CON_L:: get_version(){
        return version;
};

int JOY_CON_L:: set_version(){
};

MAC JOY_CON_L:: get_mac_addr(){
    return mac;
};

int JOY_CON_L:: set_mac_addr();

SKIN JOY_CON_L:: get_color(){
    return color;
};

int JOY_CON_L:: set_color();

int JOY_CON_L:: get_battery(){
    
};

int JOY_CON_L:: set_battery();

}
