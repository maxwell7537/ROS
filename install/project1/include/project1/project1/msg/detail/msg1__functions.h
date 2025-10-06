// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from project1:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef PROJECT1__MSG__DETAIL__MSG1__FUNCTIONS_H_
#define PROJECT1__MSG__DETAIL__MSG1__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "project1/msg/rosidl_generator_c__visibility_control.h"

#include "project1/msg/detail/msg1__struct.h"

/// Initialize msg/Msg1 message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * project1__msg__Msg1
 * )) before or use
 * project1__msg__Msg1__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
bool
project1__msg__Msg1__init(project1__msg__Msg1 * msg);

/// Finalize msg/Msg1 message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
void
project1__msg__Msg1__fini(project1__msg__Msg1 * msg);

/// Create msg/Msg1 message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * project1__msg__Msg1__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
project1__msg__Msg1 *
project1__msg__Msg1__create();

/// Destroy msg/Msg1 message.
/**
 * It calls
 * project1__msg__Msg1__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
void
project1__msg__Msg1__destroy(project1__msg__Msg1 * msg);

/// Check for msg/Msg1 message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
bool
project1__msg__Msg1__are_equal(const project1__msg__Msg1 * lhs, const project1__msg__Msg1 * rhs);

/// Copy a msg/Msg1 message.
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
ROSIDL_GENERATOR_C_PUBLIC_project1
bool
project1__msg__Msg1__copy(
  const project1__msg__Msg1 * input,
  project1__msg__Msg1 * output);

/// Initialize array of msg/Msg1 messages.
/**
 * It allocates the memory for the number of elements and calls
 * project1__msg__Msg1__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
bool
project1__msg__Msg1__Sequence__init(project1__msg__Msg1__Sequence * array, size_t size);

/// Finalize array of msg/Msg1 messages.
/**
 * It calls
 * project1__msg__Msg1__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
void
project1__msg__Msg1__Sequence__fini(project1__msg__Msg1__Sequence * array);

/// Create array of msg/Msg1 messages.
/**
 * It allocates the memory for the array and calls
 * project1__msg__Msg1__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
project1__msg__Msg1__Sequence *
project1__msg__Msg1__Sequence__create(size_t size);

/// Destroy array of msg/Msg1 messages.
/**
 * It calls
 * project1__msg__Msg1__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
void
project1__msg__Msg1__Sequence__destroy(project1__msg__Msg1__Sequence * array);

/// Check for msg/Msg1 message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_project1
bool
project1__msg__Msg1__Sequence__are_equal(const project1__msg__Msg1__Sequence * lhs, const project1__msg__Msg1__Sequence * rhs);

/// Copy an array of msg/Msg1 messages.
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
ROSIDL_GENERATOR_C_PUBLIC_project1
bool
project1__msg__Msg1__Sequence__copy(
  const project1__msg__Msg1__Sequence * input,
  project1__msg__Msg1__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // PROJECT1__MSG__DETAIL__MSG1__FUNCTIONS_H_
