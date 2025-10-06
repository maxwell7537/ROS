// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from project1:msg/Msg1.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "project1/msg/detail/msg1__rosidl_typesupport_introspection_c.h"
#include "project1/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "project1/msg/detail/msg1__functions.h"
#include "project1/msg/detail/msg1__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `s`
#include "rosidl_runtime_c/string_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  project1__msg__Msg1__init(message_memory);
}

void project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_fini_function(void * message_memory)
{
  project1__msg__Msg1__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_member_array[3] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(project1__msg__Msg1, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "s",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(project1__msg__Msg1, s),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "num",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(project1__msg__Msg1, num),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_members = {
  "project1__msg",  // message namespace
  "Msg1",  // message name
  3,  // number of fields
  sizeof(project1__msg__Msg1),
  project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_member_array,  // message members
  project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_init_function,  // function to initialize message memory (memory has to be allocated)
  project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_type_support_handle = {
  0,
  &project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_project1
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, project1, msg, Msg1)() {
  project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  if (!project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_type_support_handle.typesupport_identifier) {
    project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &project1__msg__Msg1__rosidl_typesupport_introspection_c__Msg1_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
