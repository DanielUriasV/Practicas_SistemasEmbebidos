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
#define EXAMPLE_ESP_WIFI_SSID      "softAP_SE"
#define EXAMPLE_ESP_WIFI_PASS      "SE_12345678"
#define EXAMPLE_MAX_STA_CONN       10

static const char *TAG = "softAP_WebServer";





/*
Esta funcion se encarga de imprimir todo un bloque de codigo HTML.
*/
static esp_err_t imprimirHTML(httpd_req_t *req)
{
    /*
    Esta funcion se encarga de guardar en un apuntador todo un bloque de texto que puede ser codigo HTML. Dicho texto se guarda en un buffer durante
    la misma llamada de la funcion. En caso de tener el codigo HTML en un apuntador previamente, es mejor utilizar "httpd_resp_send".
    */ 
    httpd_resp_send_chunk(req,
        "<!DOCTYPE html>"
        "<html>"
            "<head>"
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>"
            "</head>"

            "<body>"

                "<label>Practica 07: Comandos. </label>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"hello\")'>Hola mundo!</button>"
                    "<label>hello</label>"
                "</div>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"0x10\")'>Timestamp</button>"
                    "<label>0x10</label>"
                "</div>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"0x11\")'>Estado de LED</button>"
                    "<label>0x11</label>"
                "</div>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"0x12\")'>Temperatura</button>"
                    "<label>0x12</label>"
                "</div>"

                "<div>"
                    "<button type=\"submit\" onclick='nuevaDireccion(\"0x13\")'>Invertir LED</button>"
                    "<label>0x13</label>"
                "</div>"

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


    char string[20];
    char cadena[40];

    //Se toma una decisión con el valor obtenido.

    if (strcmp(resp_uri, "/0x10") == 0)
    {
        itoa(xTaskGetTickCount(),string,20);
        ESP_LOGI("TAG", "Comando 0x10\n\tTimestamp: %s\n", string);
    }
    
    else if (strcmp(resp_uri, "/0x11") == 0)
    {
        itoa((GPIO_ENABLE_REG >> 2) & 1, string, 20);
        ESP_LOGI("TAG", "Comando 0x11\n\tEstado de LED: %s\n", string);
    }

    else if (strcmp(resp_uri, "/0x12") == 0)
    {
        itoa(rand()%30, string, 20);
        ESP_LOGI("TAG", "Comando 0x12\n\tTemperatura: %s\n", string);
    }
    
    else if (strcmp(resp_uri, "/0x13") == 0)
    {
            if((GPIO_ENABLE_REG >> 2) & 1) 
                GPIO_ENABLE_REG &= ~(1 << 2);
            else 
                GPIO_ENABLE_REG |= 1 << 2;
            strcpy(string, "Led Invertido");
            ESP_LOGI("TAG", "Comando 0x13\n\t: %s\n", string);
    }
    

    //Se guarda el valor y se imprime en la pagina.
    strcat(cadena, resp_str);
    strcat(cadena, string);

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

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "Hola Mundo!!"
};

static const httpd_uri_t mostrarTimestamp = {
    .uri       = "/0x10",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "Timestamp: "
};

static const httpd_uri_t mostrarEstadoLed = {
    .uri       = "/0x11",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "Estado del Led: "
};

static const httpd_uri_t mostrarTemperatura = {
    .uri       = "/0x12",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "Temperatura: "
};

static const httpd_uri_t invertirLed = {
    .uri       = "/0x13",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = "Estado del led invertido."
};

/*
Esta funcion es un "get_handler" especial el cual solo se encarga de imprimir un bloque de HTML, en este caso el guardado en la
parte de arriba.
*/
static esp_err_t paginaPrincipal_get_handler(httpd_req_t *req)
{
    return imprimirHTML(req);
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
        ESP_LOGI(TAG, "Registrando manejadores de URI");
        httpd_register_uri_handler(server, &hello);


        //Yo añadi estas nuevas funciones
        //====================================================================
        httpd_register_uri_handler(server, &mostrarTimestamp);
        httpd_register_uri_handler(server, &mostrarEstadoLed);
        httpd_register_uri_handler(server, &mostrarTemperatura);
        httpd_register_uri_handler(server, &invertirLed);
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
    httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "init softAP");
    ESP_ERROR_CHECK(wifi_init_softap());

    server = start_webserver();
}


