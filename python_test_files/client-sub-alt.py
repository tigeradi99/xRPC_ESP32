#!/usr/bin/env python3

import queue
import time
import sys
import paho.mqtt.client as paho
import datetime
import random
import syscallprot_pb2

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
  data_inbox = syscallprot_pb2.xRPC_message()
  inb = bytes(message.payload)
  print("Raw Data: " ,inb)
  data_inbox.ParseFromString(inb)
  now = datetime.datetime.now()
  time_stamp = now.strftime("%m/%d %H:%M:%S")
  print(time_stamp, "receiving <"+ message.topic, end = '>')
  retval = syscallprot_pb2.gettimeofdayResponse.gettimeofdayRequestStatus()
  retval.CopyFrom(data_inbox.gettimeofdayResponse.gettimeofdayRequestStatus)
  #print("Message type and procedure: {} {}".format(data_inbox.mes_type.type, data_inbox.mes_type.procedure)
  print("Set time return value and errno: {} {}".format(data_inbox.setTimeResponse.return_value, data_inbox.setTimeResponse.errno_alt)
  #print("System time at ESP32 in seconds:" + data_inbox.getTimeResponse.timeval_r.tv_sec + " microseconds: " + data_inbox.getTimeResponse.timeval_r.tv_usec)
  #print("getTime response return and errno: {} {}".format(retval.return_value, retval.errno_alt))    

broker="spr.io"
port=60083
ts = datetime.datetime.now().isoformat()
c = 'client-' + ts[-6:]
print(c, broker, port)
client= paho.Client(c)
client.on_message=on_message

client.connect(broker, port)
if len(sys.argv) == 1:
    client.subscribe("#/xRPC_Response") # default: subscribe to all topics in /xRPC_Response
else:
    for arg in sys.argv[1:]:
        print("subscribing to ", arg)
        client.subscribe(arg)

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
