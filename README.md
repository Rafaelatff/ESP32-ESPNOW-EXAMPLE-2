# ESP32-ESPNOW-EXAMPLE-2# ESP32-IZ-POC

I first try to replicate the IZ code using ESP NOW. But sadly it looks that it is not possible to use the ESP NOW protocol and retrieve the RSSI. See [reference](https://www.reddit.com/r/esp32/comments/gzrd87/espnow_rssi/).

I guess I will return a few steps and try again on the common way.

# Board identification

The ESP32 development kits are identificated by its MAC. To aid with the identification, the board will have a colored label with its MAC address and will also be identificated by color.

![image](https://github.com/Rafaelatff/ESP32-IZ-POC/assets/58916022/92d28037-9e6d-4f47-ba7e-cf0b993fe9e1)

* `48:e7:29:ca:f9:78` - Green - GATEWAY
* `a0:b7:65:63:96:04` - Blue - Sensor node 1 
* `cc:db:a7:69:b7:e4` - Pink - Sensor node 2
* `48:e7:29:c8:fb:e8` - Orange - Sensor node 3
* `48:e7:29:c9:3b:20` - Yellow - Sensor node 4

# Code arranjment

The repositorie must hold the following folders, with the following codes:

* Hardware
  - Green board (Gateway)
  - Blue board (Sensor node 1)
  - Pink board (Sensor node 2)
  - Orange board (Sensor node 3)
  - Yellow (Sensor node 4)
* Firmware
  - Python N3 (Gathering RSSI PSR)
  - Python N5 (Parametrization)
  - Python N6 (Exibition)
 
# Hardware - Code explanation

All the ESP32 boards will have the main functions used and already explained on the [ESP NOW Repositorie](https://github.com/Rafaelatff/ESP32-WROOM-32-ESP-NOW/tree/main). Any difference from those code will be explained in this repositorie.

The functions are:
* `static esp_err_t init_wifi(void)` - Function used to do the WiFi Initialization.
* `static esp_err_t init_esp_now(void)` - Function used to do the ESP NOW Initialization.
* `static esp_err_t register_peer(uint8_t *peer_addr)` - Function used to register the Peers in the network.
* `static esp_err_t esp_now_send_data(const uint8_t *peer_addr, const uint8_t *data, uint8_t len)` - Function used to send data over the network.
* `void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)` - Callback function to LOGI the sent data.
* `void recv_cb(const esp_now_recv_info_t * esp_now_info, const uint8_t *data, int data_len)` - Callback function to treat the received information.

I did a small change on the `register_peer(uint8_t *peer_addr)` function. In order to register different peers I: 

```c
static esp_err_t register_peer(uint8_t *peer_addr){
    esp_now_peer_info_t esp_now_peer_info = {};
    memcpy(esp_now_peer_info.peer_addr, peer_addr, ESP_NOW_ETH_ALEN); // Changed second paramter to receive peer_addr instead of peer_mac
    esp_now_peer_info.channel = ESP_CHANNEL;
    esp_now_peer_info.ifidx = ESP_IF_WIFI_STA;

    esp_now_add_peer(&esp_now_peer_info);
    return ESP_OK;
}
```
Then I declared all new peer (ex.: `static uint8_t Pink_SN2 [ESP_NOW_ETH_ALEN] = {0xcc, 0xdb, 0xa7, 0x69, 0xb7, 0xe4};`) and registered. To test, inside the while(1) loop I did the following:

```c
        esp_now_send_data(Blue_SN1, data_LED_ON, 32);
        esp_now_send_data(Pink_SN2, data_LED_ON, 32);
        esp_now_send_data(Orange_SN3, data_LED_ON, 32);
        esp_now_send_data(Yellow_SN4, data_LED_ON, 32);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec
         gpio_set_level(BLUE_LED,0);
        esp_now_send_data(Broadcast, data_LED_OFF, 32); // Also have to declare Broadcast and call ESP_ERROR_CHECK(register_peer(Broadcast));
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec  
```

As results:

![WhatsApp Video 2023-11-20 at 22 26 42](https://github.com/Rafaelatff/ESP32-IZ-POC/assets/58916022/722ee950-5232-45c0-b3e3-43a3794fdaa7)


## Uart connection (test for gateway)

Before going to the gateway firmware, I run a small test. I created a python to send data through the UART (USB) connection and created a firmware to interpretate this data, similar to my first code running the ESP NOW protocol.

```py
import serial
import time

ser = serial.Serial('COM5', 115200, timeout=1)  # Substitua 'COMx' pela porta do seu ESP32

while True:
    dados = input()
    ser.write(dados.encode())
    time.sleep(1)
```
Then I created a firmware that I flashed to the ESP memory:

```c
#include <driver/gpio.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/Task.h"

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
    uart_driver_install(UART_NUM_1, 256, 0, 0, NULL, 0);
}

void app_main(void)
{
    gpio_set_direction(GPIO_NUM_2,GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2,1); // To test - GPIO2 is connected to the anode of blue led.

    uart_config();

    while(1){
        uint8_t data[20];
        int len = uart_read_bytes(UART_NUM_1, data, sizeof(data), 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            printf("Recebido: %.*s\n", len, data);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        uint8_t value = atoi((char *) data);
        if(value == 1){
            gpio_set_level(BLUE_LED,1);
        }
        else{
            gpio_set_level(BLUE_LED,0);
        }
}
```
Then I split my Terminal, so I could have two terminals, one for bulding and flashing the ESP board and another to run my python code.
![image](https://github.com/Rafaelatff/ESP32-IZ-POC/assets/58916022/8225e303-d80b-4cb9-bf39-17704a0bcf4e)

To run my python code, I just entered my main folder by `cd main` and then run `python envia_dados.py`.
![image](https://github.com/Rafaelatff/ESP32-IZ-POC/assets/58916022/299e27c6-f533-4c2c-abfa-b9cafd047cb9)

As results:

![WhatsApp Video 2023-11-15 at 22 24 59](https://github.com/Rafaelatff/ESP32-IZ-POC/assets/58916022/98689282-1f5a-4888-ab81-3d47d8a9e920)

## Uart - 52 characters

The ESP32 sending 52 char:

```c
const char* dados = "0122333444455555666666777777788888888999999999012233"; //52 bytes
printf("%s\n", dados);
```

The python code receing on UART:

```py
try:
    while True:
        # Leia uma linha da porta serial
        Pacote_RX = ser.read(52) # faz a leitura de 52 bytes do buffer que rec
        bytes_em_string = Pacote_RX.decode("utf-8") # ("latin1") nao retornou 1 erro se quer

        if len(Pacote_RX) >= 52:
            print ('>=52')
            print(bytes_em_string)
        else:
            print ('<52')
```

As results:

![WhatsApp Image 2023-12-06 at 22 01 04_90124017](https://github.com/Rafaelatff/ESP32-IZ-POC/assets/58916022/1542e2b0-f7e0-4ba0-b16d-c9c0b96cd5b3)


