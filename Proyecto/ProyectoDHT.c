
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "dht11.h"
#include "funciones.h"


#define gpioLED 2
#define gpioDHT 4

static const char *TAG = "Proyecto DHT11";

void app_main()
{

    DHT11_init(gpioDHT);
    

    //Inicialización de los UART
    uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);
    uartInit(UART2_PORT, UARTS_BAUD_RATE, 8, 0, 1, UART2_TX_PIN, UART2_RX_PIN);

    //Variables
    char string[100];
    char *data_to_send;
    UART_Package pkg;

    //Limpieza de buffers
    uart_flush(0); 
    uart_flush(2);
    
    while(1){ 
        
        //Valores del sensor
        ESP_LOGI(TAG, "=============================");
        ESP_LOGI(TAG, "Temperatura: %d ", DHT11_read().temperature);
        ESP_LOGI(TAG, "Humedad: %d", DHT11_read().humidity);
        ESP_LOGI(TAG, "Estado: %d", DHT11_read().status);
        myItoa(DHT11_read().temperature, string, 10);


        //Construccion del paquete
        pkg = createPackage(0x5A, (uint8_t)strtol(string, NULL, 0), 0x0, NULL, 0xB2);
        data_to_send = (char*) malloc(sizeof(char));
        uartStruct_encode(data_to_send, pkg);


        //Comprobacion de que enviamos
        
        ESP_LOGI(TAG, "Valor enviado es: %s\n\n\n", string);

        //Esto es lo que envia
        uartPuts(2, data_to_send);
        free(data_to_send);


        //Led de advertencia
        if(DHT11_read().temperature > 30 || DHT11_read().temperature < 8){
            GPIO_ENABLE_REG |=  (1UL << gpioLED);
            GPIO_OUT_REG |=  (1UL << gpioLED);

        }
        else{
            GPIO_ENABLE_REG &=   ~(1UL << gpioLED);
            GPIO_OUT_REG &=   ~(1UL << gpioLED);
        }



        delayMs(1000);

    }
}