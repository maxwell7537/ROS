// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_msgs:msg/SendData.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__SEND_DATA__STRUCT_H_
#define MY_MSGS__MSG__DETAIL__SEND_DATA__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/SendData in the package my_msgs.
typedef struct my_msgs__msg__SendData
{
  /// 底盘系的yaw
  float yaw;
  /// 底盘系的pitch
  float pitch;
  /// 是否射击
  bool if_shoot;
} my_msgs__msg__SendData;

// Struct for a sequence of my_msgs__msg__SendData.
typedef struct my_msgs__msg__SendData__Sequence
{
  my_msgs__msg__SendData * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_msgs__msg__SendData__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_MSGS__MSG__DETAIL__SEND_DATA__STRUCT_H_
