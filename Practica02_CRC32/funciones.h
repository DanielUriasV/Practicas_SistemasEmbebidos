#ifndef MY_UART_H
#define MY_UART_H

#include "driver/uart.h"


//----------------------------------------------------Estructuras---------------------------------------------------------//

//Struct for create a UART Package
typedef struct UART_Package{
    uint8_t header, command, length, end;
    uint8_t *data;
    uint32_t crc32;
} UART_Package;


//-------------------------------------------------------------Definiciones-------------------------------------------------//

// UART 0
#define PC_UART_PORT    (0)
#define PC_UART_RX_PIN  (3)
#define PC_UART_TX_PIN  (1)

// UART 1
#define UART1_PORT      (1)
#define UART1_RX_PIN    (18)
#define UART1_TX_PIN    (13)

// UART 2
#define UART2_PORT      (2)
#define UART2_RX_PIN    (16)
#define UART2_TX_PIN    (17)

// received_data de inicializacion
#define UARTS_BAUD_RATE         (115200)
#define TASK_STACK_SIZE         (1048)
#define READ_BUF_SIZE           (1024)

// Registros
#define _DEF_REG_32b( addr ) ( *(volatile uint32_t *)( addr ) )
#define GPIO_OUT_REG        _DEF_REG_32b( 0x3FF44004 )
#define GPIO_ENABLE_REG     _DEF_REG_32b( 0x3FF44020 )



//-----------------------------------------------------Prototipos---------------------------------------------------------//

void uartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin);

// Send
void uartPuts(uart_port_t uart_num, char *str);
void uartPutchar(uart_port_t uart_num, char data);

// Receive
bool uartKbhit(uart_port_t uart_num);
char uartGetchar(uart_port_t uart_num );
void uartGets(uart_port_t uart_num, char *str);

// Escape sequences
void uartClrScr( uart_port_t uart_num );
void uartSetColor(uart_port_t uart_num, uint8_t color);
void uartGotoxy(uart_port_t uart_num, uint8_t x, uint8_t y);

// Utils
void myItoa(uint16_t number, char* str, uint8_t base) ;
uint16_t myAtoi(char *str);
void delayMs(uint16_t ms);


// CRC32
UART_Package createPackage(uint8_t header, uint8_t command, uint8_t length, uint8_t *data, uint8_t end);
UART_Package uartStruct_decode(char *str);
void uartStruct_encode(char *str, UART_Package pkg);
void printStruct(UART_Package pkg);

#endif