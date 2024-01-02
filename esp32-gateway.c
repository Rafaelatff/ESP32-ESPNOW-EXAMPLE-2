#include <stdio.h>
#include <string.h> // to use memcpy functions
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/Task.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h" // to use  esp_wifi_get_mac function
#include "esp_mac.h" // to use esp_base_mac_addr_set
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/uart.h"


#define TAG "my_tag"
#define ESP_CHANNEL 1
#define BLUE_LED GPIO_NUM_2

// 48:e7:29:ca:f9:78 - Green - GATEWAY
static uint8_t Blue_SN1 [ESP_NOW_ETH_ALEN] = {0xa0, 0xb7, 0x65, 0x63, 0x96, 0x04};
static uint8_t Pink_SN2 [ESP_NOW_ETH_ALEN] = {0xcc, 0xdb, 0xa7, 0x69, 0xb7, 0xe4};
static uint8_t Orange_SN3 [ESP_NOW_ETH_ALEN] = {0x48, 0xe7, 0x29, 0xc8, 0xfb, 0xe8};
static uint8_t Yellow_SN4 [ESP_NOW_ETH_ALEN] = {0x48, 0xe7, 0x29, 0xc9, 0x3b, 0x20};
static uint8_t Broadcast [ESP_NOW_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define TXD_PIN  (GPIO_NUM_1)
#define RXD_PIN  (GPIO_NUM_3)
#define BUF_SIZE (1024)

static esp_err_t init_wifi(void){
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT(); //  // usa macro default

    esp_netif_init();
    esp_event_loop_create_default();
    nvs_flash_init();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_start();

    ESP_LOGI(TAG, "WiFi initialization completed.");
    return ESP_OK;
}

void recv_cb(const esp_now_recv_info_t * esp_now_info, const uint8_t *data, int data_len){
    ESP_LOGI(TAG, "Data received: " MACSTR "%s", MAC2STR(esp_now_info->src_addr), data);


    
}

void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status){
    if(status == ESP_NOW_SEND_SUCCESS){
        ESP_LOGI(TAG, "ESP_NOW_SEND_SUCCESS");
    }
    else{
        ESP_LOGI(TAG, "ESP_NOW_SEND_FAIL");
    }
}

static esp_err_t init_esp_now(void){
    esp_now_init();
    esp_now_register_recv_cb(recv_cb);
    esp_now_register_send_cb(send_cb);

    ESP_LOGI(TAG, "ESP NOW initialization completed.");
    return ESP_OK;
}

static esp_err_t register_peer(uint8_t *peer_addr){
    esp_now_peer_info_t esp_now_peer_info = {};
    memcpy(esp_now_peer_info.peer_addr, peer_addr, ESP_NOW_ETH_ALEN);
    esp_now_peer_info.channel = ESP_CHANNEL;
    esp_now_peer_info.ifidx = ESP_IF_WIFI_STA;

    esp_now_add_peer(&esp_now_peer_info);
    return ESP_OK;
}

static esp_err_t esp_now_send_data(const uint8_t *peer_addr, const uint8_t *data, uint8_t len){
    esp_now_send(peer_addr, data, len);
    return ESP_OK;
}

void uart_config(){
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // esp_err_t uart_driver_install(uart_port_t uart_num, int rx_buffer_size, int tx_buffer_size, int queue_size, QueueHandle_t *uart_queue, int intr_alloc_flags)
    //uart_driver_install(UART_NUM_1, 53, 53, 0, NULL, 0); // before (UART_NUM_1, 256, 0, 0, NULL, 0)
    
    // Instanciar a fila de recebimento de dados
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);

}

void app_main(void)
{
    gpio_set_direction(GPIO_NUM_2,GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2,1); // To test - GPIO2 is connected to the anode of blue led.
    printf("Iniciando o bagulho. \n\n");

    ESP_ERROR_CHECK(init_wifi());
    ESP_ERROR_CHECK(init_esp_now());
    ESP_ERROR_CHECK(register_peer(Blue_SN1));
    /*ESP_ERROR_CHECK(register_peer(Pink_SN2));
    ESP_ERROR_CHECK(register_peer(Orange_SN3));
    ESP_ERROR_CHECK(register_peer(Yellow_SN4));
    ESP_ERROR_CHECK(register_peer(Broadcast));*/

    uint8_t data_LED_ON[] = "1";
    uint8_t data_LED_OFF[] = "0";

    ESP_LOGI(TAG, "preparing to enter while loop");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //uart_config(); //bugando
    vTaskDelay(100 / portTICK_PERIOD_MS);

    esp_log_level_set("turning off logs", ESP_LOG_NONE); // nao funciona
    ESP_LOGI(TAG, "this message cannot appear");
    while(1){

        uint8_t data[52];
        /*int len = uart_read_bytes(UART_NUM_1, data, sizeof(data), 52 / portTICK_PERIOD_MS);
        if (len > 0) {
            printf("Recebido: %.*s\n", len, data);

            //uart_write_bytes(UART_NUM_1, (char*) data[0], strlen((char*) data));
        }*/

        vTaskDelay(100 / portTICK_PERIOD_MS);
        // Bytes a serem enviados
        const char* dados = "0122333444455555666666777777788888888999999999012233"; //52 bytes
        printf("%s\n", dados);

        
        wifi_config_t wifi_config;
        esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
        ESP_LOGI(TAG, "SSID: %s", (char *)wifi_config.sta.ssid);

        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);
        ESP_LOGI(TAG, "RSSI: %d dBm", ap_info.rssi);
        

        //uart_write_bytes(UART_NUM_1, (char*) data[0], strlen((char*) data));
        
        //uint8_t value = atoi((char *) data);
        /*if(value == 1){
            gpio_set_level(BLUE_LED,1);
        }
        else{
            gpio_set_level(BLUE_LED,0);
        }*/

        // Funcionando! 
        
        gpio_set_level(BLUE_LED,1);
        esp_now_send_data(Blue_SN1, data_LED_ON, 32);
        /*esp_now_send_data(Pink_SN2, data_LED_ON, 32);
        esp_now_send_data(Orange_SN3, data_LED_ON, 32);
        esp_now_send_data(Yellow_SN4, data_LED_ON, 32); */
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec
        gpio_set_level(BLUE_LED,0);
        esp_now_send_data(Broadcast, data_LED_OFF, 32);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec  
             
    }

}
