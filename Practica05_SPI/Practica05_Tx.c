// Slave as a transmitter for SPI communitation

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "funciones.h"

// Pins in use
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

// Main application
void app_main(void)
{

    // Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num=GPIO_MOSI,
        .miso_io_num=GPIO_MISO,
        .sclk_io_num=GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
        .mode=0,
        .spics_io_num=GPIO_CS,
        .queue_size=3,
        .flags=0,
    };

    // Initialize SPI slave interface
    spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);


    // SPI variables 
    char cadena[128] = {0};
    char valorRecibido[129] = "";
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));

    //Duda
    //spi_device_handle_t handle;

    //Inicialización de los UART
    uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);

    //Limpieza de buffers
    uart_flush(0); 
    uart_flush(2);


    //------------------------------------------------------------------------------------------------


    //Inicialización de los UART
    uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);

    char valorRecibido[129] = "";
    char cadena[129] = "";
    memset(valorRecibido, 0, 33);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));


    printf("Slave input:\n");
    while (1)
    {

        //Aqui se recibe el valor
        t.length = 128 * 8;
        t.rx_buffer = valorRecibido;
        spi_device_transmit(handle, &t);

        //printf("Received: %s\n", recvbuf);


        //Decisión
            uint8_t comand = strtol((char *)valorRecibido , NULL , 0);

            if(comand == 0x10) itoa(xTaskGetTickCount(), cadena,10);
            else if(comand == 0x11) itoa((GPIO_OUT_REG >> gpioLED) & 1, cadena, 10);
            else if(comand == 0x12) itoa(rand()%40, cadena, 10);
            else if(comand == 0x13){
                GPIO_OUT_REG ^= (1 << gpioLED);
                strcpy(cadena, "Led alternado.");
            }

            uartPuts(2, cadena);

        //Enviar dato
        t.length = sizeof(cadena) * 8;
        t.tx_buffer = cadena;
        spi_slave_transmit(handle, &t);

        delayMs(2);
    }
}





