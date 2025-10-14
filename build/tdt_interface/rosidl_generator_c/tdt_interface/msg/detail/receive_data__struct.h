// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from tdt_interface:msg/ReceiveData.idl
// generated code does not contain a copyright notice

#ifndef TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__STRUCT_H_
#define TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'timestamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in msg/ReceiveData in the package tdt_interface.
typedef struct tdt_interface__msg__ReceiveData
{
  /// 底盘系的yaw
  float yaw;
  /// 底盘系的pitch
  float pitch;
  builtin_interfaces__msg__Time timestamp;
} tdt_interface__msg__ReceiveData;

// Struct for a sequence of tdt_interface__msg__ReceiveData.
typedef struct tdt_interface__msg__ReceiveData__Sequence
{
  tdt_interface__msg__ReceiveData * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} tdt_interface__msg__ReceiveData__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__STRUCT_H_
