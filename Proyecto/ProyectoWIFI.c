//Librerias
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_http_server.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>
#include "esp_eth.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "funciones.h"


//Valores configurables
#define EXAMPLE_ESP_WIFI_SSID      "Daniel"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_MAX_STA_CONN       10

static const char *TAG = "softAP_WebServer";

//Mas constantes
#define gpioLED 2

//Variables globales
uint8_t temperatura;
uint8_t humedad;





/*
Esta es la funcion encargada de hacer la peticion de un valor, originalmente estaba hecha especificamente para la funcion "Hello", 
pero la adapte para el resto de opciones.
*/
/* An HTTP GET handler */
static esp_err_t get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");



//=============================================================Este fue mi añadido a esta función==========================================================//

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx; // <--- Esta funcion recibe el mensaje escrito en la funcion "httpd_uri_t".
    const char* resp_uri = (const char*) req->uri;      // <--- Esta funcion recibe la uri escrita en la funcion "httpd_uri_t".


    char string[100];
    char cadena[1000];

    //Se toma una decisión con el valor obtenido.




    if (strcmp(resp_uri, "/mediciones") == 0)
    {

        //======================================Construccion de la pagina============================
        
        //Comprobacion del valor de temperatura
        myItoa(temperatura, string, 10);
        ESP_LOGI("TAG", "\n\tTemperatura: %i\n", temperatura);



        //Cabecera
        strcpy(cadena, "");
        strcat(cadena,         
        "<!DOCTYPE html>"
             "<html>"
                "<head>"
                    "<meta http-equiv=\"refresh\" content=\"5\" name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>"
                "</head>"
                "<body>");


        //Cuerpo
        strcpy(string, "<div>Valores de temperatura detectados:</div>");
        strcat(cadena, string);

        strcpy(string, "<div>&nbsp;</div>");
        strcat(cadena, string);

        strcpy(string, "<div>Temperatura: ");
        strcat(cadena, string);
        myItoa(temperatura, string, 10);
        strcat(cadena, string);
        strcpy(string, " C</div>");
        strcat(cadena, string);

        strcpy(string, "<div>&nbsp;</div>");
        strcat(cadena, string);

        strcpy(string, "<div>Niveles de temperatura esperables.</div>");
        strcat(cadena, string);

        strcpy(string, "<div>Minima: 8 C</div>");
        strcat(cadena, string);
        strcpy(string, "<div>Optima: 16 C</div>");
        strcat(cadena, string);
        strcpy(string, "<div>Maxima: 30 C</div>");
        strcat(cadena, string);

        strcpy(string, "<div>&nbsp;</div>");
        strcat(cadena, string);
        strcpy(string, "<div>&nbsp;</div>");
        strcat(cadena, string);


        //Comprobacion del valor de humedad
        myItoa(humedad, string, 10);
        ESP_LOGI("TAG", "\n\tHumedad: %s\n", string);


        //Construccion de la pagina
        strcpy(string, "<div>Valores de humedad detectados:</div>");
        strcat(cadena, string);

        strcpy(string, "<div>&nbsp;</div>");
        strcat(cadena, string);

        strcpy(string, "<div>Humedad: ");
        strcat(cadena, string);
        myItoa(humedad, string, 10);
        strcat(cadena, string);
        strcpy(string, "%</div>");
        strcat(cadena, string);

        strcpy(string, "<div>&nbsp;</div>");
        strcat(cadena, string);

        strcpy(string, "<div>Niveles de humedad esperables.</div>");
        strcat(cadena, string);

        strcpy(string, "<div>Durante el crecimiento: 80%</div>");
        strcat(cadena, string);
        strcpy(string, "<div>Durante la ultima fase de crecimiento: 60%</div>");
        strcat(cadena, string);
        strcpy(string, "<div>Durante la fase de floracion: 50%</div>");
        strcat(cadena, string);



        //Final de la pagina
        strcat(cadena,         
                "<script>"
                "</script>"
            "</body>"
        "</html>");

    }




    //Se guarda todo y se imprime en la pagina.
    httpd_resp_send(req, cadena, HTTPD_RESP_USE_STRLEN); //<--- Esta funcion se encarga de imprimir el texto guardado en una cadena en la pagina. Puede ser incluso HTML.


//========================================================================================================================================================//

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}






















/*
Estas son las funciones que tendra la pagina web, añadi todas las necesarias basandome en el ejemplo de la funcion "hello" que venia en el archivo.
*/

static const httpd_uri_t mostrarMediciones = {
    .uri       = "/mediciones",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "Mediciones: "
};




/*
Esta funcion es un "get_handler" especial el cual solo se encarga de imprimir un bloque de HTML, en este caso 
la información del proyecto.
*/

static esp_err_t mostrarInformacion_get_handler(httpd_req_t *req)
{

    //ESP_LOGI("TAG", "\n\tMostrar información: %s\n", string);

    httpd_resp_send_chunk(req,
        "<!DOCTYPE html>"
            "<html>"
                "<head>"
                    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>"
                "</head>"

                "<body>"

                "<label>Informacion del proyecto. </label>"

                "<div>&nbsp;</div>"

                "<div>"
                    "<label>Nombre del proyecto: Sistema de monitorizacion para plantas.</label>"
                "</div>"

                "<div>&nbsp;</div>"

                "<div>"
                    "<label>Nombre de los integrantes:</label>"
                    "<div><label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Urias Vega Juan Daniel</label></div>"
                    "<div><label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Cristina</label></div>"
                    "<div><label>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Eloy</label></div>"
                    "</div>"
                "</div>"

                "<div>&nbsp;</div>"

                "<div>"
                    "<label>Nombre de la asignatura: Sistemas embebidos.</label>"
                "</div>"

                "<div>&nbsp;</div>"

                "<div>"
                    "<label>Nombre del docente: Lara Camacho Evangelina.</label>"
                "</div>"

            "</body>"
        "</html>",
        -1);

    return ESP_OK;

}

