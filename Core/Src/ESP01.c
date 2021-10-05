/*
 * ESP01.c
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */


#include "ESP01.h"
#include "usart.h"

extern const char API_Key[];
extern uint8_t UART_RX_val;

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


static ESP_status ESP8266_Disconnect(ESP8266_t* ESP)
{
	uint8_t message[36];
	uint8_t length;
	length = sprintf((char*)message, "AT+CWQAP\r\n");
	UART_send_message((char*)message, length);
	return ESP_OK;
}


static ESP_status ESP8266_EnterDeepSleep(ESP8266_t* ESP, uint32_t sleep_time_ms)
{
	uint8_t message[36];
	uint8_t length;
	length = sprintf((char*)message, "AT+GSLP=%i\r\n", sleep_time_ms);
	UART_send_message((char*)message, length);


	//while(!ESP8266_Check_OK_Respond(ESP));
	return ESP_OK;
}


static ESP_status ESP8266_CheckAT(ESP8266_t* ESP)
{
	UART_send_string("AT\r\n");
	while(RX_RESPOND_FLAG); // wait for receiving message

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
	//while(!ESP8266_Check_OK_Respond(ESP));

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}

ESP_status ESP8266_SetMode(ESP8266_t* ESP, ESP_mode mode)
{
	RX_RESPOND_FLAG = 1;
	HAL_Delay(10000);

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
	//while(!ESP8266_Check_OK_Respond(ESP)); // wait for receiving  OK message


	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}


ESP_status ESP8266_Init(ESP8266_t* ESP, char* SSID, char* PSWD, ESP_mode Mode)
{
	ESP->SSID = SSID;
	ESP->PSWD = PSWD;
	HAL_Delay(15000); // wait sec
	ESP8266_Disconnect(ESP);

	RingBuffer_Init(&ESP->ESP_RX_Buff);
	// TCP client connection config:
	// 0. Check AT
//	if(ESP8266_CheckAT(ESP) != ESP_OK)
//	{
//		return ESP_NOK;
//	}
	// 1. Set WiFi mode
	ESP8266_SetMode(ESP, Mode);
	// 2. Connect to a router
	ESP8266_Connect_To_Router(ESP);

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
	HAL_Delay(5000);

	//while(!ESP8266_Check_OK_Respond(ESP));

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}


ESP_status ESP8266_Connect_TCP(ESP8266_t* ESP, char* Target_IP, char* PORT, ESP_ConnectionMode mode)
{
	ESP8266_SetConnectionMode(ESP, mode);

	uint8_t message[128];
	uint8_t length;
	length = sprintf((char*)message, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", Target_IP, PORT);
	UART_send_message((char*)message, length);
	HAL_Delay(1000);

	//while(!ESP8266_Check_OK_Respond(ESP));

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	ESP->ESP8266_status = ESP_OK;

	return ESP_OK;
}

ESP_status ESP8266_Disconnect_TCP(ESP8266_t* ESP)
{
	UART_send_string("AT+CIPCLOSE\r\n");

	//while(!ESP8266_Check_OK_Respond(ESP));

	RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
	return ESP_OK;
}





ESP_status ESP8266_TS_Send_Data_MultiField(ESP8266_t* ESP, uint8_t number_of_fields, uint16_t data_buffer[])
{
	ESP8266_Connect_TCP(ESP, "184.106.153.149", "80", SINGLE_CONNECTION);

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

	strcat(message, "\r\n");

	// send data length information
	sprintf(cipsend_buff, "AT+CIPSEND=%d\r\n", strlen(message));
	UART_send_message(cipsend_buff, strlen(cipsend_buff));

	HAL_Delay(5 * ESP_RESPOND_TIME);
	//while(!(Parser_simple_parse(">", ESP->MessageReceive)));
	UART_send_message(message, strlen(message)); // send data
	//while(!ESP8266_Check_OK_Respond(ESP));

	// if TCP isn't closed
	if(ESP8266_is_TCP_disconnected(ESP) != ESP_OK)
	{
		ESP8266_Disconnect_TCP(ESP);
	}

	//ESP8266_EnterDeepSleep(ESP, 2000);

	ESP->ESP8266_status = ESP_OK;
	return ESP_OK;
}

