/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: gpsw.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "gpsw.pb-c.h"
void   cfe_gpsw_pub_key_ser__init
                     (CfeGpswPubKeySer         *message)
{
  static const CfeGpswPubKeySer init_value = CFE_GPSW_PUB_KEY_SER__INIT;
  *message = init_value;
}
size_t cfe_gpsw_pub_key_ser__get_packed_size
                     (const CfeGpswPubKeySer *message)
{
  assert(message->base.descriptor == &cfe_gpsw_pub_key_ser__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t cfe_gpsw_pub_key_ser__pack
                     (const CfeGpswPubKeySer *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &cfe_gpsw_pub_key_ser__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t cfe_gpsw_pub_key_ser__pack_to_buffer
                     (const CfeGpswPubKeySer *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &cfe_gpsw_pub_key_ser__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
CfeGpswPubKeySer *
       cfe_gpsw_pub_key_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (CfeGpswPubKeySer *)
     protobuf_c_message_unpack (&cfe_gpsw_pub_key_ser__descriptor,
                                allocator, len, data);
}
void   cfe_gpsw_pub_key_ser__free_unpacked
                     (CfeGpswPubKeySer *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &cfe_gpsw_pub_key_ser__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   cfe_gpsw_key_ser__init
                     (CfeGpswKeySer         *message)
{
  static const CfeGpswKeySer init_value = CFE_GPSW_KEY_SER__INIT;
  *message = init_value;
}
size_t cfe_gpsw_key_ser__get_packed_size
                     (const CfeGpswKeySer *message)
{
  assert(message->base.descriptor == &cfe_gpsw_key_ser__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t cfe_gpsw_key_ser__pack
                     (const CfeGpswKeySer *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &cfe_gpsw_key_ser__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t cfe_gpsw_key_ser__pack_to_buffer
                     (const CfeGpswKeySer *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &cfe_gpsw_key_ser__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
CfeGpswKeySer *
       cfe_gpsw_key_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (CfeGpswKeySer *)
     protobuf_c_message_unpack (&cfe_gpsw_key_ser__descriptor,
                                allocator, len, data);
}
void   cfe_gpsw_key_ser__free_unpacked
                     (CfeGpswKeySer *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &cfe_gpsw_key_ser__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   cfe_gpsw_cipher_ser__init
                     (CfeGpswCipherSer         *message)
{
  static const CfeGpswCipherSer init_value = CFE_GPSW_CIPHER_SER__INIT;
  *message = init_value;
}
size_t cfe_gpsw_cipher_ser__get_packed_size
                     (const CfeGpswCipherSer *message)
{
  assert(message->base.descriptor == &cfe_gpsw_cipher_ser__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t cfe_gpsw_cipher_ser__pack
                     (const CfeGpswCipherSer *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &cfe_gpsw_cipher_ser__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t cfe_gpsw_cipher_ser__pack_to_buffer
                     (const CfeGpswCipherSer *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &cfe_gpsw_cipher_ser__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
CfeGpswCipherSer *
       cfe_gpsw_cipher_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (CfeGpswCipherSer *)
     protobuf_c_message_unpack (&cfe_gpsw_cipher_ser__descriptor,
                                allocator, len, data);
}
void   cfe_gpsw_cipher_ser__free_unpacked
                     (CfeGpswCipherSer *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &cfe_gpsw_cipher_ser__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor cfe_gpsw_pub_key_ser__field_descriptors[2] =
{
  {
    "t",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(CfeGpswPubKeySer, t),
    &vec_octet_ser__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "y",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(CfeGpswPubKeySer, y),
    &octet_ser__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned cfe_gpsw_pub_key_ser__field_indices_by_name[] = {
  0,   /* field[0] = t */
  1,   /* field[1] = y */
};
static const ProtobufCIntRange cfe_gpsw_pub_key_ser__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor cfe_gpsw_pub_key_ser__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "cfe_gpsw_pub_key_ser",
  "CfeGpswPubKeySer",
  "CfeGpswPubKeySer",
  "",
  sizeof(CfeGpswPubKeySer),
  2,
  cfe_gpsw_pub_key_ser__field_descriptors,
  cfe_gpsw_pub_key_ser__field_indices_by_name,
  1,  cfe_gpsw_pub_key_ser__number_ranges,
  (ProtobufCMessageInit) cfe_gpsw_pub_key_ser__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor cfe_gpsw_key_ser__field_descriptors[2] =
{
  {
    "d",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(CfeGpswKeySer, d),
    &vec_octet_ser__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "msp",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(CfeGpswKeySer, msp),
    &msp_ser__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned cfe_gpsw_key_ser__field_indices_by_name[] = {
  0,   /* field[0] = d */
  1,   /* field[1] = msp */
};
static const ProtobufCIntRange cfe_gpsw_key_ser__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor cfe_gpsw_key_ser__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "cfe_gpsw_key_ser",
  "CfeGpswKeySer",
  "CfeGpswKeySer",
  "",
  sizeof(CfeGpswKeySer),
  2,
  cfe_gpsw_key_ser__field_descriptors,
  cfe_gpsw_key_ser__field_indices_by_name,
  1,  cfe_gpsw_key_ser__number_ranges,
  (ProtobufCMessageInit) cfe_gpsw_key_ser__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor cfe_gpsw_cipher_ser__field_descriptors[3] =
{
  {
    "gamma",
    1,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_INT64,
    offsetof(CfeGpswCipherSer, n_gamma),
    offsetof(CfeGpswCipherSer, gamma),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "e0",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(CfeGpswCipherSer, e0),
    &octet_ser__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "e",
    3,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(CfeGpswCipherSer, e),
    &vec_octet_ser__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned cfe_gpsw_cipher_ser__field_indices_by_name[] = {
  2,   /* field[2] = e */
  1,   /* field[1] = e0 */
  0,   /* field[0] = gamma */
};
static const ProtobufCIntRange cfe_gpsw_cipher_ser__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor cfe_gpsw_cipher_ser__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "cfe_gpsw_cipher_ser",
  "CfeGpswCipherSer",
  "CfeGpswCipherSer",
  "",
  sizeof(CfeGpswCipherSer),
  3,
  cfe_gpsw_cipher_ser__field_descriptors,
  cfe_gpsw_cipher_ser__field_indices_by_name,
  1,  cfe_gpsw_cipher_ser__number_ranges,
  (ProtobufCMessageInit) cfe_gpsw_cipher_ser__init,
  NULL,NULL,NULL    /* reserved[123] */
};