static const httpd_uri_t mostrarInformacion = {
    .uri       = "/informacion",
    .method    = HTTP_GET,
    .handler   = mostrarInformacion_get_handler,
    .user_ctx  = NULL
};


/*
Esta funcion es un "get_handler" especial el cual solo se encarga de imprimir un bloque de HTML, en este caso el guardado en la
parte de arriba.
*/
static esp_err_t paginaPrincipal_get_handler(httpd_req_t *req)
{
    httpd_resp_send_chunk(req,
            "<!DOCTYPE html>"
            "<html>"
                "<head>"
                    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>"
                "</head>"

                "<body>"

                "<label>Proyecto: Monitorizacion de una planta. </label>"

                "<div>&nbsp;</div>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"informacion\")'>Informacion</button>"
                    "<label>Informacion sobre el proyecto.</label>"
                "</div>"

                "<div>&nbsp;</div>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"mediciones\")'>Mediciones</button>"
                    "<label>Informacion sobre los niveles de temperatura y humedad.</label>"
                "</div>"

                "<div>&nbsp;</div>"



                    "<script>"
                            "function nuevaDireccion(string)"
                            "{"
                                    "window.open('./'+string)"

                            "}"
                    "</script>"
            "</body>"
        "</html>",
    -1);
    return ESP_OK;
}

/*
Este es el objeto "httpd_uri_t" llamado "paginaPrincipal", al llamarlo, este llama al get_handler indicado el cual imprime un bloque HTML.
El proposito de esto es tener un pagina principal al cual accesaremos mediante "192.168.4.1/paginaPrincipal"
*/
httpd_uri_t paginaPrincipal = {             
    .uri = "/paginaPrincipal",
    .method = HTTP_GET,
    .handler = paginaPrincipal_get_handler,
    .user_ctx = NULL
};

















/*
Esta funcion inicializa el servidor, los unicos cambios necesarios fueron la implementacion de los "httpd_register_uri_handler" que serian algo asi como
funciones objeto del servidor.
*/
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Iniciar el servidor httpd 
    ESP_LOGI(TAG, "Iniciando el servidor en el puerto: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {

        // Manejadores de URI
        //ESP_LOGI(TAG, "Registrando manejadores de URI");
        //httpd_register_uri_handler(server, &hello);


        //Yo añadi estas nuevas funciones
        //====================================================================
        httpd_register_uri_handler(server, &mostrarInformacion);
        httpd_register_uri_handler(server, &mostrarMediciones);
        httpd_register_uri_handler(server, &paginaPrincipal);
        //===================================================================


        return server;

    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}





/*
Esta función no fue necesaria modificarla, se encarga simplemente de mostrar que dispositivos se conectan o desconectan en la terminal.
No fue necesaria moficarla.
*/
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "estacion "MACSTR" se unio, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "estacion "MACSTR" se desconecto, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}





/*
Esta función se encarga de inicializar el protocolo softap, el cual genera una red con nombre y contraseña ya establecidas por el usuario.
No fue necesaria cambiarla.
*/
esp_err_t wifi_init_softap(void)
{
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Inicializacion de softAP terminada. SSID: %s password: %s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    return ESP_OK;
}





/*
La funcion main, simplemente se utiliza para inicializar todas las funciones establecidas, no fue necesaria cambiarla.
*/
void app_main(void)
{


    //Inicializacion de UARTs
    uartInit(PC_UART_PORT, UARTS_BAUD_RATE, 8, 0, 1, PC_UART_TX_PIN, PC_UART_RX_PIN);
    uartInit(UART2_PORT, UARTS_BAUD_RATE, 8, 0, 1, UART2_TX_PIN, UART2_RX_PIN);
    uartInit(UART1_PORT, UARTS_BAUD_RATE, 8, 0, 1, UART1_TX_PIN, UART1_RX_PIN);

    //Variables para UART
    uint8_t *received_data = (uint8_t *) malloc(READ_BUF_SIZE);
    char string[100];
    UART_Package pkg;


    httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "init softAP");
    ESP_ERROR_CHECK(wifi_init_softap());

    server = start_webserver();


    while(1){ 

        ESP_LOGI(TAG, "====================================");

        //Sensor DHT
        int len = uart_read_bytes(2, received_data, (READ_BUF_SIZE - 1), 20 / portTICK_RATE_MS);
        if (len) {

            //Se recibe y decodifica el paquete
            pkg = uartStruct_decode((char *)received_data);
            temperatura = pkg.command;
            ESP_LOGI(TAG, "Valor obtenido en temperatura: %i", temperatura);
        }
        delayMs(1000); 


        //Sensor BME280
        len = uart_read_bytes(1, received_data, (READ_BUF_SIZE - 1), 20 / portTICK_RATE_MS);
        if (len) {
            //Se recibe y decodifica el paquete
            pkg = uartStruct_decode((char *)received_data);
            humedad = pkg.command;
            ESP_LOGI(TAG, "Valor obtenido en humedad: %i", humedad);
        }


        printf("\n\n");
        delayMs(1000); 
    }

    
    
}


