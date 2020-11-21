/* MQTT (over TCP) Protocol Buffers Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
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
#include "messagefile.pb-c.h"
#include <sys/time.h>

static const char *TAG = "xRPC_example";

void *buffer; //buffer to store incoming.
/* Function used to get time and store it into response which us a struct of type GettimeofdayResponse*/
int gettimeofday_func(void *clnt , void *request)
{
    printf("Gettime function running. \n");
    struct timeval tv;
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)clnt;
    //Request *request_gettime = (Request*)request;
    XRPCMessage toSend = X_RPCMESSAGE__INIT;
    toSend.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;

    Response resp;
    toSend.response = &resp;
    response__init(toSend.response);

    toSend.response->response_func_case = RESPONSE__RESPONSE_FUNC_GETTIMEOFDAY_RESPONSE;
    GettimeofdayResponse getTimeResponse = GETTIMEOFDAY_RESPONSE__INIT;
    
    int status = gettimeofday(&tv , NULL);
    getTimeResponse.has_return_value = 1;
    getTimeResponse.has_errno_alt = 1;
    getTimeResponse.return_value = status;
    getTimeResponse.errno_alt = errno;
     
    TimeVal tv_get = TIME_VAL__INIT;
    tv_get.has_tv_sec = 1;
    tv_get.has_tv_usec = 1;
    tv_get.tv_sec = tv.tv_sec;
    tv_get.tv_usec = tv.tv_usec;
    getTimeResponse.timeval = &tv_get;
    toSend.response->gettimeofday_response = &getTimeResponse;
    printf("Get time parameters: return_value = %d | errno_alt = %d \n", toSend.response->gettimeofday_response->return_value, toSend.response->gettimeofday_response->errno_alt);
    printf("Get time parameters: tv_sec = %d | tv_usec = %d \n", toSend.response->gettimeofday_response->timeval->tv_sec, 
    toSend.response->gettimeofday_response->timeval->tv_usec);
    
    TimeVal time_st;
    toSend.time_stamp = &time_st;
    time_val__init(toSend.time_stamp);
    toSend.time_stamp->has_tv_sec = 1;
    toSend.time_stamp->has_tv_usec = 1;
    toSend.time_stamp->tv_sec = tv.tv_sec;
    toSend.time_stamp->tv_usec = tv.tv_usec;

    int len = x_rpcmessage__get_packed_size(&toSend);
    uint8_t buffer[len + 1];
    x_rpcmessage__pack(&toSend , (void*)buffer);
    int msg_id_1;
    msg_id_1 = esp_mqtt_client_publish(client,"101/xRPC_Response", (void*)buffer , len , 0, 0);
    ESP_LOGI(TAG, "Successfully published to 101/xRPC_Response, msg_id=%d", msg_id_1);


    return 0;
}
/*Function used to set time. It gets the input from request, struct of type SysRpc__SettimeofdayRequest and store response in struct of type
SysRpc__SettimeofdayResponse */
int settimeofday_func(void *clnt , void *request)
{
    printf("Settime function running. \n");
    struct timeval tv;
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)clnt;
    Request *request_settime = (Request*)request;
    TimeVal *time = request_settime->settimeofday_request->timeval;
    printf("Set time parameters: tv_sec = %d | tv_usec = %d \n", time->tv_sec, time->tv_usec);
    XRPCMessage toSend = X_RPCMESSAGE__INIT;
    toSend.message_type_case = X_RPCMESSAGE__MESSAGE_TYPE_RESPONSE;

    Response resp;
    toSend.response = &resp;
    response__init(toSend.response);

    toSend.response->response_func_case = RESPONSE__RESPONSE_FUNC_SETTIMEOFDAY_RESPONSE;
    SettimeofdayResponse setTimeResponse;
    toSend.response->settimeofday_response = &setTimeResponse;
    settimeofday_response__init(toSend.response->settimeofday_response);

    tv.tv_sec = time->tv_sec;
    tv.tv_usec = time->tv_usec;
    int status = settimeofday(&tv, NULL);
    toSend.response->settimeofday_response->has_return_value = 1;
    toSend.response->settimeofday_response->has_errno_alt = 1;
    toSend.response->settimeofday_response->return_value = status;
    toSend.response->settimeofday_response->errno_alt = errno;
    printf("Return Value: %d | errno_alt: %d \n", toSend.response->settimeofday_response->return_value,toSend.response->settimeofday_response->errno_alt);

    TimeVal time_st;
    toSend.time_stamp = &time_st;
    time_val__init(toSend.time_stamp);
    toSend.time_stamp->has_tv_sec = 1;
    toSend.time_stamp->has_tv_usec = 1;
    toSend.time_stamp->tv_sec = time->tv_sec;
    toSend.time_stamp->tv_usec = time->tv_usec;
    
    int len = x_rpcmessage__get_packed_size(&toSend);
    uint8_t buffer[len + 1];
    x_rpcmessage__pack(&toSend , (void*)buffer);
    int msg_id_1;
    msg_id_1 = esp_mqtt_client_publish(client,"101/xRPC_Response", (void*)buffer , len , 0, 0);
    ESP_LOGI(TAG, "Successfully published to 101/xRPC_Response, msg_id=%d", msg_id_1);
    return status;
}

int func0(void *clnt , void *request)
{
    printf("Invalid type");
    return 0;
}

typedef int (*pf)(void *clnt , void *request);
static pf xRPC_func[] = {
    func0,
    settimeofday_func, 
    gettimeofday_func};




static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
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
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        /* The below case MQTT_EVENT_DATA handles incoming MQTT EVENTS from a publisher.
        *   We can use this case to handle the incoming MQTT event, so the code to do so should be written down here. 
        */    
        case MQTT_EVENT_DATA: //PROCESSES INCOMING PUBLISH REQUEST TO TOPIC SUBSCRIBED CURRENTLY. RN subscribed to 101/xRPC_Request, process accordingly.
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //Start of RPC HANDLER service below.
            size_t len = event->data_len;
            printf("Size of event: %d \n", len);
            buffer = malloc(len);
            buffer = event->data;
            //unserialize data
            XRPCMessage *recvd = x_rpcmessage__unpack(NULL, len, buffer);
            printf("Message unpacked, length of buffer: %d  \n", len);
            printf("Reached check condition \n");
            if(recvd->message_type_case == X_RPCMESSAGE__MESSAGE_TYPE_REQUEST)
            {         
                printf("TImestamp: tv_sec : %d | tv_usec: %d \n", recvd->time_stamp->tv_sec, recvd->time_stamp->tv_usec);
                xRPC_func[recvd->request->request_func_case]((void*)client , (void*)recvd->request);
            }
            else
            {
                break;
            }
            //The below block will show the currently set time.
            time_t now;
            char strftime_buf[64];
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time is: %s \n", strftime_buf);
            x_rpcmessage__free_unpacked(recvd, NULL);// Destroy the *recvd pointer after usage and free up memory
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
