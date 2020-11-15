/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
//#include "driver/gpio.h"
#include "syscallprot.pb-c.h"
#include<sys/time.h>
typedef int (*pf)(void *request, void *response);
int x_gettimeofday(void *request, void *response)
{
    return 0;
}

int x_settimeofday(void *request, void *response)
{
    request = (SysRpc__SettimeofdayRequest*)request;
    response = (SysRpc__SettimeofdayResponse*)response;
    struct timeval tv;
    tv.tv_sec = request->timeval_s->tv_sec;
    tv.tv_usec = request->timeval_s->tv_usec;
    struct timezone tz;
    tz.tz_minuteswest = request->timezone_s->tz_minuteswest;
    tz.tz_dsttime = request->timezone_s->tz_dsttime;
    int status = settimeofday(&tv, NULL);
    return status;
}
static pf xRPC_func[] = {x_settimeofday, x_gettimeofday};
//#define LED_PIN 2
/*static size_t read_buffer(char *out)
*{
*    size_t cur_len = 0;
*    while(out[cur_len]!=0)
*    {
*        cur_len++;
*    }
*    return cur_len;
*}
*/
static const char *TAG = "MQTT_EXAMPLE";


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    //FstMsg msg = FST_MSG__INIT;//initialize protobuf structure
    //ProtobufCBinaryData tx;//a ProtobufCBinaryData structure named tx is initialized, to store bytes variable "value" using unsigned char(uint8_t)
    //void *buffer; //BUffer to store serialized data to be transmitted and received serialized data
    size_t len;  //to store length of serialized data
    //char *msgpass = (char *)(buffer);
    /* The below statement initializes a toSend variable of type SysRpc__XRPCMessage; which is a structure that contains submessages as pointer types.
    *  The submessages:
    *  mes_type of type SysRpc__XRPCMessageType-> specifies the type of message :CHeck proto file for details
    *  setTimeRequest of type SysRpc__SettimeofdayRequest-> struct which contains submessages timeval and timezone.
    *  setTimeResponse of type SysRpc__SettimeofdayResponse-> tells about the status: whether successful or not 
    *  getTimeRequest of type SysRpc__GettimeofdayRequest-> not clearly specified yet
    *  getTimeResponse of type SysRpc__GettimeofdayResponse->  stores the time in timeval format (as specified in sys/time.h), yet to implemented
    */
    SysRpc__XRPCMessage toSend = SYS_RPC__X_RPC_MESSAGE__INIT;
    //SysRpc__XRPCMessageType__GettimeofdayResponse getTimeOfDayResponse = SYS_RPC__X_RPC_MESSAGE_TYPE__GETTIMEOFDAY_RESPONSE__INIT;
    //SysRpc__XRPCMessageType__GettimeofdayResponse__Timeval getTimeOfDayResponseTimeval = SYS_RPC__X_RPC_MESSAGE_TYPE__GETTIMEOFDAY_RESPONSE__TIMEVAL__INIT;
    //SysRpc__XRPCMessageType__GettimeofdayResponse__Timezone getTimeOfDayResponseTimezone = SYS_RPC__X_RPC_MESSAGE_TYPE__GETTIMEOFDAY_RESPONSE__TIMEZONE__INIT;
    //SysRpc__XRPCMessageType__GettimeofdayResponse__GettimeofdayRequestStatus getTimeOfDayRequestStatus = SYS_RPC__X_RPC_MESSAGE_TYPE__GETTIMEOFDAY_RESPONSE__GETTIMEOFDAY_REQUEST_STATUS__INIT;
    //SysRpc__XRPCMessageType__SettimeofdayRequest setTimeOfDayRequest = SYS_RPC__X_RPC_MESSAGE_TYPE__SETTIMEOFDAY_REQUEST__INIT;
    //SysRpc__XRPCMessageType__SettimeofdayRequest__Timeval setTimeOfDayRequestTimeval = SYS_RPC__X_RPC_MESSAGE_TYPE__SETTIMEOFDAY_REQUEST__TIMEVAL__INIT;
    //SysRpc__XRPCMessageType__SettimeofdayRequest__Timezone setTimeOfDayRequestTimezone = SYS_RPC__X_RPC_MESSAGE_TYPE__SETTIMEOFDAY_REQUEST__TIMEZONE__INIT;
    //SysRpc__XRPCMessageType__SettimeofdayResponse setTimeOfDayResponse = SYS_RPC__X_RPC_MESSAGE_TYPE__SETTIMEOFDAY_RESPONSE__INIT;
    void *buffer;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
        /* This event handler instance handles cases where MQTT connection is established. In our case, we would like to subscribe to the topic device_id/xRPC_Request, where device_id = 101
        *  for this program. This is used to handle incoming RPCs gettimeofday() and settimeofday() via the topic 101/xRPC_Request.
        */
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "101/xRPC_Request", 0);
            ESP_LOGI(TAG, "Successfully subscribed to msg_id=%d", msg_id);// Use this? The same will also be printed via MQTT_EVENT_SUBSCRIBED.
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            /*tx.data = (uint8_t *)"temperature";// string is stored array of unsigned chars, planning to include relevant string from stdin in future
            *tx.len = 12; //length of string in tx.data
            *msg.value = tx;
            *msg.data = 32;// integer data
            *len = fst_msg__get_packed_size(&msg);
            *buffer = malloc(len);
            *fst_msg__pack(&msg,buffer);//this is the buffer used to send serialized data. fst_msg__pack() serializes the data and stores it in the buffer.
            *msg_id = esp_mqtt_client_publish(client, "/topic/qos0", buffer, len, 0, 0);
            *ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);*/
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: //PROCESSES INCOMING PUBLISH REQUEST TO TOPIC SUBSCRIBED CURRENTLY. RN subscribed to 101/xRPC_Request, process accordingly.
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //Initialize a pointer of type FstMsg to get received message bytes
            /*FstMsg *recvd;
            *len = event->data_len; //Store the length of mesage. Used for deserializing recvd
            *buffer = malloc(len);//Allocate space for serialized message buffer
            *buffer = event->data; //Store the received message in the buffer
            *recvd = fst_msg__unpack(NULL, len, buffer); //Deserialization of received data
            */
            //printf("DATA=%.*s\r\n", event->data_len, event->data);

            //tx = recvd->value; //Store the byte array in ProtobufCBinaryData struct tx defined earlier, to access byte array (unsigned char array)
