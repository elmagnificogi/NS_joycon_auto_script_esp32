#pragma once

#include <stdint.h>

enum NOTIFY_STATUS{
    NOTIFY_IDLE            =0,
    NOTIFY_INIT            ,
    NOTIFY_PAIRING         ,
    NOTIFY_CONNECTED       ,
    NOTIFY_SCRIPT_START    ,
    NOTIFY_SCRIPT_PAUSE    ,
    NOTIFY_SCRIPT_STOP     ,
    NOTIFY_SCRIPT_OVER     ,
    NOTIFY_ERROR           
		
};

enum CONTROLLER_STATUS{
    CONTROLLER_IDLE            =0,
    CONTROLLER_INIT            ,
    CONTROLLER_PAIRING         ,
    CONTROLLER_CONNECTED       ,
    CONTROLLER_DISCONNECTED    ,
    CONTROLLER_ERROR           
		
};

typedef uint8_t MAC[6];

/*
Color Name	                 Body HEX	    Button HEX
Gray                         #828282	    #0F0F0F
Neon Red         	         #FF3C28	    #1E0A0A
Neon Blue	    	         #0AB9E6	    #001E1E
Neon Yellow 	             #E6FF00        #142800
Neon Green	                 #1EDC00	    #002800
Neon Pink	                 #FF3278	    #28001E
Red                          #E10F00	    #280A0A
Blue    	                 #4655F5	    #00000A
Neon Purple             	 #B400E6	    #140014
Neon Orange                  #FAA005	    #0F0A00
Pokemon Let's Go! Pikachu    #FFDC00	    #322800
Pokemon Let's Go! Eevee      #C88C32	    #281900
Nintendo Labo Creators Contest Edition
                             #D7AA73	    #1E1914
infact it's just the RGB HEX,you could fill any color you want
*/
struct SKIN {
    uint8_t body_color[3];
    uint8_t button_color[3];
    uint8_t left_grip_color[3];
    uint8_t right_grip_color[3];
    union uint8_t buf[12];
};

enum CONTROLLER_BUTTON{
    BUTTON_Y            =0x000001,
    BUTTON_X            =0x000002,
    BUTTON_B            =0x000004,
    BUTTON_A            =0x000008,
    BUTTON_SR_L         =0x000010,
    BUTTON_SL_L         =0x000020,    
    BUTTON_R            =0x000040,
    BUTTON_ZR           =0x000080,
    BUTTON_MINUS        =0x000100,
    BUTTON_PLUS         =0x000200,
    BUTTON_R_STICK      =0x000400,
    BUTTON_L_STICK      =0x000800,
    BUTTON_HOME         =0x001000,
    BUTTON_CAPTURE      =0x002000,
    BUTTON_NONE         =0x004000,
    BUTTON_CHARGINGGRIP =0x008000,
    BUTTON_DOWN         =0x010000,
    BUTTON_UP           =0x020000,
    BUTTON_RIGHT        =0x040000,	
    BUTTON_LEFT         =0x080000,
    BUTTON_SR_L         =0x100000,
    BUTTON_SL_L         =0x200000,    
    BUTTON_L            =0x400000,
    BUTTON_ZL           =0x800000
};

typedef uint8_t CONTROLLER_STICK[3];

// support for judge rumble and motion sensing aiming
// maybe there should more opt like simulate IR camera or NFC ?
enum CONTROLLER_OTHER_OPT{
    RUMBLE              =0x01,
    AIMING              =0x02
}

struct CONTROLLER_OTHER_CMD{
    CONTROLLER_OTHER_OPT opt,
    uint8_t opt_param[4];
    union uint8_t buf[5];
}

struct CMD{
    CONTROLLER_BUTTON button;
    CONTROLLER_STICK left_stick;
    CONTROLLER_STICK right_stick;
    CONTROLLER_OTHER_CMD other;
    union uint8_t buf[14];
}

