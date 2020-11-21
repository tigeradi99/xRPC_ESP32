#!/usr/bin/env python3

import queue
import time
import sys
import paho.mqtt.client as paho
import datetime
import random
import messagefile_pb2 as message_in

# print(sys.version_info)

message_q=queue.Queue()

def empty_queue(delay=0):
    while not message_q.empty():
      m=message_q.get()
      print("Received message  ",m)
    if delay!=0:
      time.sleep(delay)
#define callback
def on_message(client, userdata, message):
  #time.sleep(1)
  data_inbox = message_in.xRPCMessage()
  inb = message.payload
  print("Raw Data: " ,inb)
  print("Parsing message.")
  data_inbox.ParseFromString(inb)
  print("Data inbox: ", data_inbox)
  now = datetime.datetime.now()
  time_stamp = now.strftime("%m/%d %H:%M:%S")
  print(time_stamp, "receiving <"+ message.topic +">")
  if data_inbox.response.HasField("gettimeofday_response"):
      print("Message type: gettimeofday_response")
      print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
      print("System time at ESP32 in seconds and microseconds: {} {}".format(data_inbox.response.gettimeofday_response.timeval.tv_sec, data_inbox.response.gettimeofday_response.timeval.tv_usec))
      print("getTime response return and errno: {} {}".format(data_inbox.response.gettimeofday_response.return_value, data_inbox.response.gettimeofday_response.errno))
  elif data_inbox.response.HasField("settimeofday_response"):
      print("Procedure called: settimeofday_response")
      print("Set time return value and errno: {} {}".format(data_inbox.response.settimeofday_response.return_value, data_inbox.response.settimeofday_response.errno))
      print("Timestamp: seconds: {} microseconds: {}".format(data_inbox.time_stamp.tv_sec , data_inbox.time_stamp.tv_usec))
  else:
      print("Requests are not supported by the subscriber.")

def rpc_sen(topic , procedure):
    data_outbox = message_in.xRPCMessage()
    
    sec1 , usec1 = divmod(time.time(),1)
    time_stamp = message_in.TimeVal()
    time_stamp.tv_sec = int(sec1)
    time_stamp.tv_usec = int(usec1 * 1000000)
    data_outbox.time_stamp.CopyFrom(time_stamp)
    print("Current timestamp: seconds {} | microseconds {}".format(data_outbox.time_stamp.tv_sec, data_outbox.time_stamp.tv_usec))
    if procedure == "settimeofday":
        now = datetime.datetime.utcnow()
        time_stamp = now.strftime("%m/%d %H:%M:%S")
        print(time_stamp)
        sec , usec = divmod(time.time(),1)
        time_stamp_setRequest = message_in.TimeVal()
        time_stamp_setRequest.tv_sec = int(sec)
        time_stamp_setRequest.tv_usec = int(usec * 1000000)
        data_outbox.request.settimeofday_request.timeval.CopyFrom(time_stamp_setRequest)
        print("Request to set time sent. Timeval: {} {}".format(data_outbox.request.settimeofday_request.timeval.tv_sec, 
        data_outbox.request.settimeofday_request.timeval.tv_usec ))
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
    if procedure == "gettimeofday":
        data_outbox.request.gettimeofday_request.stub = 0
        print("Message type:", data_outbox.request)
        print("Outbox: ", data_outbox)
        print("Request to get time set.")
    otb = data_outbox.SerializeToString()
    client.publish(topic,otb,qos=0)


broker="spr.io"
port=60083
ts = datetime.datetime.now().isoformat()
c = 'client-' + ts[-6:]
print(c, broker, port)
client= paho.Client(c)
client.on_message=on_message
print("Enter subscriber topic:")
topic = input()
client.connect(broker, port)

client.subscribe("+/xRPC_Response") # default: subscribe to all topics in /xRPC_Response
print("Enter procedure: gettimeofday/settimeofday. ")
procedure = input()
rpc_sen(topic , procedure)

client.loop_start() #start loop to process received messages

time.sleep(1)
try:
  while True:
    time.sleep(1)
    pass
except KeyboardInterrupt:
    print ("You hit control-c")

time.sleep(1)

client.disconnect()
client.loop_stop()
