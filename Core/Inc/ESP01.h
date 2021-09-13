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


#define ESP_RESPOND_TIME 		100 // 100ms
#define ESP_MessageSize			64


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

typedef struct ESP8266_t
{
	char* 			SSID;
	char* 			PSWD;
	RingBuffer_t 	ESP_TX_Buff;
	RingBuffer_t 	ESP_RX_Buff;
	uint8_t 		MessageReceive[ESP_MessageSize];
}ESP8266_t;



// function prototypes
ESP_status ESP8266_Init(ESP8266_t* ESP, char* SSID, char* PSWD, ESP_mode Mode);
ESP_status ESP8266_SetMode(ESP8266_t* ESP, ESP_mode mode);




#endif /* INC_ESP01_H_ */
