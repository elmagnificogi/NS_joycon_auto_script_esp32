//
//  BlueCubeMod Firmware
//
//
//  Created by Nathan Reeves 2019
//

#include "esp_log.h"
#include "esp_hidd_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_gap_bt_api.h"
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"
#include <stdio.h>

#include "driver/uart.h"



#define LED_GPIO    21
#define PIN_SEL  (1ULL<<LED_GPIO)

static uint8_t but1_send=0;
static uint8_t but2_send=0;
static uint8_t but3_send=0;
static uint8_t ls1_send=0x7F;
static uint8_t rs1_send=0xF0;
static uint8_t ls2_send=0x7F;
static uint8_t rs2_send=0x07;
static uint8_t ls3_send=0x7F;
static uint8_t rs3_send=0x7F;

static uint8_t new_send = 0;
static uint8_t start_script = 0;

SemaphoreHandle_t xSemaphore;
bool connected = false;
int paired = 0;
TaskHandle_t SendingHandle = NULL;
TaskHandle_t BlinkHandle = NULL;
uint8_t timer = 0;

//Polls controller and formats response
//GameCube Controller Protocol: http://www.int03.co.uk/crema/hardware/gamecube/gc-control.html


//Switch button report example //         batlvl       Buttons              Lstick           Rstick
//static uint8_t report30[] = {0x30, 0x00, 0x90,   0x00, 0x00, 0x00,   0x00, 0x00, 0x00,   0x00, 0x00, 0x00};
static uint8_t report30[] = {
    0x30,
    0x0,
    0x80,
    0,//but1
    0,//but2
    0,//but3
    0,//Ls
    0,//Ls
    0,//Ls
    0,//Rs
    0,//Rs
    0,//Rs
    0x08
};

static uint8_t report3F[] = {
	0x3F,// report id
	0x00,//button status0
	0x00,//button status1
	0x00,//stick hat data
	0x00,// filler data
	0x80,
	0x00,
	0x80,
	0x00,
	0x80,
	0x00,
	0x80,
};

static uint8_t cmd[]={
  //cmd ,delay   
	0x00,200,
	0x05,10,
	0x11,10,
	0x05,10,
	0x08,10,
	0x06,10,
	0x00,10,
	0x06,30,
	0x00,200
};

