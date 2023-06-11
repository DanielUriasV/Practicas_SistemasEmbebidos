// Librerias
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "string.h"
#include "funciones.h"


static const char *TAG = "Practica 6";
#define SPI_MASTER_NUM       SPI3_HOST
#define SPI_MASTER_MISO_PIN  19
#define SPI_MASTER_MOSI_PIN  23
#define SPI_MASTER_SCLK_PIN  18
#define SPI_MASTER_CS_PIN    5


// Direcciones del acelerometro MPU-6050
#define WHO_AM_I        0x75
#define OUT_X_H         0x32
#define OUT_X_L         0x33
#define OUT_Y_H         0x34
#define OUT_Y_L         0x35
#define OUT_Z_H         0x36
#define OUT_Z_L         0x37

#define BW_RATE         0x2C
#define POWER_CTL       0x2D
#define DATA_FORMAT     0x31


#define MODO_MEDICION   0x08

#define TIPO_FORMATO    0x10
#define SPI_BIT         0x00
#define FULL_RES        0x01
#define CONFIG_DATAFORMAT  (SPI_BIT << 6 | FULL_RES << 3 |TIPO_FORMATO << 0)

#define RATE            0x07
#define LP_MODE         0x00
#define CONFIG_BWRATE   (LP_MODE << 4 | RATE << 0)

#define LEN             8





// Inicializacion del protocolo SPI
spi_device_handle_t spi;

static esp_err_t spi_master_init(void){
    esp_err_t ret;
    spi_bus_config_t config ={
        .mosi_io_num = SPI_MASTER_MOSI_PIN,
        .miso_io_num = SPI_MASTER_MISO_PIN,
        .sclk_io_num = SPI_MASTER_SCLK_PIN,
        .quadwp_io_num=-1, .quadhd_io_num=-1
    };
    ret = spi_bus_initialize(SPI_MASTER_NUM, &config, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
 
    spi_device_interface_config_t devconfig = {
        .mode = 1,
        .spics_io_num = SPI_MASTER_CS_PIN,
        .queue_size = 1,
        .clock_speed_hz = 1.6,
        .pre_cb = NULL,
        .address_bits = 8
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_MASTER_NUM, &devconfig, &spi));
    return ret;
}





// Funciones que pide la practica
esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data,  size_t len) {
    uint8_t tx_data = ((1<<7) | reg_addr);
    spi_transaction_t t = {
        .length = 8,
        .rx_buffer = data,
        .rxlength = len,
        .addr = tx_data
    };
    esp_err_t ret=spi_device_polling_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
    return ret;
}

esp_err_t device_register_write(uint8_t reg_addr, uint8_t data) {
    uint8_t tx_data = ((1<<7) | reg_addr);
    spi_transaction_t t = {
        .addr = tx_data,
        .tx_buffer = &data,
        .length = 8
    };
    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
    return ret;
}


//Estas funciones las use de prueba, pero creo que no funcionan 
/*
esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, size_t len){
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.addr = reg_addr | 0x80;
    //t.addr = reg_addr;
    t.length = len;
    t.rx_buffer = data;
    esp_err_t ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
    return ret;
}


esp_err_t device_register_write(uint8_t reg_addr, uint8_t data){
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.addr = reg_addr & 0x7F;
    //t.addr = reg_addr;
    t.length = 8;
    t.tx_buffer = &data;
    ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
    return ret;
}
*/






void print_data(char* str, uint8_t addr){
    uint8_t data = 0;
    char cad[40];

    if (device_register_read(addr, &data, LEN) == ESP_OK)
        sprintf(cad,"%s \t= 0x%x",str, data);
    else
        sprintf(cad,"ERROR %s \t= 0x%x",str,data);
    ESP_LOGI(TAG, "%s", cad);

}


void app_main(void)
{

    //Inicializa el protocolo SPI
    ESP_ERROR_CHECK(spi_master_init());
    ESP_LOGI(TAG, "SPI initialized successfully");
    
    //Coloca el acelerometro en modo medicion
    device_register_write(POWER_CTL, MODO_MEDICION);

    //Coloca el acelerometro en Low-Power mode 12.5
    device_register_write(BW_RATE, CONFIG_BWRATE);
    
    //Coloca el tipo de formato en el acelerometro
    device_register_write(DATA_FORMAT, CONFIG_DATAFORMAT);



    while (1){

        printf("\n\n");
        //print_data("WHO_I_AM", WHO_AM_I);
        print_data("OUT_X_H", OUT_X_H);
        print_data("OUT_X_L", OUT_X_L);
        print_data("OUT_Y_H", OUT_Y_H);
        print_data("OUT_Y_L", OUT_Y_L);
        print_data("OUT_Z_H", OUT_Z_H);
        print_data("OUT_Z_L", OUT_Z_L);

        delayMs(3000);
    }

    ESP_ERROR_CHECK(spi_bus_free(SPI_MASTER_NUM));
    ESP_LOGI(TAG, "SPI unitialized successfully");
}
