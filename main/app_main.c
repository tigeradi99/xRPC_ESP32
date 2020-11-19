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
/*struct of type SysRpc__SettimeofdayRequest, which stores set time requests coming form the publisher.
*The struct has the following fields:
* a struct of type timeval_s, which contains:
* tv_sec = type int32 to store sceonds elapsed since 1st Jan, 1970 00:00:00 UTC
* tv_usec = type int32, to store time in microseconds
*/
SysRpc__SettimeofdayRequest setTimeRequest = SYS_RPC__SETTIMEOFDAY_REQUEST__INIT;
/*struct of type SysRpc__SettimeofdayResponse, which stores set time requests coming form the publisher.
*The struct has the following fields:
* return_value = stores value returned by settimeofday() function. If it is zero, this indicates that time has been set succesfully, else -1 is returned.
* errno_alt = stores errno variable, which is set to a specific value whenever error occurs.
*/
SysRpc__SettimeofdayResponse setTimeResponse = SYS_RPC__SETTIMEOFDAY_RESPONSE__INIT;
/*struct of type SysRpc__SettimeofdayRequest, which stores set time requests coming form the publisher.
*The struct has the following fields:
* a struct of type timeval_r, which contains:
* tv_sec = type int32 to store sceonds elapsed since 1st Jan, 1970 00:00:00 UTC
* tv_usec = type int32, to store time in microseconds
*
* a struct of type gettimeofdayRequestStatus, which contains:
*  return_value = stores value returned by gettimeofday() function. If it is zero, this indicates that time has been set succesfully, else -1 is returned.
* errno_alt = stores errno variable, which is set to a specific value whenever error occurs.
*/
SysRpc__GettimeofdayResponse getTimeResponse = SYS_RPC__GETTIMEOFDAY_RESPONSE__INIT;
//struct of type SysRpc__SettimeofdayRequest__Timeval to store value of tv_sec and tv_usec and use both to set time.
SysRpc__SettimeofdayRequest__Timeval ReqTimeSet = SYS_RPC__SETTIMEOFDAY_REQUEST__TIMEVAL__INIT; 
//struct of type SysRpc__GettimeofdayResponse__Timeval to store value of tv_sec and tv_usec obtained from gettimeofday function
SysRpc__GettimeofdayResponse__Timeval RespTimeGet = SYS_RPC__GETTIMEOFDAY_RESPONSE__TIMEVAL__INIT; 
//struct of type SysRpc__GettimeofdayResponse__GettimeofdayRequestStatus to store status variables return_value and errno
SysRpc__GettimeofdayResponse__GettimeofdayRequestStatus getRespStatus = SYS_RPC__GETTIMEOFDAY_RESPONSE__GETTIMEOFDAY_REQUEST_STATUS__INIT; 
/* variable of type SysRpc__XRPCMessage; which is a structure that contains submessages as pointer types.
    *  It is sent to the subscriber with the appropriate values
    *  The submessages:
    *  mes_type of type SysRpc__XRPCMessageType-> specifies the type of message :Check proto file for details
    *  setTimeRequest of type SysRpc__SettimeofdayRequest-> struct which contains submessages timeval.
    *  setTimeResponse of type SysRpc__SettimeofdayResponse-> tells about the status: whether successful or not 
    *  getTimeResponse of type SysRpc__GettimeofdayResponse->  stores the time in timeval format (as specified in sys/time.h)
    */
