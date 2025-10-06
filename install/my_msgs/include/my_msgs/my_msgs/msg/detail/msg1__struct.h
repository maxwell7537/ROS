// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_msgs:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__MSG1__STRUCT_H_
#define MY_MSGS__MSG__DETAIL__MSG1__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 's'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Msg1 in the package my_msgs.
typedef struct my_msgs__msg__Msg1
{
  rosidl_runtime_c__String s;
  int32_t num;
} my_msgs__msg__Msg1;

// Struct for a sequence of my_msgs__msg__Msg1.
typedef struct my_msgs__msg__Msg1__Sequence
{
  my_msgs__msg__Msg1 * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_msgs__msg__Msg1__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_MSGS__MSG__DETAIL__MSG1__STRUCT_H_
