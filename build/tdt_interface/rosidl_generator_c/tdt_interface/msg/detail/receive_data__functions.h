// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from tdt_interface:msg/ReceiveData.idl
// generated code does not contain a copyright notice

#ifndef TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__FUNCTIONS_H_
#define TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "tdt_interface/msg/rosidl_generator_c__visibility_control.h"

#include "tdt_interface/msg/detail/receive_data__struct.h"

/// Initialize msg/ReceiveData message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * tdt_interface__msg__ReceiveData
 * )) before or use
 * tdt_interface__msg__ReceiveData__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
bool
tdt_interface__msg__ReceiveData__init(tdt_interface__msg__ReceiveData * msg);

/// Finalize msg/ReceiveData message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
void
tdt_interface__msg__ReceiveData__fini(tdt_interface__msg__ReceiveData * msg);

/// Create msg/ReceiveData message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * tdt_interface__msg__ReceiveData__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
tdt_interface__msg__ReceiveData *
tdt_interface__msg__ReceiveData__create();

/// Destroy msg/ReceiveData message.
/**
 * It calls
 * tdt_interface__msg__ReceiveData__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
void
tdt_interface__msg__ReceiveData__destroy(tdt_interface__msg__ReceiveData * msg);

/// Check for msg/ReceiveData message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
bool
tdt_interface__msg__ReceiveData__are_equal(const tdt_interface__msg__ReceiveData * lhs, const tdt_interface__msg__ReceiveData * rhs);

/// Copy a msg/ReceiveData message.
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
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
bool
tdt_interface__msg__ReceiveData__copy(
  const tdt_interface__msg__ReceiveData * input,
  tdt_interface__msg__ReceiveData * output);

/// Initialize array of msg/ReceiveData messages.
/**
 * It allocates the memory for the number of elements and calls
 * tdt_interface__msg__ReceiveData__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
bool
tdt_interface__msg__ReceiveData__Sequence__init(tdt_interface__msg__ReceiveData__Sequence * array, size_t size);

/// Finalize array of msg/ReceiveData messages.
/**
 * It calls
 * tdt_interface__msg__ReceiveData__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
void
tdt_interface__msg__ReceiveData__Sequence__fini(tdt_interface__msg__ReceiveData__Sequence * array);

/// Create array of msg/ReceiveData messages.
/**
 * It allocates the memory for the array and calls
 * tdt_interface__msg__ReceiveData__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
tdt_interface__msg__ReceiveData__Sequence *
tdt_interface__msg__ReceiveData__Sequence__create(size_t size);

/// Destroy array of msg/ReceiveData messages.
/**
 * It calls
 * tdt_interface__msg__ReceiveData__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
void
tdt_interface__msg__ReceiveData__Sequence__destroy(tdt_interface__msg__ReceiveData__Sequence * array);

/// Check for msg/ReceiveData message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
bool
tdt_interface__msg__ReceiveData__Sequence__are_equal(const tdt_interface__msg__ReceiveData__Sequence * lhs, const tdt_interface__msg__ReceiveData__Sequence * rhs);

/// Copy an array of msg/ReceiveData messages.
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
ROSIDL_GENERATOR_C_PUBLIC_tdt_interface
bool
tdt_interface__msg__ReceiveData__Sequence__copy(
  const tdt_interface__msg__ReceiveData__Sequence * input,
  tdt_interface__msg__ReceiveData__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__FUNCTIONS_H_
