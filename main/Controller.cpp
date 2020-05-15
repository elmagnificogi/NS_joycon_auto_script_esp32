#include "Controller.hpp"

namespace controller
{	

CONTROLLER::CONTROLLER(){
}

uint16_t CONTROLLER:: get_version(){
        return version;
};

int CONTROLLER:: set_version(){
};

MAC CONTROLLER:: get_mac_addr(){
    return mac;
};

int CONTROLLER:: set_mac_addr();

SKIN CONTROLLER:: get_color(){
    return color;
};

int CONTROLLER:: set_color();

int CONTROLLER:: get_battery(){
    
};

int CONTROLLER:: set_battery();

}
