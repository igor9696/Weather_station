/*
 * ESP01.c
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */


#include "ESP01.h"


static ESP_status ESP8266_Check_OK_Respond(ESP8266_t* ESP)
{
	if(ESP->ESP_RX_Buff.BUFFER_EMPTY_FLAG)
	{
		return ESP_NOK;
	}

	Parser_clean_string(&ESP->ESP_RX_Buff, ESP->MessageReceive);

	if(!(Parser_simple_parse("OK", ESP->MessageReceive)))
	{
		return ESP_NOK;
	}

	return ESP_OK;
}


static ESP_status ESP8266_CheckAT(ESP8266_t* ESP)
{
	UART_send_string("AT\r\n");
	HAL_Delay(ESP_RESPOND_TIME);

	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		return ESP_NOK;
	}

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}


static ESP_status ESP8266_Connect_To_Router(ESP8266_t* ESP)
{
	uint8_t message[36];
	uint8_t length;
	length = sprintf((char*)message, "AT+CWJAP=\"%s\",\"%s\"\r\n", ESP->SSID, ESP->PSWD);
	UART_send_message((char*)message, length);

	HAL_Delay(20000);

	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		return ESP_NOK;
	}

	return ESP_OK;
}

ESP_status ESP8266_SetMode(ESP8266_t* ESP, ESP_mode mode)
{
	switch(mode)
	{
	case STATION:
		UART_send_string("AT+CWMODE=1\r\n"); // Set WiFi mode to station mode

	case ACCESS_POINT:
		UART_send_string("AT+CWMODE=2\r\n"); // Set WiFi mode to access point
		break;

	case AP_STATION:
		UART_send_string("AT+CWMODE=3\r\n"); // Set WiFi mode to station mode + AP mode
		break;
	}

	HAL_Delay(ESP_RESPOND_TIME);

	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		return ESP_NOK;
	}

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}


ESP_status ESP8266_Init(ESP8266_t* ESP, char* SSID, char* PSWD, ESP_mode Mode)
{
	ESP->SSID = SSID;
	ESP->PSWD = PSWD;
	RingBuffer_Init(&ESP->ESP_RX_Buff);
	RingBuffer_Init(&ESP->ESP_TX_Buff);

	// TCP client connection config:
	// 0. Check AT
	if(ESP8266_CheckAT(ESP) != ESP_OK)
	{
		return ESP_NOK;
	}
	// 1. Set WiFi mode
	if(ESP8266_SetMode(ESP, Mode) != ESP_OK)
	{
		return ESP_NOK;
	}
	// 2. Connect to a router
	ESP8266_Connect_To_Router(ESP);
	// 3. Connect to a server as a TCP client <- w innej funkcji


	return ESP_OK;
}
