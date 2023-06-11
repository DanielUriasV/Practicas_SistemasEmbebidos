// Master as a receiver for SPI communication
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_spi_flash.h"

#include "driver/gpio.h"
#include "funciones.h"

// Pins in use
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

// Perifericos
#define gpioLED     (2)



// Main application
void app_main(void)
{
    spi_device_handle_t handle;

    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};

    // Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 5000000,
        .duty_cycle_pos = 128, // 50% duty cycle
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .cs_ena_posttrans = 3, // Keep the CS low 3 cycles after transaction
        .queue_size = 3};

    spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(HSPI_HOST, &devcfg, &handle);


//-------------------------------------------------------------------------------------------------------------------------------


    //Inicialización de los UART
    uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);

    char valorRecibido[129] = "";
    char cadena[129] = "";
    char string[129] = "";
    memset(valorRecibido, 0, 33);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));


    //Limpieza de buffers
    uart_flush(0); 
    uart_flush(2);

    printf("Master transmission:\n");
    while (1)
    {

        //Menu
        uartClrScr(0);
        uartGotoxy(0,2,2);
        uartPuts(0, "Introduce un comando: ");
        uartGets(0, string);

        //Enviar valor
        snprintf(cadena, sizeof(cadena), string);

        //Aqui se recibe el valor
        t.length = 128 * 8;
        t.rx_buffer = valorRecibido;
        spi_device_transmit(handle, &t);

        
        //fgets(cadena, sizeof(cadena), stdin);
        //snprintf(cadena, sizeof(cadena), "Enviado por Slave - %d", i);

        if( !strcmp(cadena,"0x10") || !strcmp(cadena,"0x11") || !strcmp(cadena,"0x12") || !strcmp(cadena,"0x13"))
            {
                uartPuts(2, cadena);
                uartClrScr(0);
                uartGotoxy(0,2,2);
                
                //Aqui se recibe la respuesta
                t.length = 128 * 8;
                t.rx_buffer = valorRecibido;

                //int len = uart_read_bytes(2, datos, READ_BUF_SIZE, 20 / portTICK_RATE_MS);// cambiar
                //int len = valorRecibido.length;

                //if(len){   
                    //datos[len] = '\0';


                    if(!strcmp(cadena,"0x10")) uartPuts(0, "TIMESTAMP: ");
                    else if(!strcmp(cadena,"0x11")) uartPuts(0, "ESTADO DEL LED: ");
                    else if(!strcmp(cadena,"0x12")) uartPuts(0, "TEMPERATURA: ");
                    else if(!strcmp(cadena,"0x13")) uartPuts(0, "INVERTIR LED: ");
                    
                    uartGotoxy(0,4,2);

                    uartPuts(0, (char*) valorRecibido);//cambiar  

                //}

                //string[0] = '\0'; 
            }
            else{
                uartClrScr(0);
                uartGotoxy(0,2,2);

                strcpy(cadena, "Error. \n");
                uartPuts(0, cadena);
            }
        uartGetchar(0);
    }
}



