#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "funciones.h"


// Debug Mode
#define SENDER      (0)
#define RECEIVER    (1)
#define MODE        (1)

// Perifericos
#define gpioLED     (2)





//---------------------------------------------Modo RECEIVE-----------------------------------------//

#if MODE == RECEIVER

    void app_main(void)
    {   
        //Inicialización de los UART
        uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);
        uartInit(UART2_PORT, UARTS_BAUD_RATE, 8, 0, 1, UART2_TX_PIN, UART2_RX_PIN);

        //Varables
        uint8_t *datos = (uint8_t *) malloc(READ_BUF_SIZE);
        char string[20];

        //Limpieza de buffers'
        uart_flush(0); 
        uart_flush(2);

        //Inicialización de GPIOs
        GPIO_ENABLE_REG |= (1 << gpioLED);


        while(1){ 

            int len = uart_read_bytes(2, datos, (READ_BUF_SIZE - 1), 20 / portTICK_RATE_MS);
            
            if (len) {
                datos[len] = '\0';
                uint8_t comand = strtol((char *)datos , NULL , 0);

                if(comand == 0x10) itoa(xTaskGetTickCount(),string,10);
                else if(comand == 0x11) itoa((GPIO_OUT_REG >> gpioLED) & 1, string, 10);
                else if(comand == 0x12) itoa(rand()%40, string, 10);
                else if(comand == 0x13){
                    GPIO_OUT_REG ^= (1 << gpioLED);

                    strcpy(string, "");
                }

                uartPuts(2, string);
            }
            delayMs(2); 
        }
    }


//---------------------------------------------Modo SENDER-----------------------------------------//

    #elif MODE == SENDER

    void app_main(void)
    {   
        //Inicialización de los UART
        uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);
        uartInit(UART2_PORT, UARTS_BAUD_RATE, 8, 0, 1, UART2_TX_PIN, UART2_RX_PIN);

        //Varables
        uint8_t *datos = (uint8_t *) malloc(READ_BUF_SIZE);
        char string[20];

        //Limpieza de buffers
        uart_flush(0); 
        uart_flush(2);


    
        while(1){ 
            
            //Menu
            uartClrScr(0);
            uartGotoxy(0,2,2);
            uartPuts(0, "Introduce un comando: ");
            uartGets(0, string);
            

            if( !strcmp(string,"0x10") || !strcmp(string,"0x11") || !strcmp(string,"0x12") || !strcmp(string,"0x13"))
            {
                uartPuts(2, string);
                uartClrScr(0);
                uartGotoxy(0,2,2);
                
                int len = uart_read_bytes(2, datos, READ_BUF_SIZE, 20 / portTICK_RATE_MS);
                if(len){   
                    datos[len] = '\0';
                    
                    if(!strcmp(string,"0x10")) uartPuts(0, "TIMESTAMP: ");
                    else if(!strcmp(string,"0x11")) uartPuts(0, "ESTADO DEL LED: ");
                    else if(!strcmp(string,"0x12")) uartPuts(0, "TEMPERATURA: ");
                    else if(!strcmp(string,"0x13")) uartPuts(0, "INVERTIR LED: ");
                    
                    uartGotoxy(0,4,2);
                    uartPuts(0, (char*) datos);    

                }

                string[0] = '\0'; 
            }
            else{
                uartClrScr(0);
                uartGotoxy(0,2,2);

                strcpy(string, "Error. \n");
                uartPuts(0, string);
            }
            uartGetchar(0);

        }

        delayMs(2); 

    }
    #endif


