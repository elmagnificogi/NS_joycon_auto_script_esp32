#include "Controller.hpp"

namespace controller
{	

JOY_CON_R::JOY_CON_R(){
}

uint16_t JOY_CON_R:: get_version(){
        return version;
};

int JOY_CON_R:: set_version(){
};

MAC JOY_CON_R:: get_mac_addr(){
    return mac;
};

int JOY_CON_R:: set_mac_addr();

SKIN JOY_CON_R:: get_color(){
    return color;
};

int JOY_CON_R:: set_color();

int JOY_CON_R:: get_battery(){
    
};

int JOY_CON_R:: set_battery();

}
