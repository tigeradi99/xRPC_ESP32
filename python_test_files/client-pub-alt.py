#!/usr/bin/env python3

import sys
import paho.mqtt.client as paho
import datetime
import time
import syscallprot_pb2

# print(sys.version_info)

broker="mqtt.eclipse.org"
port=1883
ts = datetime.datetime.now().isoformat()
c = 'client-' + ts[-6:] # use the timestamp to create unique MQTT client
print(c, broker, port)

client= paho.Client(c)
client.connect(broker, port)

# publish an empty string
if len(sys.argv) == 2:
    client.publish(sys.argv[1],'',retain = False, qos=0)
# publish <topic> <message>
elif len(sys.argv) == 3:
    data_outbox = syscallprot_pb2.xRPC_message()
    data_outbox.mes_type.type = syscallprot_pb2.xRPC_message_type.Type.request
    if sys.argv[2] == "settimeofday":
        data_outbox.mes_type.procedure = syscallprot_pb2.xRPC_message_type.Procedure.settimeofday
        data_outbox.setTimeRequest.timeval_s.tv_sec = int(round(time.time()))
        data_outbox.setTimeRequest.timeval_s.tv_usec = 0
        print("Request to set time sent.")
    if sys.argv[2] == "gettimeofday":
        data_outbox.mes_type.procedure = syscallprot_pb2.xRPC_message_type.Procedure.gettimeofday
        print("Sent request to get time")
    otb = data_outbox.SerializeToString()
    client.publish(sys.argv[1],otb,qos=0)
# publish <topic> <message> <retain>
elif len(sys.argv) == 4:
    data_outbox = syscallprot_pb2.xRPC_message()
    data_outbox.mes_type.type = syscallprot_pb2.xRPC_message_type.Type.request
    if sys.argv[2] == "settimeofday":
        data_outbox.mes_type.procedure = syscallprot_pb2.xRPC_message_type.Procedure.settimeofday
        data_outbox.setTimeRequest.timeval_s.tv_sec = long(round(time.time()))
        data_outbox.setTimeRequest.timeval_s.tv_usec = long(round(time.time() * 1000000))
        print("Request to set time sent.")
    if sys.argv[2] == "gettimeofday":
        data_outbox.mes_type.procedure = syscallprot_pb2.xRPC_message_type.Procedure.gettimeofday
        print("Sent request to get time")
    otb = data_outbox.SerializeToString()
    client.publish(sys.argv[1],otb, retain = True, qos=0)
else:
    print(sys.argv[0], '<topic> <message> [True (retain)]')

client.disconnect()
client.loop_stop()