void send_buttons()
{
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
	//ESP_LOGI("sb", "2" );
    report30[1] = timer;
    //buttons
    report30[3] = but1_send;
    report30[4] = but2_send;
    report30[5] = but3_send;
    //encode left stick
    report30[6] = ls1_send;
    report30[7] = ls2_send;
    report30[8] = ls3_send;
    //encode right stick
    report30[9] =  rs1_send;
    report30[10] = rs2_send;
    report30[11] = rs3_send;
    xSemaphoreGive(xSemaphore);
    timer+=1;
    if(timer == 255)
        timer = 0;
	
	if (esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, report30[0], sizeof(report30)-1, report30+1) != ESP_OK) {
		ESP_LOGI("STOP", "Failed");
	}
}
const uint8_t hid_descriptor_gamecube[] = {
	0x05, 0x01,
	0x09, 0x05,
	0xa1, 0x01,
	0x06, 0x01,
	0xff, 0x85,
	0x21, 0x09,
	0x21, 0x75,
	0x08, 0x95,
	0x30, 0x81,
	0x02, 0x85,
	0x30, 0x09,
	0x30, 0x75,
	0x08, 0x95,
	0x30, 0x81,
	0x02, 0x85,
	0x31, 0x09,
	0x31, 0x75,
	0x08, 0x96,
	0x69, 0x01,
	0x81, 0x02,
	0x85,
	0x32, 0x09,
	0x32, 0x75,
	0x08, 0x96,
	0x69, 0x01,
	0x81, 0x02,
	0x85, 0x33,
	0x09, 0x33,
	0x75, 0x08,
	0x96, 0x69,
	0x01, 0x81,
	0x02, 0x85,
	0x3f, 0x05, 0x09, 0x19, 0x01, 0x29, 0x10, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
	0x95, 0x10,
	0x81, 0x02, 0x05, 0x01, 0x09, 0x39, 0x15, 0x00, 0x25, 0x07, 0x75, 0x04,
	0x95, 0x01,
	0x81, 0x42,
	0x05, 0x09, 0x75, 0x04,
	0x95, 0x01,
	0x81, 0x01,
	0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x33, 0x09, 0x34, 0x16, 0x00, 0x00,
	0x27, 0xff, 0xff,
	0x00, 0x00, 0x75, 0x10,
	0x95, 0x04,
	0x81, 0x02,
	0x06, 0x01, 0xff,
	0x85, 0x01,
	0x09, 0x01,
	0x75, 0x08,
	0x95, 0x30,
	0x91, 0x02,
	0x85, 0x10,
	0x09, 0x10,
	0x75, 0x08,
	0x95, 0x30,
	0x91, 0x02,
	0x85, 0x11,
	0x09, 0x11,
	0x75, 0x08,
	0x95, 0x30,
	0x91, 0x02,
	0x85, 0x12,
	0x09, 0x12,
	0x75, 0x08,
	0x95, 0x30,
	0x91, 0x02,
	0xc0
    //0x75, 0x08,          //     report_size = 8
    //0x81, 0x03,          //     input = cnst,var,abs
    //Sticks
//    0x09, 0x30,        //   Usage (X)
//    0x09, 0x31,        //   Usage (Y)
//    0x09, 0x32,        //   Usage (Z)
//    0x09, 0x35,        //   Usage (Rz)
//    0x15, 0x00,        //   Logical Minimum (0)
//    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
//    0x75, 0x08,        //   Report Size (8)
//    0x95, 0x04,        //   Report Count (4)
//    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//    //DPAD
//    0x09, 0x39,        //   Usage (Hat switch)
//    0x15, 0x00,        //   Logical Minimum (0)
//    0x25, 0x07,        //   Logical Maximum (7)
//    0x35, 0x00,        //   Physical Minimum (0)
//    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
//    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
//    0x75, 0x04,        //   Report Size (4)
//    0x95, 0x01,        //   Report Count (1)
//    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
//    //Buttons
//    0x65, 0x00,        //   Unit (None)
//    0x05, 0x09,        //   Usage Page (Button)
//    0x19, 0x01,        //   Usage Minimum (0x01)
//    0x29, 0x0E,        //   Usage Maximum (0x0E)
//    0x15, 0x00,        //   Logical Minimum (0)
//    0x25, 0x01,        //   Logical Maximum (1)
//    0x75, 0x01,        //   Report Size (1)
//    0x95, 0x0E,        //   Report Count (14)
//    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//    //Padding
//    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
//    0x09, 0x20,        //   Usage (0x20)
//    0x75, 0x06,        //   Report Size (6)
//    0x95, 0x01,        //   Report Count (1)
//    0x15, 0x00,        //   Logical Minimum (0)
//    0x25, 0x7F,        //   Logical Maximum (127)
//    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//    //Triggers
//    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
//    0x09, 0x33,        //   Usage (Rx)
//    0x09, 0x34,        //   Usage (Ry)
//    0x15, 0x00,        //   Logical Minimum (0)
//    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
//    0x75, 0x08,        //   Report Size (8)
//    0x95, 0x02,        //   Report Count (2)
//    0x81, 0x02,
//    0xc0
};
int hid_descriptor_gc_len = sizeof(hid_descriptor_gamecube);
///Switch Replies
//                          repId Timer nibble|     button     |     L stick     |     R stick    | Vib   ACK  subId                         |    MAC ADDRESS                   | ???   color
static uint8_t reply02[] = {0x21, 0x01, 0x40, 0x00, 0x00, 0x00, 0xe6, 0x27, 0x78, 0xab, 0xd7, 0x76, 0x00, 0x82, 0x02, 0x03, 0x48, 0x03, 0x02, 0xD8, 0xA0, 0x1D, 0x40, 0x15, 0x66, 0x03, 0x00, 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 };
static uint8_t reply08[] = {0x21, 0x02, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 };
//                                                                                                                   input-mode
static uint8_t reply03[] = {0x21, 0x05, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 };
//                                                                                                                   |    L     |     R     |     ZL     |     ZR   |     SL    |     SR     |    HOME   |     
static uint8_t reply04[] = {0x21, 0x06, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x83, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00};
static uint8_t reply1060[]={0x21, 0x03, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x00, 0x60, 0x00, 0x00, 0x10, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 };
//                                                                                                                    color reg                    | color data   
static uint8_t reply1050[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x50, 0x60, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 };
static uint8_t reply1080[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x80, 0x60, 0x00, 0x00, 0x18, 0x5e, 0x01, 0x00, 0x00, 0xf1, 0x0f,
    0x19, 0xd0, 0x4c, 0xae, 0x40, 0xe1,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00};
