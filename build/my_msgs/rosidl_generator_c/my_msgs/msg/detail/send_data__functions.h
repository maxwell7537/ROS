// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from my_msgs:msg/SendData.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__SEND_DATA__FUNCTIONS_H_
#define MY_MSGS__MSG__DETAIL__SEND_DATA__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "my_msgs/msg/rosidl_generator_c__visibility_control.h"

#include "my_msgs/msg/detail/send_data__struct.h"

/// Initialize msg/SendData message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * my_msgs__msg__SendData
 * )) before or use
 * my_msgs__msg__SendData__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
bool
my_msgs__msg__SendData__init(my_msgs__msg__SendData * msg);

/// Finalize msg/SendData message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
void
my_msgs__msg__SendData__fini(my_msgs__msg__SendData * msg);

/// Create msg/SendData message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * my_msgs__msg__SendData__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
my_msgs__msg__SendData *
my_msgs__msg__SendData__create();

/// Destroy msg/SendData message.
/**
 * It calls
 * my_msgs__msg__SendData__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
void
my_msgs__msg__SendData__destroy(my_msgs__msg__SendData * msg);

/// Check for msg/SendData message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
bool
my_msgs__msg__SendData__are_equal(const my_msgs__msg__SendData * lhs, const my_msgs__msg__SendData * rhs);

/// Copy a msg/SendData message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
bool
my_msgs__msg__SendData__copy(
  const my_msgs__msg__SendData * input,
  my_msgs__msg__SendData * output);

/// Initialize array of msg/SendData messages.
/**
 * It allocates the memory for the number of elements and calls
 * my_msgs__msg__SendData__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
bool
my_msgs__msg__SendData__Sequence__init(my_msgs__msg__SendData__Sequence * array, size_t size);

/// Finalize array of msg/SendData messages.
/**
 * It calls
 * my_msgs__msg__SendData__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
void
my_msgs__msg__SendData__Sequence__fini(my_msgs__msg__SendData__Sequence * array);

/// Create array of msg/SendData messages.
/**
 * It allocates the memory for the array and calls
 * my_msgs__msg__SendData__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
my_msgs__msg__SendData__Sequence *
my_msgs__msg__SendData__Sequence__create(size_t size);

/// Destroy array of msg/SendData messages.
/**
 * It calls
 * my_msgs__msg__SendData__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
void
my_msgs__msg__SendData__Sequence__destroy(my_msgs__msg__SendData__Sequence * array);

/// Check for msg/SendData message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
bool
my_msgs__msg__SendData__Sequence__are_equal(const my_msgs__msg__SendData__Sequence * lhs, const my_msgs__msg__SendData__Sequence * rhs);

/// Copy an array of msg/SendData messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_msgs
bool
my_msgs__msg__SendData__Sequence__copy(
  const my_msgs__msg__SendData__Sequence * input,
  my_msgs__msg__SendData__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // MY_MSGS__MSG__DETAIL__SEND_DATA__FUNCTIONS_H_
