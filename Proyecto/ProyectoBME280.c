
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "funciones.h"

static const char *TAG = "Proyecto BME280";


#define I2C_MASTER_SCL_IO           22         /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21         /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000


// Pines
#define gpioLED 2


// Direcciones del acelerometro BME280
#define WHO_AM_I        0xD0
#define HUM_MSB         0xFD
#define HUM_LSB         0xFE

#define LEN             8
#define SENSOR_ADDR     0x76

//Registros a configurar
#define CTRL_HUM        0xF2
#define CONFIG_CTRL_HUM 4
#define CTRL_MEAS       0xF4
#define CONFIG_CTRL_MEAS 3

//Variable global
uint8_t valorObtenido = 0;


static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


void delay(uint32_t ms){
    vTaskDelay(ms / portTICK_PERIOD_MS);
}


static esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1), 0);
    i2c_master_write_byte(cmd, reg_addr, 0);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1) | 1, 0);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}


esp_err_t device_register_write(uint8_t reg_addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1), 0);
    i2c_master_write_byte(cmd, reg_addr, 0);
    i2c_master_write_byte(cmd, data, 0);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}



void print_data(char* str, uint8_t addr){
    uint8_t data = 0;
    char test[50];

    if (device_register_read(addr, &data, LEN) == ESP_OK){
        sprintf(test,"%s \t= 0x%x",str, data);
        valorObtenido = data;
    }
    else 
        sprintf(test,"ERROR %s \t= 0x%x",str,data);
    ESP_LOGI(TAG, "%s", test);
}



void app_main(void)
{
    //Inicializacion de I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");


    //Configuracion de registros
    uint8_t data = CONFIG_CTRL_HUM;
    device_register_write(CTRL_HUM, data);
    data = CONFIG_CTRL_MEAS;
    device_register_write(CTRL_MEAS, data);


    //Inicializacion de UART
    uartInit(0,UARTS_BAUD_RATE,8,0,1, PC_UART_TX_PIN, PC_UART_RX_PIN);
    uartInit(UART2_PORT,UARTS_BAUD_RATE,8,0,1, UART2_TX_PIN, UART2_RX_PIN);

    uart_flush(0); 
    uart_flush(2);

    char string[100];
    char *data_to_send;
    UART_Package pkg;
    
    uint8_t msb_humedad = 0;
    uint8_t lsb_humedad = 0;
    uint16_t humedadRaw = 0;
    uint16_t humedad = 0;

  

    while(1) {
        
        //Comprobacion de valores
        ESP_LOGI(TAG, "=====================================");
        print_data("WHO_I_AM", WHO_AM_I);
       
        //Capturamos ambas partes de la humedad
        print_data("HUM_MSB", HUM_MSB);
        msb_humedad = valorObtenido;
        print_data("HUM_LSB", HUM_LSB);
        lsb_humedad = valorObtenido;



        //Capturamos el valor
        humedadRaw = (msb_humedad << 8) | lsb_humedad;
        humedad = msb_humedad - 55;
        
        myItoa(humedad, string, 10);
        ESP_LOGI(TAG, "Valor enviado: %s \n\n", string);



        //Enviamos el valor
        pkg = createPackage(0x5A, (uint8_t)strtol(string, NULL, 0), 0x0, NULL, 0xB2);
        data_to_send = (char*) malloc(sizeof(char));
        uartStruct_encode(data_to_send, pkg);


        //Esto es lo que envia
        uartPuts(2, data_to_send);
        free(data_to_send);


        //Led de advertencia
        if(humedad < 40 || humedad > 90){
            GPIO_ENABLE_REG |=  (1UL << gpioLED);
            GPIO_OUT_REG |=  (1UL << gpioLED);

        }
        else{
            GPIO_ENABLE_REG &=   ~(1UL << gpioLED);
            GPIO_OUT_REG &=   ~(1UL << gpioLED);
        }



        delay(1000);

    }

    delayMs(100);
    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG, "I2C unitialized successfully");
}

