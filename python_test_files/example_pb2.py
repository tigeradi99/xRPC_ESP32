# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: example.proto
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='example.proto',
  package='',
  syntax='proto3',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\rexample.proto\"&\n\x07\x66st_msg\x12\r\n\x05value\x18\x01 \x01(\x0c\x12\x0c\n\x04\x64\x61ta\x18\x02 \x01(\x05\x62\x06proto3'
)




_FST_MSG = _descriptor.Descriptor(
  name='fst_msg',
  full_name='fst_msg',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='value', full_name='fst_msg.value', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=b"",
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
    _descriptor.FieldDescriptor(
      name='data', full_name='fst_msg.data', index=1,
      number=2, type=5, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=17,
  serialized_end=55,
)

DESCRIPTOR.message_types_by_name['fst_msg'] = _FST_MSG
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

fst_msg = _reflection.GeneratedProtocolMessageType('fst_msg', (_message.Message,), {
  'DESCRIPTOR' : _FST_MSG,
  '__module__' : 'example_pb2'
  # @@protoc_insertion_point(class_scope:fst_msg)
  })
_sym_db.RegisterMessage(fst_msg)


# @@protoc_insertion_point(module_scope)