static uint8_t reply1098[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x98, 0x60, 0x00, 0x00, 0x12, 0x19, 0xd0, 0x4c, 0xae, 0x40, 0xe1,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00};
//User analog stick calib
static uint8_t reply1010[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x10, 0x80, 0x00, 0x00, 0x18, 0x00, 0x00};
static uint8_t reply103D[]={0x21, 0x05, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x3D, 0x60, 0x00, 0x00, 0x19, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply1020[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x90, 0x10, 0x20, 0x60, 0x00, 0x00, 0x18, 0x00, 0x00};
static uint8_t reply4001[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply4801[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply3001[]={0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply3333[]={0x21, 0x03, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00 , 0x00 , 0x00
    , 0x00 , 0x00 , 0x00 , 0x00  , 0x00 , 0x00, 0x00 };


int cmd_index =0;
uint32_t cur_time = 0;

// sending bluetooth values every 15ms
void send_task(void* pvParameters) {
    const char* TAG = "send_task";
    ESP_LOGI(TAG, "Sending hid reports on core %d\n", xPortGetCoreID() );
	uint16_t cmd_delay = 16;//ms
	
    while(1)
    {
    	if(start_script){			
			but1_send=0;
			but2_send=0;
			but3_send=0;
			ls1_send=0xF0;
			ls2_send=0x07;
			ls3_send=0x7F;
			rs1_send=0xF0;
			rs2_send=0x07;
			rs3_send=0x7F;
			//ESP_LOGI(TAG, "cmd : %d\n", cmd[i] );
			switch(cmd[cmd_index])
			{
				case 0:
					but1_send  = 0x08;// A
					break;
				case 1:
					but1_send  = 0x04;// B
					break;
				case 2:
					but1_send  = 0x02;// X
					break;
				case 3:
					but1_send  = 0x01;// Y
					break;
				case 4:
					but2_send  = 0x02;// PLUS
					break;
				case 5:
					but3_send  = 0x08;// Left
					break;
				case 6:
					but3_send  = 0x04;// Right
					break;
				case 7:
					but3_send  = 0x01;// Down
					break;
				case 8:
					but3_send  = 0x02;// Up
					break;
				case 9:
					but1_send  = 0x80;// ZR
					break;
				case 10:
					but3_send  = 0x80;// ZL
					break;
				case 11:
					but1_send  = 0x40;// R
					break;
				case 12:
					but1_send  = 0x40;// z+plus
					but2_send  = 0x01;//Minus
					break;
				case 13:
					but1_send  = 0x40;// z+up
					but2_send  = 0x10;//Home
					break;
				case 14:
					ls1_send=rs1_send=0xF0;
					ls2_send=rs2_send=0x07;
					ls3_send=rs2_send=0x7F;
					break;
				case 15:
					ls1_send=0xF0;//UP l
					ls2_send=0x07;
					ls3_send=0xFF;
					break;
				case 16:
					ls1_send=0xF0;//down l
					ls2_send=0x07;
					ls3_send=0x00;
					break;
				case 17:
					ls1_send=0x00;//left l 
					ls2_send=0x00;
					ls3_send=0x7F;
					break;
				case 18:
					ls1_send=0xF0;//right l
					ls2_send=0x0F;
					ls3_send=0x7F;
					break;
				case 19:
					but1_send=0;
					but2_send=0;
					but3_send=0;
					ls1_send=rs1_send=0xF0;
					ls2_send=rs2_send=0x07;
					ls3_send=rs2_send=0x7F;
					break;
				case 20:
					rs1_send=0xF0;//UP r
					rs2_send=0x07;
					rs3_send=0xFF;
					break;
				case 21:
					rs1_send=0xF0;//down r
					rs2_send=0x07;
					rs3_send=0x00;
					break;
				case 22:
					// has some problem
					rs1_send=0x00;//left r 
					rs2_send=0x00;
					rs3_send=0x7F;
					break;
				case 23:
					// has some problem
					rs1_send=0xF0;//right r
					rs2_send=0x0F;
					rs3_send=0x7F;
					break;
				case 24:
					start_script = 1;
					break;
				case 25:
					start_script = 0;
					break;
				case 26:
					break;
				case 27:
					break;
				case 28:
					break;
				case 29:
					break;
				case 30:
					break;
				case 31:
					break;	
				default:
					break;
			}
    	}
	
    	//ESP_LOGI(TAG, "1" );
    	// new command 
    	if(new_send!=0){
			new_send++;
			if(new_send==5){
				new_send=0;
			}
    	}
		else
		{
		// free time
			new_send=0;
			but1_send=0;
			but2_send=0;
			but3_send=0;
			ls1_send=0xF0;
			ls2_send=0x07;
			ls3_send=0x7F;
			rs1_send=0xF0;
			rs2_send=0x07;
			rs3_send=0x7F;
			if(start_script)
			{
				cur_time += cmd_delay;
				if(cur_time>cmd[cmd_index+1]){
					cmd_index+=2;
					cur_time=0;
					new_send=1;
					if(cmd_index==sizeof(cmd)){
						cmd_index=0;
						ESP_LOGE(TAG, "cmd completed");
					}
				}

			}
		}
		send_buttons();
		if(!paired)
			vTaskDelay(100/portTICK_PERIOD_MS);
		else{
			vTaskDelay(cmd_delay / portTICK_PERIOD_MS);

		}
    }
}

// callback for notifying when hidd application is registered or not registered
void application_cb(esp_bd_addr_t bd_addr, esp_hidd_application_state_t state) {
    const char* TAG = "application_cb";

    switch(state) {
        case ESP_HIDD_APP_STATE_NOT_REGISTERED:
            ESP_LOGI(TAG, "app not registered");
            break;
        case ESP_HIDD_APP_STATE_REGISTERED:
            ESP_LOGI(TAG, "app is now registered!");
            if(bd_addr == NULL) {
                ESP_LOGI(TAG, "bd_addr is null...");
                break;
            }
            break;
        default:
            ESP_LOGW(TAG, "unknown app state %i", state);
            break;
    }
}
//LED blink
void startBlink()
{
    while(1) {
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(500 / portTICK_RATE_MS);
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}
// callback for hidd connection changes
void connection_cb(esp_bd_addr_t bd_addr, esp_hidd_connection_state_t state) {
    const char* TAG = "connection_cb";
    
    switch(state) {
        case ESP_HIDD_CONN_STATE_CONNECTED:
            ESP_LOGI(TAG, "connected to %02x:%02x:%02x:%02x:%02x:%02x",
                bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            ESP_LOGI(TAG, "setting bluetooth non connectable");
            esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

            
            //start solid
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
            connected = true;
            xSemaphoreGive(xSemaphore);
            //restart send_task
            if(SendingHandle != NULL)
            {
            	ESP_LOGI(TAG, "delete send_task");
                vTaskDelete(SendingHandle);
                SendingHandle = NULL;
            }
			ESP_LOGI(TAG, "start send_task");
            xTaskCreatePinnedToCore(send_task, "send_task", 2048, NULL, 2, &SendingHandle, 0);
			vTaskSuspend(BlinkHandle);
			gpio_set_level(LED_GPIO, 1);
            break;
        case ESP_HIDD_CONN_STATE_CONNECTING:
            ESP_LOGI(TAG, "connecting");
            break;
        case ESP_HIDD_CONN_STATE_DISCONNECTED:
            //xTaskCreate(startBlink, "blink_task", 1024, NULL, 1, &BlinkHandle);
			vTaskResume(BlinkHandle);
			//start blink
            ESP_LOGI(TAG, "disconnected from %02x:%02x:%02x:%02x:%02x:%02x",
                bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            ESP_LOGI(TAG, "making self discoverable");
            paired = 0;
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
            connected = false;
            xSemaphoreGive(xSemaphore);
            break;
        case ESP_HIDD_CONN_STATE_DISCONNECTING:
            ESP_LOGI(TAG, "disconnecting");
            break;
        default:
            ESP_LOGI(TAG, "unknown connection status");
            break;
    }
}

//callback for discovering
void get_device_cb()
{
    ESP_LOGI("hi", "found a device");
}

// callback for when hid host requests a report
void get_report_cb(uint8_t type, uint8_t id, uint16_t buffer_size) {
    const char* TAG = "get_report_cb";
    ESP_LOGI(TAG, "got a get_report request from host");
}

// callback for when hid host sends a report
void set_report_cb(uint8_t type, uint8_t id, uint16_t len, uint8_t* p_data) {
    const char* TAG = "set_report_cb";
    ESP_LOGI(TAG, "got a report from host");
}

// callback for when hid host requests a protocol change
void set_protocol_cb(uint8_t protocol) {
    const char* TAG = "set_protocol_cb";
    ESP_LOGI(TAG, "got a set_protocol request from host");
}

// callback for when hid host sends interrupt data
void intr_data_cb(uint8_t report_id, uint16_t len, uint8_t* p_data) {
    const char* TAG = "intr_data_cb";
	p_data -= 1;
	len += 1;
	uint8_t tmp = p_data[0];
	p_data[0] = report_id;
	
    //switch pairing sequence
    if(len == 49)
    {
	    if(p_data[10] == 2)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply02[0], sizeof(reply02) - 1, reply02 + 1);
	    }
	    if (p_data[10] == 8)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply08[0], sizeof(reply08) - 1, reply08 + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 0 && p_data[12] == 96)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply1060[0], sizeof(reply1060) - 1, reply1060 + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 80 && p_data[12] == 96)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply1050[0], sizeof(reply1050) - 1, reply1050 + 1);
	    }
	    if (p_data[10] == 3)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply03[0], sizeof(reply03) - 1, reply03 + 1);
	    }
	    if (p_data[10] == 4)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply04[0], sizeof(reply04) - 1, reply04 + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 128 && p_data[12] == 96)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply1080[0], sizeof(reply1080) - 1, reply1080 + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 152 && p_data[12] == 96)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply1098[0], sizeof(reply1098) - 1, reply1098 + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 16 && p_data[12] == 128)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply1010[0], sizeof(reply1010) - 1, reply1010 + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 61 && p_data[12] == 96)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply103D[0], sizeof(reply103D) - 1, reply103D + 1);
	    }
	    if (p_data[10] == 16 && p_data[11] == 32 && p_data[12] == 96)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply1020[0], sizeof(reply1020) - 1, reply1020 + 1);
	    }
	    if (p_data[10] == 64 && p_data[11] == 1)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply4001[0], sizeof(reply4001) - 1, reply4001 + 1);
	    }
	    if (p_data[10] == 72 && p_data[11] == 1)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply4801[0], sizeof(reply4801) - 1, reply4801 + 1);
	    }
	    if (p_data[10] == 48 && p_data[11] == 1)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply3001[0], sizeof(reply3001) - 1, reply3001 + 1);
	    }
	    
	    if (p_data[10] == 33 && p_data[11] == 33)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply3001[0], sizeof(reply3333) - 1, reply3333 + 1);
	        paired = 1;
	        
	    }
	    if (p_data[10] == 64 && p_data[11] == 2)
	    {
		    esp_hid_device_send_report(ESP_HIDD_REPORT_TYPE_INTRDATA, reply4001[0], sizeof(reply4001) - 1, reply4001 + 1);
	    }
        ESP_LOGI(TAG, "got an interrupt report from host, subcommand: %d  %d  %d Length: %d", p_data[10], p_data[11], p_data[12], len);
    }
    else
    {
        ESP_LOGI("heap size:", "%d", xPortGetFreeHeapSize());
        ESP_LOGI(TAG, "pairing packet size != 49, subcommand: %d  %d  %d  Length: %d", p_data[10], p_data[11], p_data[12], len);
    }
    
    
}

