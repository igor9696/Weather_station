/*
 * ESP01.h
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */

#ifndef INC_ESP01_H_
#define INC_ESP01_H_


#include "main.h"
#include "Utilis.h"
#include "RingBuffer.h"
#include "string.h"
#include "parser.h"
#include "rtc_utilis.h"


#define ESP_RESPOND_TIME 		100 // 100ms
#define ESP_MessageSize			128
#define ESP_MAX_RESPOND_TIME	5000
#define ESP_RECONNECTING_TO_WIFI_TIME_INTERVAL		60 // seconds
#define ESP_STOP_TIME_BETWEEN_SENDING_DATA			120 // seconds
uint8_t ESP_RESPOND_FLAG;


//volatile uint8_t RX_RESPOND_FLAG;

typedef enum ESP_status
{
	ESP_NOK,
	ESP_OK,
}ESP_status;

typedef enum ESP_mode
{
	STATION,
	ACCESS_POINT,
	AP_STATION,
}ESP_mode;

typedef enum ESP_ConnectionMode
{
	SINGLE_CONNECTION,
	MULTIPLE_CONNECTION,
}ESP_ConnectionMode;

typedef struct ESP8266_t
{
	char* 			SSID;
	char* 			PSWD;
	RingBuffer_t 	ESP_RX_Buff;
	RingBuffer_t	ESP_RX_msg_to_parsed;
	uint8_t 		MessageReceive[ESP_MessageSize];
	ESP_status		ESP8266_status;
}ESP8266_t;



// function prototypes
ESP_status ESP8266_Init(ESP8266_t* ESP, char* SSID, char* PSWD, ESP_mode Mode);
ESP_status ESP8266_SetMode(ESP8266_t* ESP, ESP_mode mode);
ESP_status ESP8266_Connect_TCP(ESP8266_t* ESP, char* Target_IP, char* PORT, ESP_ConnectionMode mode);
ESP_status ESP8266_TS_Send_Data_MultiField(ESP8266_t* ESP, uint8_t number_of_fields, uint16_t data_buffer[]);
ESP_status ESP8266_Disconnect_TCP(ESP8266_t* ESP);
ESP_status ESP8266_EnterLightSleep(ESP8266_t* ESP, uint8_t GPIO_wakeup, uint8_t gpio_trigg_level);
ESP_status ESP8266_EnterDeepSleep(ESP8266_t* ESP, uint16_t sleep_seconds);

#endif /* INC_ESP01_H_ */
