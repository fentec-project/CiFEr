/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: data.proto */

#ifndef PROTOBUF_C_data_2eproto__INCLUDED
#define PROTOBUF_C_data_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _MpzSer MpzSer;
typedef struct _MatSer MatSer;
typedef struct _OctetSer OctetSer;
typedef struct _MspSer MspSer;


/* --- enums --- */


/* --- messages --- */

struct  _MpzSer
{
  ProtobufCMessage base;
  size_t n_val;
  int64_t *val;
  protobuf_c_boolean neg;
};
#define MPZ_SER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mpz_ser__descriptor) \
    , 0,NULL, 0 }


struct  _MatSer
{
  ProtobufCMessage base;
  size_t n_val;
  MpzSer **val;
  int64_t rows;
  int64_t cols;
};
#define MAT_SER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mat_ser__descriptor) \
    , 0,NULL, 0, 0 }


struct  _OctetSer
{
  ProtobufCMessage base;
  size_t n_val;
  int32_t *val;
  int64_t len;
  int64_t max;
};
#define OCTET_SER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&octet_ser__descriptor) \
    , 0,NULL, 0, 0 }


struct  _MspSer
{
  ProtobufCMessage base;
  MatSer *mat;
  size_t n_row_to_attrib;
  int64_t *row_to_attrib;
};
#define MSP_SER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&msp_ser__descriptor) \
    , NULL, 0,NULL }


/* MpzSer methods */
void   mpz_ser__init
                     (MpzSer         *message);
size_t mpz_ser__get_packed_size
                     (const MpzSer   *message);
size_t mpz_ser__pack
                     (const MpzSer   *message,
                      uint8_t             *out);
size_t mpz_ser__pack_to_buffer
                     (const MpzSer   *message,
                      ProtobufCBuffer     *buffer);
MpzSer *
       mpz_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mpz_ser__free_unpacked
                     (MpzSer *message,
                      ProtobufCAllocator *allocator);
/* MatSer methods */
void   mat_ser__init
                     (MatSer         *message);
size_t mat_ser__get_packed_size
                     (const MatSer   *message);
size_t mat_ser__pack
                     (const MatSer   *message,
                      uint8_t             *out);
size_t mat_ser__pack_to_buffer
                     (const MatSer   *message,
                      ProtobufCBuffer     *buffer);
MatSer *
       mat_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mat_ser__free_unpacked
                     (MatSer *message,
                      ProtobufCAllocator *allocator);
/* OctetSer methods */
void   octet_ser__init
                     (OctetSer         *message);
size_t octet_ser__get_packed_size
                     (const OctetSer   *message);
size_t octet_ser__pack
                     (const OctetSer   *message,
                      uint8_t             *out);
size_t octet_ser__pack_to_buffer
                     (const OctetSer   *message,
                      ProtobufCBuffer     *buffer);
OctetSer *
       octet_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   octet_ser__free_unpacked
                     (OctetSer *message,
                      ProtobufCAllocator *allocator);
/* MspSer methods */
void   msp_ser__init
                     (MspSer         *message);
size_t msp_ser__get_packed_size
                     (const MspSer   *message);
size_t msp_ser__pack
                     (const MspSer   *message,
                      uint8_t             *out);
size_t msp_ser__pack_to_buffer
                     (const MspSer   *message,
                      ProtobufCBuffer     *buffer);
MspSer *
       msp_ser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   msp_ser__free_unpacked
                     (MspSer *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*MpzSer_Closure)
                 (const MpzSer *message,
                  void *closure_data);
typedef void (*MatSer_Closure)
                 (const MatSer *message,
                  void *closure_data);
typedef void (*OctetSer_Closure)
                 (const OctetSer *message,
                  void *closure_data);
typedef void (*MspSer_Closure)
                 (const MspSer *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor mpz_ser__descriptor;
extern const ProtobufCMessageDescriptor mat_ser__descriptor;
extern const ProtobufCMessageDescriptor octet_ser__descriptor;
extern const ProtobufCMessageDescriptor msp_ser__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_data_2eproto__INCLUDED */