/*            printf("%s : %d C \n", tx.data, recvd->data);
*            if(recvd->data == 1)
*            {
*                gpio_set_level(LED_PIN, 1); //turn on LED  when 1
*            }
*            else
*            {
*                gpio_set_level(LED_PIN, 0); //turn off led when 0
*            }*/           
           //fst_msg__free_unpacked(recvd, NULL);// Free up the space allocated for recvd
           //Start of RPC HANDLER service below this, the above code is kept fpr reference if any.
            len = event->data_len;
            buffer = malloc(len);
            buffer = event->data;
            int ret = 0;
            SysRpc__XRPCMessage *recvd = sys_rpc__x_rpc_message__unpack(NULL, len, buffer);//unserialize data
            if(recvd->mes_type->type == SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__request && recvd->mes_type->procedure == SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__gettimeofday)
            {
                toSend.mes_type->type = SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__response; //specify message type as response
                toSend.mes_type->procedure = SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__gettimeofday;// specify procedure carried out as gettimeofday
                
            }
            if(recvd->mes_type->type == SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__request && recvd->mes_type->procedure == SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__settimeofday)
            {
                toSend.mes_type->type = SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__response; //specify message type as response
                toSend.mes_type->procedure = SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__settimeofday;// specify procedure carried out as settimeofday
                ret = (*xRPC_func[1])(recvd->settimerequest, recvd->settimeresponse);// execute x_settimeofday() func as defined above. YET TO BE COMPLETED!!
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}