// callback for when hid host does a virtual cable unplug
void vc_unplug_cb(void) {
    const char* TAG = "vc_unplug_cb";
    ESP_LOGI(TAG, "host did a virtual cable unplug");
}

void print_bt_address() {
    const char* TAG = "bt_address";
    const uint8_t* bd_addr;

    bd_addr = esp_bt_dev_get_address();
    ESP_LOGI(TAG, "my bluetooth address is %02X:%02X:%02X:%02X:%02X:%02X",
        bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
}

#define SPP_TAG "tag"
static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch(event){
        case ESP_BT_GAP_DISC_RES_EVT:
            ESP_LOGI(SPP_TAG, "ESP_BT_GAP_DISC_RES_EVT");
            esp_log_buffer_hex(SPP_TAG, param->disc_res.bda, ESP_BD_ADDR_LEN);
            break;
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            ESP_LOGI(SPP_TAG, "ESP_BT_GAP_DISC_STATE_CHANGED_EVT");
            break;
        case ESP_BT_GAP_RMT_SRVCS_EVT:
            ESP_LOGI(SPP_TAG, "ESP_BT_GAP_RMT_SRVCS_EVT");
            ESP_LOGI(SPP_TAG, "%d", param->rmt_srvcs.num_uuids);
            break;
        case ESP_BT_GAP_RMT_SRVC_REC_EVT:
            ESP_LOGI(SPP_TAG, "ESP_BT_GAP_RMT_SRVC_REC_EVT");
            break;
        case ESP_BT_GAP_AUTH_CMPL_EVT:{
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(SPP_TAG, "authentication success: %s", param->auth_cmpl.device_name);
                esp_log_buffer_hex(SPP_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
            } else {
                ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        
        default:
            break;
    }
}

#define ECHO_TEST_TXD  (GPIO_NUM_4)
#define ECHO_TEST_RXD  (GPIO_NUM_5)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

static void echo_task()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 2*1024, 0, 0, NULL, 0);


    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 10 / portTICK_RATE_MS);
		//uart_write_bytes(UART_NUM_1, (const char *) data, len);
		for(int i=0;i<len;i++)
		{
			    //but1_send=0;
				//but2_send=0;
				//but3_send=0;
				//ls1_send=rs1_send=0x00;//center
				//ls2_send=rs2_send=0x08;
				//ls3_send=rs2_send=0x80;
			xSemaphoreTake(xSemaphore, portMAX_DELAY);
			switch(data[i])
			{
				case 0:
					but1_send  += 0x08;// A
					break;
				case 1:
					but1_send  += 0x04;// B
					break;
				case 2:
					but1_send  += 0x02;// X
					break;
				case 3:
					but1_send  += 0x01;// Y
					break;
				case 4:
					but2_send  += 0x02;// PLUS
					break;
				case 5:
					but3_send  += 0x08;// Left
					break;
				case 6:
					but3_send  += 0x04;// Right
					break;
				case 7:
					but3_send  += 0x01;// Down
					break;
				case 8:
					but3_send  += 0x02;// Up
					break;
				case 9:
					but1_send  += 0x80;// ZR
					break;
				case 10:
					but3_send  += 0x80;// ZL
					break;
				case 11:
					but1_send  += 0x40;// R
					break;
				case 12:
					but2_send  += 0x01;//Minus
					break;
				case 13:
					but2_send  += 0x10;//Home
					break;
				case 14:
					ls1_send=rs1_send=0x00;//center
					ls2_send=rs2_send=0x08;
					ls3_send=rs2_send=0x80;
					break;
				case 15:
					ls1_send=0x00;//UP l
					ls2_send=0xF8;
					ls3_send=0xFF;
					break;
				case 16:
					ls1_send=0x00;//down l
					ls2_send=0x08;
					ls3_send=0x00;
					break;
				case 17:
					ls1_send=0x00;//left l
					ls2_send=0x00;
					ls3_send=0x80;
					break;
				case 18:
					ls1_send=0xFF;//right l
					ls2_send=0x0F;
					ls3_send=0x80;
					break;
				case 19:
					but1_send=0;
					but2_send=0;
					but3_send=0;
					ls1_send=rs1_send=0x00;
					ls2_send=rs2_send=0x08;
					ls3_send=rs2_send=0x80;
					break;
				case 20:
					rs1_send=0x00;//UP r
					rs2_send=0xF8;
					rs3_send=0xFF;
					break;
				case 21:
					rs1_send=0x00;//down r
					rs2_send=0x08;
					rs3_send=0x00;
					break;
				case 22:
					// has some problem
					rs1_send=0x00;//left r 
					rs2_send=0x00;
					rs3_send=0x80;
					break;
				case 23:
					// has some problem
					rs1_send=0xFF;//right r
					rs2_send=0x0F;
					rs3_send=0x80;
					break;
				case 24:
					start_script = 1;
					cmd_index=0;
					cur_time=0;
					break;
				case 25:
					start_script = 0;
					i=0;
					cur_time=0;
					break;
				case 26:
					but3_send  += 0x40; // L
					break;
				case 27:
					but2_send  += 0x02; // plus
					break;
				case 28:
					but2_send  += 0x04;  // R stick
					break;
				case 29:
					but2_send  += 0x08;  // L stick
					break;
				case 30:
					but2_send  += 0x20;   // Capture
					break;
				case 31:
					break;	
				default:
					break;
			}
			new_send=1;
			xSemaphoreGive(xSemaphore);
			uart_write_bytes(UART_NUM_0, (const char *)(&data[i]), 1);
			ESP_LOGI("heap size:", "%d", xPortGetFreeHeapSize());
		}
	
        // Write data back to the UART
        //uart_write_bytes(UART_NUM_1, (const char *) data, len);
    }
}