SysRpc__XRPCMessage toSend = SYS_RPC__X_RPC_MESSAGE__INIT;
SysRpc__XRPCMessageType messageType = SYS_RPC__X_RPC_MESSAGE_TYPE__INIT;
void *buffer; //buffer to store incoming.
uint8_t *buffer2; //buffer to store response.
/* Function used to get time and store it into response which us a struct of type SysRpc__GettimeofdayResponse*/
int x_gettimeofday(void *request, void *response)
{
    struct timeval tv;
    printf("Passing Parameters to gettimeofday. \n");
    int status = gettimeofday(&tv,NULL);//retrieve current time from sys/time.h
    printf("Parameters passed, values obtained from gettimeofday. \n");
    printf("Storing values to response \n"); 
    RespTimeGet.tv_sec = tv.tv_sec;
    RespTimeGet.tv_usec = tv.tv_usec;
    ((SysRpc__GettimeofdayResponse*)response)->timeval_r = &RespTimeGet;
    printf("seconds : %d\nmicro seconds : %d \n",RespTimeGet.tv_sec,RespTimeGet.tv_usec);
    getRespStatus.return_value = status;
    getRespStatus.errno_alt = errno;
    ((SysRpc__GettimeofdayResponse*)response)->status = &getRespStatus;
    printf("Returning value and exiting function gettimeofday. \n");
    return status;
}
/*Function used to set time. It gets the input from request, struct of type SysRpc__SettimeofdayRequest and store response in struct of type
SysRpc__SettimeofdayResponse */
int x_settimeofday(void *request, void *response)
{
    struct timeval tv;
    printf("Passing Parameters to timeval struct. \n");
    ReqTimeSet = *(((SysRpc__SettimeofdayRequest*)request)->timeval_s);
    tv.tv_sec = ReqTimeSet.tv_sec;
    tv.tv_usec = ReqTimeSet.tv_usec;
    printf("tv_sec: %ld \n", tv.tv_sec);
    printf("tv_usec: %ld \n", tv.tv_usec);
    printf("Passing Parameters to settimeofday. \n");
    int status = settimeofday(&tv, NULL); //Obtain current time from settimeofday
    printf("Parameters passed, values obtained from settimeofday. \n");
    printf("Storing values to response \n");
    ((SysRpc__SettimeofdayResponse*)response)->return_value = status;
    ((SysRpc__SettimeofdayResponse*)response)->errno_alt = errno;
    printf("Returning value and exiting function settimeofday. \n");
    return status;
}
static pf xRPC_func[] = {x_gettimeofday, x_settimeofday};
//#define LED_PIN 2
static const char *TAG = "MQTT_EXAMPLE";


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    size_t len;  //to store length of serialized data
    size_t len2;
    /* The below statement initializes a toSend variable of type SysRpc__XRPCMessage; which is a structure that contains submessages as pointer types.
    *  The submessages:
    *  mes_type of type SysRpc__XRPCMessageType-> specifies the type of message :Check proto file for details
    *  setTimeRequest of type SysRpc__SettimeofdayRequest-> struct which contains submessages timeval.
    *  setTimeResponse of type SysRpc__SettimeofdayResponse-> tells about the status: whether successful or not 
    *  getTimeResponse of type SysRpc__GettimeofdayResponse->  stores the time in timeval format (as specified in sys/time.h)
    */
    int ret = 0, internal_status=0, internal_errno_alt=0;
    struct timeval tupdate;
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
        case MQTT_EVENT_DATA: //PROCESSES INCOMING PUBLISH REQUEST TO TOPIC SUBSCRIBED CURRENTLY. RN subscribed to 101/xRPC_Request, process accordingly.
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //Start of RPC HANDLER service below.
            len = event->data_len;
            printf("Size of event: %d \n", len);
            buffer = malloc(len);
            buffer = event->data;
            SysRpc__XRPCMessage *recvd = sys_rpc__x_rpc_message__unpack(NULL, len, buffer);//unserialize data
            printf("Message unpacked, length of buffer: %d  \n", len);
            printf("Reached check condition \n");
            if(recvd->mes_type->type == SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__request && recvd->mes_type->procedure == SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__gettimeofday)
            {         
                printf("Reached value storing. \n");   
                messageType.type = SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__response; //specify message type as response
                messageType.procedure = SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__gettimeofday;// specify procedure carried out as gettimeofday
                printf("toSend.mes_type : %p \n", toSend.mes_type);
                printf("messageType : %p \n", &messageType);
                toSend.mes_type = &(messageType);
                printf("toSend.mes_type : %p \n", toSend.mes_type);
                if(toSend.mes_type->type == SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__response && toSend.mes_type->procedure == SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__gettimeofday)
                {
                    printf("Values succesfully set by message type. \n");
                }
                printf("Starting function call: \n");
                ret = (*xRPC_func[0])(NULL, &getTimeResponse);
                printf("Return value generated from x_gettimeofday. \n");
                if(ret == 0)
                {
                    toSend.gettimeresponse = &getTimeResponse;//store values gettimeresponse
                    printf("Time obtained from gettimeofday: \n");
                    printf("seconds : %d\nmicro seconds : %d \n",toSend.gettimeresponse->timeval_r->tv_sec,toSend.gettimeresponse->timeval_r->tv_usec);
                    //store values to settimerequest
                    ReqTimeSet.tv_sec = recvd->settimerequest->timeval_s->tv_sec;
                    ReqTimeSet.tv_usec = recvd->settimerequest->timeval_s->tv_usec;
                    setTimeRequest.timeval_s = &ReqTimeSet;
                    toSend.settimerequest = &setTimeRequest;
                    //store values to settimeresponse
                    setTimeResponse.return_value = recvd->settimeresponse->return_value;
                    setTimeResponse.errno_alt = recvd->settimeresponse->errno_alt;
                    toSend.settimeresponse = &setTimeResponse;
                    //Value check; to check if they are correctly stored
                    printf("gettime response: seconds: %d | microseconds: %d| return_value: %d | errno: %d \n", 
                    toSend.gettimeresponse->timeval_r->tv_sec, toSend.gettimeresponse->timeval_r->tv_usec, toSend.gettimeresponse->status->return_value,
                    toSend.gettimeresponse->status->errno_alt);
                    printf("settimerequest: seconds: %d | microseconds: %d \n", toSend.settimerequest->timeval_s->tv_sec, toSend.settimerequest->timeval_s->tv_usec);
                    printf("set time response: return_value: %d | errno: %d \n", toSend.settimeresponse->return_value, toSend.settimeresponse->errno_alt);
                    printf("All variables set, going to pack. \n");
                    len2 = sys_rpc__x_rpc_message__get_packed_size(&toSend);
                    printf("Length of packed buffer: %d \n", len2);
                    buffer2 = malloc(len2);
                    sys_rpc__x_rpc_message__pack(&toSend, buffer2);
                    printf("Packing complete \n");
                    msg_id = esp_mqtt_client_publish(client, "101/xRPC_Response", ((char*)buffer2) , len, 0, 0);
                    ESP_LOGI(TAG, "sent publish response successful, msg_id=%d", msg_id);
                }
                
            }
            if(recvd->mes_type->type == SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__request && recvd->mes_type->procedure == SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__settimeofday)
            {
                printf("Reached value storing. \n");
                messageType.type = SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__response; //specify message type as response
                messageType.procedure = SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__settimeofday;// specify procedure carried out as settimeofday
                printf("toSend.mes_type : %p \n", toSend.mes_type);
                printf("messageType : %p \n", &messageType);
                toSend.mes_type =  &(messageType);
                printf("toSend.mes_type : %p \n", toSend.mes_type);
                if(toSend.mes_type->type == SYS_RPC__X_RPC_MESSAGE_TYPE__TYPE__response && toSend.mes_type->procedure == SYS_RPC__X_RPC_MESSAGE_TYPE__PROCEDURE__settimeofday)
                {
                    printf("Values succesfully set by message type. \n");
                }
                printf("Starting function call: \n");
                ret = (*xRPC_func[1])(recvd->settimerequest, &setTimeResponse);// execute x_settimeofday() func as defined above.COMPLETED!!
                printf("Return value generated from x_settimeofday. \n");
                //Get time from gettimeofday()
                internal_status = gettimeofday(&tupdate, NULL);
                internal_errno_alt = errno;
                if(ret == 0)
                {
                    toSend.settimeresponse = &setTimeResponse; //store values to settimeresponse

                    //store values to settimerequest
                    ReqTimeSet.tv_sec = recvd->settimerequest->timeval_s->tv_sec;
                    ReqTimeSet.tv_usec = recvd->settimerequest->timeval_s->tv_usec;
                    setTimeRequest.timeval_s = &ReqTimeSet;
                    toSend.settimerequest = &setTimeRequest;
                    //store values to gettimeresponse
                    RespTimeGet.tv_sec = tupdate.tv_sec; //get time from gettimeofday(), update both tv_sec and tv_usec
                    RespTimeGet.tv_usec = tupdate.tv_usec;
                    getTimeResponse.timeval_r = &RespTimeGet;
                    getRespStatus.return_value = internal_status;
                    getRespStatus.errno_alt = internal_errno_alt;
                    getTimeResponse.status = &getRespStatus;
                    toSend.gettimeresponse = &getTimeResponse;
                    //Value check; to check if they are correctly stored
                    printf("gettime response: seconds: %d | microseconds: %d| return_value: %d | errno: %d \n", 
                    toSend.gettimeresponse->timeval_r->tv_sec, toSend.gettimeresponse->timeval_r->tv_usec, toSend.gettimeresponse->status->return_value,
                    toSend.gettimeresponse->status->errno_alt);
                    printf("settimerequest: seconds: %d | microseconds: %d \n", toSend.settimerequest->timeval_s->tv_sec, toSend.settimerequest->timeval_s->tv_usec);
                    printf("set time response: return_value: %d | errno: %d \n", toSend.settimeresponse->return_value, toSend.settimeresponse->errno_alt);
                    printf("All variables set, going to pack. \n");
                    len2 = sys_rpc__x_rpc_message__get_packed_size(&toSend);
                    buffer2 = malloc(len2);
                    printf("Length of packed buffer: %d \n", len2);
                    sys_rpc__x_rpc_message__pack(&toSend, buffer2);
                    printf("Packing complete \n");
                    msg_id = esp_mqtt_client_publish(client, "101/xRPC_Response", ((char*)buffer2), len, 0, 0);
                    ESP_LOGI(TAG, "sent publish response successful, msg_id=%d", msg_id);
                }
            }
            //The below block will show the currently set time.
            time_t now;
            char strftime_buf[64];
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time is: %s \n", strftime_buf);
            sys_rpc__x_rpc_message__free_unpacked(recvd, NULL);// Destroy the *recvd pointer after usage and free up memory
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
