/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: example.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "example.pb-c.h"
void   fst_msg__init
                     (FstMsg         *message)
{
  static const FstMsg init_value = FST_MSG__INIT;
  *message = init_value;
}
size_t fst_msg__get_packed_size
                     (const FstMsg *message)
{
  assert(message->base.descriptor == &fst_msg__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t fst_msg__pack
                     (const FstMsg *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &fst_msg__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t fst_msg__pack_to_buffer
                     (const FstMsg *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &fst_msg__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
FstMsg *
       fst_msg__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (FstMsg *)
     protobuf_c_message_unpack (&fst_msg__descriptor,
                                allocator, len, data);
}
void   fst_msg__free_unpacked
                     (FstMsg *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &fst_msg__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor fst_msg__field_descriptors[2] =
{
  {
    "value",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(FstMsg, value),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "data",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(FstMsg, data),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned fst_msg__field_indices_by_name[] = {
  1,   /* field[1] = data */
  0,   /* field[0] = value */
};
static const ProtobufCIntRange fst_msg__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor fst_msg__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "fst_msg",
  "FstMsg",
  "FstMsg",
  "",
  sizeof(FstMsg),
  2,
  fst_msg__field_descriptors,
  fst_msg__field_indices_by_name,
  1,  fst_msg__number_ranges,
  (ProtobufCMessageInit) fst_msg__init,
  NULL,NULL,NULL    /* reserved[123] */
};
