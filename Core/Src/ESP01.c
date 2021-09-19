/*
 * ESP01.c
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */


#include "ESP01.h"

extern const char API_Key[];

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


static ESP_status ESP8266_is_TCP_disconnected(ESP8266_t* ESP)
{
	if(ESP->ESP_RX_Buff.BUFFER_EMPTY_FLAG)
	{
		return ESP_NOK;
	}

	Parser_clean_string(&ESP->ESP_RX_Buff, ESP->MessageReceive);

	if(!(Parser_simple_parse("CLOSED", ESP->MessageReceive)))
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

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}

ESP_status ESP8266_SetMode(ESP8266_t* ESP, ESP_mode mode)
{
	switch(mode)
	{
	case STATION:
		UART_send_string("AT+CWMODE=1\r\n"); // Set WiFi mode to station mode
		break;

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
	if(ESP8266_Connect_To_Router(ESP) != ESP_OK)
	{
		return ESP_NOK;
	}

	return ESP_OK;
}

ESP_status ESP8266_SetConnectionMode(ESP8266_t* ESP, ESP_ConnectionMode mode)
{
	switch(mode)
	{
	case SINGLE_CONNECTION:
		UART_send_string("AT+CIPMUX=0\r\n");
		break;
	case MULTIPLE_CONNECTION:
		UART_send_string("AT+CIPMUX=1\r\n");
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


ESP_status ESP8266_Connect_TCP(ESP8266_t* ESP, char* Target_IP, char* PORT, ESP_ConnectionMode mode)
{
	if(ESP8266_SetConnectionMode(ESP, mode) != ESP_OK)
	{
		ESP->ESP8266_status = ESP_NOK;
	}

	uint8_t message[128];
	uint8_t length;
	length = sprintf((char*)message, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", Target_IP, PORT);
	UART_send_message((char*)message, length);

	HAL_Delay(5000);

	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		ESP->ESP8266_status = ESP_NOK;
	}


	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	ESP->ESP8266_status = ESP_OK;

	return ESP_OK;
}

ESP_status ESP8266_Disconnect_TCP(ESP8266_t* ESP)
{
	UART_send_string("AT+CIPCLOSE\r\n");

	HAL_Delay(ESP_RESPOND_TIME);
	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		return ESP_NOK;
	}

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}

ESP_status ESP8266_TS_Send_Data_SingleField(ESP8266_t* ESP, uint8_t field_number, uint16_t data)
{
	if(ESP8266_Connect_TCP(ESP, "184.106.153.149", "80", SINGLE_CONNECTION) != ESP_OK)
	{
		ESP->ESP8266_status = ESP_NOK;
	}

	uint8_t tmp_mess[25];
	uint8_t tmp_length;
	uint8_t message[64];
	uint8_t length;
	length = sprintf((char*)message, "GET /update?api_key=%s&field%d=%d\r\n", API_Key, field_number, data);
	tmp_length = sprintf((char*)tmp_mess, "AT+CIPSEND=%d\r\n", length);

	UART_send_message((char*)tmp_mess, tmp_length);  // send data length
	HAL_Delay(ESP_RESPOND_TIME);

	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		ESP->ESP8266_status = ESP_NOK;
	}

	UART_send_message((char*)message, length); // send data
	HAL_Delay(2000);

	// if TCP isn't closed
	if(ESP8266_is_TCP_disconnected(ESP) != ESP_OK)
	{
		ESP8266_Disconnect_TCP(ESP);
	}

	ESP->ESP8266_status = ESP_OK;
	return ESP_OK;
}


ESP_status ESP8266_TS_Send_Data_MultiField(ESP8266_t* ESP, uint16_t data_buffer[])
{
	for(int i=1; i<4; i++)
	{
		ESP8266_TS_Send_Data_SingleField(ESP, i, data_buffer[i - 1]);
		HAL_Delay(1000);
	}

	return ESP_OK;
}

