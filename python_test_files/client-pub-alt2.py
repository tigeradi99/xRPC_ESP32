#!/usr/bin/env python3

import sys
import paho.mqtt.client as paho
import datetime
import time
import syscallprot_pb2

# print(sys.version_info)
def rpc_sen():
    data_outbox = syscallprot_pb2.xRPC_message()
    messageType = syscallprot_pb2.xRPC_message_type()
    messageType.type = syscallprot_pb2.xRPC_message_type.Type.request
    setTimeReqTimeval = syscallprot_pb2.settimeofdayRequest.timeval()
    setTimeReq = syscallprot_pb2.settimeofdayRequest()
    if sys.argv[2] == "settimeofday":
        messageType.procedure = syscallprot_pb2.xRPC_message_type.Procedure.settimeofday
        now = datetime.datetime.utcnow()
        time_stamp = now.strftime("%m/%d %H:%M:%S")
        print(time_stamp)
        sec , usec = divmod(time.time(),1)
        setTimeReqTimeval.tv_sec = int(sec)
        setTimeReqTimeval.tv_usec = int(usec * 1000000)
        print("Request to set time sent. Timeval: {} {}".format(setTimeReqTimeval.tv_sec, setTimeReqTimeval.tv_usec ))
    if sys.argv[2] == "gettimeofday":
        messageType.procedure = syscallprot_pb2.xRPC_message_type.Procedure.gettimeofday
        setTimeReqTimeval.tv_sec = 0
        setTimeReqTimeval.tv_usec = 0
        now = datetime.datetime.utcnow()
        time_stamp = now.strftime("%m/%d %H:%M:%S")
        print(time_stamp)
        print("Sent request to get time")
    setTimeResp = syscallprot_pb2.settimeofdayResponse()
    setTimeResp.return_value = 0
    setTimeResp.errno_alt = 0
    getTimeRespTimeval = syscallprot_pb2.gettimeofdayResponse.timeval()
    getTimeRespTimeval.tv_sec = 0
    getTimeRespTimeval.tv_usec = 0
    getTimeRespStatus = syscallprot_pb2.gettimeofdayResponse.gettimeofdayRequestStatus()
    getTimeRespStatus.return_value = 0
    getTimeRespStatus.errno_alt = 0
    getTimeResp = syscallprot_pb2.gettimeofdayResponse()
    getTimeResp.timeval_r.CopyFrom(getTimeRespTimeval)
    getTimeResp.status.CopyFrom(getTimeRespStatus)
    setTimeReq.timeval_s.CopyFrom(setTimeReqTimeval)
    data_outbox.mes_type.CopyFrom(messageType)
    data_outbox.setTimeRequest.CopyFrom(setTimeReq)
    data_outbox.setTimeResponse.CopyFrom(setTimeResp)
    data_outbox.getTimeResponse.CopyFrom(getTimeResp)
    otb = data_outbox.SerializeToString()
    client.publish(sys.argv[1],otb,qos=0)

broker="spr.io"
port=60083
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
    rpc_sen()
# publish <topic> <message> <retain>
elif len(sys.argv) == 4:
    data_outbox = syscallprot_pb2.xRPC_message()
    messageType = syscallprot_pb2.xRPC_message_type()
    messageType.type = syscallprot_pb2.xRPC_message_type.Type.request
    setTimeReqTimeval = syscallprot_pb2.settimeofdayRequest.timeval()
    setTimeReq = syscallprot_pb2.settimeofdayRequest()
    if sys.argv[2] == "settimeofday":
        messageType.procedure = syscallprot_pb2.xRPC_message_type.Procedure.settimeofday
        setTimeReqTimeval.tv_sec = int(round(time.time()))
        setTimeReqTimeval.tv_usec = 0
        print("Request to set time sent.")
    if sys.argv[2] == "gettimeofday":
        messageType.procedure = syscallprot_pb2.xRPC_message_type.Procedure.gettimeofday
        setTimeReqTimeval.tv_sec = 0
        setTimeReqTimeval.tv_usec = 0
        print("Sent request to get time")
    setTimeResp = syscallprot_pb2.settimeofdayResponse()
    setTimeResp.return_value = 0
    setTimeResp.errno_alt = 0
    getTimeRespTimeval = syscallprot_pb2.gettimeofdayResponse.timeval()
    getTimeRespTimeval.tv_sec = 0
    getTimeRespTimeval.tv_usec = 0
    getTimeRespStatus = syscallprot_pb2.gettimeofdayResponse.gettimeofdayRequestStatus()
    getTimeRespStatus.return_value = 0
    getTimeRespStatus.errno_alt = 0
    getTimeResp = syscallprot_pb2.gettimeofdayResponse()
    getTimeResp.timeval_r.CopyFrom(getTimeRespTimeval)
    getTimeResp.status.CopyFrom(getTimeRespStatus)
    setTimeReq.timeval_s.CopyFrom(setTimeReqTimeval)
    data_outbox.mes_type.CopyFrom(messageType)
    data_outbox.setTimeRequest.CopyFrom(setTimeReq)
    data_outbox.setTimeResponse.CopyFrom(setTimeResp)
    data_outbox.getTimeResponse.CopyFrom(getTimeResp)
    otb = data_outbox.SerializeToString()
    client.publish(sys.argv[1],otb, retain = True, qos=0)
else:
    print(sys.argv[0], '<topic> <message> [True (retain)]')

client.disconnect()
client.loop_stop()