void app_main() {
    //GameCube Contoller reading init
    //rmt_tx_init();
    //rmt_rx_init();

	// get button from game cube
    //xTaskCreatePinnedToCore(get_buttons, "gbuttons", 2048, NULL, 1, NULL, 1);

	//flash LED
    vTaskDelay(100);
    gpio_set_level(LED_GPIO, 0);
    vTaskDelay(100);
    gpio_set_level(LED_GPIO, 1);
    vTaskDelay(100);
    gpio_set_level(LED_GPIO, 0);
    vTaskDelay(100);
    gpio_set_level(LED_GPIO, 1);
    vTaskDelay(100);
    gpio_set_level(LED_GPIO, 0);
	
    const char* TAG = "app_main";
	esp_err_t ret;
    static esp_hidd_callbacks_t callbacks;
    static esp_hidd_app_param_t app_param;
    static esp_hidd_qos_param_t both_qos;

    xSemaphore = xSemaphoreCreateMutex();

	// set LED gpio
    gpio_config_t io_conf;
	//disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    
	app_param.name = "Wireless Gamepad";
	app_param.description = "Gamepad";
	app_param.provider = "Nintendo";
    app_param.subclass = 0x8;
    app_param.desc_list = hid_descriptor_gamecube;
    app_param.desc_list_len = hid_descriptor_gc_len;
    memset(&both_qos, 0, sizeof(esp_hidd_qos_param_t));

	// all call back
    callbacks.application_state_cb = application_cb;
    callbacks.connection_state_cb = connection_cb;
    callbacks.get_report_cb = get_report_cb;
    callbacks.set_report_cb = set_report_cb;
    callbacks.set_protocol_cb = set_protocol_cb;
    callbacks.intr_data_cb = intr_data_cb;
    callbacks.vc_unplug_cb = vc_unplug_cb;

	// flash use as e2prom
	ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// erase and then retry it
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    //esp_bt_mem_release(ESP_BT_MODE_BLE);
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "initialize controller failed: %s\n",  esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(TAG, "enable controller failed: %s\n",  esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "initialize bluedroid failed: %s\n",  esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "enable bluedroid failed: %s\n",  esp_err_to_name(ret));
        return;
    }
    //esp_bt_gap_register_callback(esp_bt_gap_cb);
	
    ESP_LOGI(TAG, "setting hid parameters");
    esp_hid_device_register_app(&app_param, &both_qos, &both_qos);

	ESP_LOGI(TAG, "starting hid device");
	esp_hid_device_init(&callbacks);

    ESP_LOGI(TAG, "setting device name");
    esp_bt_dev_set_device_name("Pro Controller");

    ESP_LOGI(TAG, "setting to connectable, discoverable");
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    //start blinking
    xTaskCreate(startBlink, "blink_task", 1024, NULL, 1, &BlinkHandle);

	// cli task
	xTaskCreate(echo_task, "uart_echo_task", 2048, NULL, 10, NULL);
	print_bt_address();
    
}
