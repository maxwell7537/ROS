// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from tdt_interface:msg/ReceiveData.idl
// generated code does not contain a copyright notice
#include "tdt_interface/msg/detail/receive_data__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `timestamp`
#include "builtin_interfaces/msg/detail/time__functions.h"

bool
tdt_interface__msg__ReceiveData__init(tdt_interface__msg__ReceiveData * msg)
{
  if (!msg) {
    return false;
  }
  // yaw
  // pitch
  // timestamp
  if (!builtin_interfaces__msg__Time__init(&msg->timestamp)) {
    tdt_interface__msg__ReceiveData__fini(msg);
    return false;
  }
  return true;
}

void
tdt_interface__msg__ReceiveData__fini(tdt_interface__msg__ReceiveData * msg)
{
  if (!msg) {
    return;
  }
  // yaw
  // pitch
  // timestamp
  builtin_interfaces__msg__Time__fini(&msg->timestamp);
}

bool
tdt_interface__msg__ReceiveData__are_equal(const tdt_interface__msg__ReceiveData * lhs, const tdt_interface__msg__ReceiveData * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // yaw
  if (lhs->yaw != rhs->yaw) {
    return false;
  }
  // pitch
  if (lhs->pitch != rhs->pitch) {
    return false;
  }
  // timestamp
  if (!builtin_interfaces__msg__Time__are_equal(
      &(lhs->timestamp), &(rhs->timestamp)))
  {
    return false;
  }
  return true;
}

bool
tdt_interface__msg__ReceiveData__copy(
  const tdt_interface__msg__ReceiveData * input,
  tdt_interface__msg__ReceiveData * output)
{
  if (!input || !output) {
    return false;
  }
  // yaw
  output->yaw = input->yaw;
  // pitch
  output->pitch = input->pitch;
  // timestamp
  if (!builtin_interfaces__msg__Time__copy(
      &(input->timestamp), &(output->timestamp)))
  {
    return false;
  }
  return true;
}

tdt_interface__msg__ReceiveData *
tdt_interface__msg__ReceiveData__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  tdt_interface__msg__ReceiveData * msg = (tdt_interface__msg__ReceiveData *)allocator.allocate(sizeof(tdt_interface__msg__ReceiveData), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(tdt_interface__msg__ReceiveData));
  bool success = tdt_interface__msg__ReceiveData__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
tdt_interface__msg__ReceiveData__destroy(tdt_interface__msg__ReceiveData * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    tdt_interface__msg__ReceiveData__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
tdt_interface__msg__ReceiveData__Sequence__init(tdt_interface__msg__ReceiveData__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  tdt_interface__msg__ReceiveData * data = NULL;

  if (size) {
    data = (tdt_interface__msg__ReceiveData *)allocator.zero_allocate(size, sizeof(tdt_interface__msg__ReceiveData), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = tdt_interface__msg__ReceiveData__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        tdt_interface__msg__ReceiveData__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
tdt_interface__msg__ReceiveData__Sequence__fini(tdt_interface__msg__ReceiveData__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      tdt_interface__msg__ReceiveData__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

tdt_interface__msg__ReceiveData__Sequence *
tdt_interface__msg__ReceiveData__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  tdt_interface__msg__ReceiveData__Sequence * array = (tdt_interface__msg__ReceiveData__Sequence *)allocator.allocate(sizeof(tdt_interface__msg__ReceiveData__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = tdt_interface__msg__ReceiveData__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
tdt_interface__msg__ReceiveData__Sequence__destroy(tdt_interface__msg__ReceiveData__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    tdt_interface__msg__ReceiveData__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
tdt_interface__msg__ReceiveData__Sequence__are_equal(const tdt_interface__msg__ReceiveData__Sequence * lhs, const tdt_interface__msg__ReceiveData__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!tdt_interface__msg__ReceiveData__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
tdt_interface__msg__ReceiveData__Sequence__copy(
  const tdt_interface__msg__ReceiveData__Sequence * input,
  tdt_interface__msg__ReceiveData__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(tdt_interface__msg__ReceiveData);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    tdt_interface__msg__ReceiveData * data =
      (tdt_interface__msg__ReceiveData *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!tdt_interface__msg__ReceiveData__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          tdt_interface__msg__ReceiveData__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!tdt_interface__msg__ReceiveData__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
