#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "funciones.h"


void uartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin)
{
    uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = size-5,
        .parity    = parity,
        .stop_bits = stop,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(uart_num, READ_BUF_SIZE, READ_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

}

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void uartClrScr(uart_port_t uart_num)
{
    // Uso "const" para sugerir que el contenido del arreglo lo coloque en Flash y no en RAM
    const char caClearScr[] = "\e[2J";
    uart_write_bytes(uart_num, caClearScr, sizeof(caClearScr));
}

void uartGoto11(uart_port_t uart_num)
{
    // Limpie un poco el arreglo de caracteres, los siguientes tres son equivalentes:
     // "\e[1;1H" == "\x1B[1;1H" == {27,'[','1',';','1','H'}
    const char caGoto11[] = "\e[1;1H";
    uart_write_bytes(uart_num, caGoto11, sizeof(caGoto11));
}

bool uartKbhit(uart_port_t uart_num)
{
    uint8_t length;
    uart_get_buffered_data_len(uart_num, (size_t*)&length);
    return (length > 0);
}

void uartPutchar(uart_port_t uart_num, char c)
{
    uart_write_bytes(uart_num, &c, sizeof(c));
}

char uartGetchar(uart_port_t uart_num)
{
    char c;
    // Wait for a received byte
    while(!uartKbhit(uart_num))
    {
        delayMs(10);
    }
    // read byte, no wait
    uart_read_bytes(uart_num, &c, sizeof(c), 0);

    return c;
}

void uartGets(uart_port_t uart_num, char *str)
{
    char c;
    int i = 0;

    while((c = uartGetchar(uart_num)) != 13){
        if(c >= 32 && c <= 126){
            *str = c;
            str++;
            i++;
            uartPutchar(uart_num, c);
        }

        if(c == 8){

            if(i-- > 0)
            str--;
            uartPuts(uart_num, "\e[1D");
            uartPuts(uart_num, "\e[0K");
        }
    }

    *str = '\0';
}

void uartPuts(uart_port_t uart_num, char *str){
    
    while(*str) uartPutchar(uart_num, *str++);

}

void myItoa(uint16_t number, char* str, uint8_t base){

    int n = 0, i = 0, length = 0;
    char temp;

    while(number != 0)
    {
        n = n * 10;
        n = number % base;
        str[i] = n >= 10 && n <= 15 ? n + 55 : n + 48;
        number /= base;
        i++;
    }
    str[i] = '\0';
    length = i;
    for(i = 0; i < length/2; i++)
    {
        temp = str[i];
        str[i] = str[length - i - 1];
        str[length - i -1] = temp;
    }

}

uint16_t myAtoi(char *str){

    uint16_t num = 0, i;
    char c;

    if (str[0] == '0' || strcmp(str, "65536"))
        num = 0;
    
    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] > 47 && str[i] < 58)
        {
            num*=10;
            c = str[i];
            c-=48;
            num+=c; 
        } else{
            break;
        }
    }

    return num;

}

void uartSetColor(uart_port_t uart_num, uint8_t color)
{
    char changeC[10];
    sprintf(changeC, "\e[0;%dm", color);
    uartPuts(uart_num, changeC);
}

void uartGotoxy(uart_port_t uart_num, uint8_t x, uint8_t y)
{
    char caGotoxy[11];
    sprintf(caGotoxy, "\e[%d;%dH", x, y);
    uartPuts(uart_num, caGotoxy);
}

