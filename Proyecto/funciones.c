#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "stdlib.h"
#include "string.h"
#include "funciones.h"


//================================================================ UART ========================================================================//

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





//==================================================================== SEND ========================================================//

void uartPutchar(uart_port_t uart_num, char c)
{
    uart_write_bytes(uart_num, &c, sizeof(c));
}

void uartPuts(uart_port_t uart_num, char *str){
    
    while(*str) uartPutchar(uart_num, *str++);

}




//=================================================================== RECEIVE =======================================================//


bool uartKbhit(uart_port_t uart_num)
{
    uint8_t length;
    uart_get_buffered_data_len(uart_num, (size_t*)&length);
    return (length > 0);
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



//======================================================================== ESCAPE SEQUENCES ====================================================//


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



//=================================================================== Utils ================================================================//

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

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}






//============================================================== Protocolo CRC32 ===============================================================//


void array_append(char *arr, int size, char value){
    int new_size = size + 1;
    
    arr = (char*) realloc(arr, new_size * (int)sizeof(char));
    arr[new_size - 1] = value;
}


uint32_t crc32b(char *message) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i] != 0xB2) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}


UART_Package createPackage(uint8_t header, uint8_t command, uint8_t length, uint8_t *data, uint8_t end){
    UART_Package pkg;
    
    pkg = (UART_Package) {
        .header = header, 
        .command = command, 
        .length = length,
        .end = end,
        .crc32 = 0
    };

    if(length > 0){
        pkg.data = (uint8_t*) malloc(sizeof(uint8_t) * length);
        memcpy(pkg.data, data, length+1);
    }else{
        pkg.data = NULL;
    }

    return pkg;
}


UART_Package uartStruct_decode(char *str){
    UART_Package pkg;
    int i = 0;
    uint8_t position = 0, auxVar = 0;
    uint32_t crc32 = 0;

    pkg.header  = ((uint8_t) str[position++]);
    pkg.command = ((uint8_t) str[position++]);
    pkg.length = (((uint8_t) str[position++]));

    if(pkg.length > 0){
        pkg.data = (uint8_t*) malloc(sizeof(uint8_t) * pkg.length + 1); 
        for(; i < pkg.length; i++){
            pkg.data[i] = str[position++];
        }
        pkg.data[i] = '\0';
    }else{
        pkg.data = NULL;
    }

    pkg.end = ((uint8_t) str[position]);
    
    for(int i = 0; i < 3; i++){
        auxVar = ((uint8_t) str[position++]);
        crc32 |= auxVar;
        crc32 <<= 8;
    }
    crc32 |= ((uint8_t) str[position]);
    
    pkg.crc32 = 
    ((crc32 >> 24) & 0xFF) | 
    ((crc32 << 8) & 0xFF0000) | 
    ((crc32 >> 8) & 0xFF00) | 
    ((crc32 << 24) & 0xFF000000);

    /**
     * Aqui se podria agregar una funcion para ver si el crc32 que viene es el mismo que se calcula con los received_data
    **/

    return pkg;
}


void uartStruct_encode(char *str, UART_Package pkg){
    int size = 0;
    uint32_t crc32 = 0;

    array_append(str, size++, (char) pkg.header);
    array_append(str, size++, (char) pkg.command);

    array_append(str, size++, (char) (pkg.length == 0x0 ? -0x1 : pkg.length));

    for(int i = 0; i < pkg.length; i++){
        array_append(str, size++, (char)pkg.data[i]);
    }

    array_append(str, size++, (char) pkg.end);

    pkg.crc32 = crc32b(str);

    crc32 = pkg.crc32;

    for(int i = 0 ; i < 4; i++){
        array_append(str, size++, (char) crc32);
        crc32 >>= 8;
    }
}


void printStruct(UART_Package pkg){
    uartPuts(0, "\n");
    ESP_LOGI("Header", "0x%01X\n", pkg.header);
    ESP_LOGI("Command", "0x%01X\n", pkg.command);
    ESP_LOGI("Length", "0x%01X\n", pkg.length);
    if(pkg.length > 0)
        ESP_LOGI("*data", "%s\n", pkg.data); 
    else
        ESP_LOGI("*data", "NULL\n"); 
    ESP_LOGI("End", "0x%01X\n", pkg.end);
    ESP_LOGI("CRC32", "0x%08X\n", pkg.crc32);
}