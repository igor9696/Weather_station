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





ESP_status ESP8266_TS_Send_Data_MultiField(ESP8266_t* ESP, uint8_t number_of_fields, uint16_t data_buffer[])
{
	if(ESP8266_Connect_TCP(ESP, "184.106.153.149", "80", SINGLE_CONNECTION) != ESP_OK)
	{
		ESP->ESP8266_status = ESP_NOK;
	}

	char cipsend_buff[25] = {0};
	char field_buff[35] = {0};
	char message[128] = {0};

	// prepare message
	sprintf(message, "GET /update?api_key=%s", API_Key);
	for(int i = 1; i < number_of_fields + 1; i++)
	{
		sprintf(field_buff, "&field%d=%u", i, data_buffer[i - 1]);
		strcat(message, field_buff);
	}

//	for(int i=4; i < number_of_fields + 1; i++)
//	{
//		sprintf(field_buff, "&field%d=%g", i, data_buffer[i]);
//		strcat(message, field_buff);
//	}
	strcat(message, "\r\n");

	// send data length information
	sprintf(cipsend_buff, "AT+CIPSEND=%d\r\n", strlen(message));
	UART_send_message(cipsend_buff, strlen(cipsend_buff));
	HAL_Delay(ESP_RESPOND_TIME);

	if(ESP8266_Check_OK_Respond(ESP) != ESP_OK)
	{
		ESP->ESP8266_status = ESP_NOK;
	}

	UART_send_message(message, strlen(message)); // send data
	HAL_Delay(1000);

	// if TCP isn't closed
	if(ESP8266_is_TCP_disconnected(ESP) != ESP_OK)
	{
		ESP8266_Disconnect_TCP(ESP);
	}

	ESP->ESP8266_status = ESP_OK;
	return ESP_OK;
}

