// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from project1:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef PROJECT1__MSG__DETAIL__MSG1__STRUCT_H_
#define PROJECT1__MSG__DETAIL__MSG1__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 's'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/Msg1 in the package project1.
typedef struct project1__msg__Msg1
{
  std_msgs__msg__Header header;
  rosidl_runtime_c__String s;
  uint8_t num;
} project1__msg__Msg1;

// Struct for a sequence of project1__msg__Msg1.
typedef struct project1__msg__Msg1__Sequence
{
  project1__msg__Msg1 * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} project1__msg__Msg1__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // PROJECT1__MSG__DETAIL__MSG1__STRUCT_H_
