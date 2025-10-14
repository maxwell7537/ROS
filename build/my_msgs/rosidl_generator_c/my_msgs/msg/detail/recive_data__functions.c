// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from my_msgs:msg/ReciveData.idl
// generated code does not contain a copyright notice
#include "my_msgs/msg/detail/recive_data__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `timestamp`
#include "builtin_interfaces/msg/detail/time__functions.h"

bool
my_msgs__msg__ReciveData__init(my_msgs__msg__ReciveData * msg)
{
  if (!msg) {
    return false;
  }
  // yaw
  // pitch
  // timestamp
  if (!builtin_interfaces__msg__Time__init(&msg->timestamp)) {
    my_msgs__msg__ReciveData__fini(msg);
    return false;
  }
  return true;
}

void
my_msgs__msg__ReciveData__fini(my_msgs__msg__ReciveData * msg)
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
my_msgs__msg__ReciveData__are_equal(const my_msgs__msg__ReciveData * lhs, const my_msgs__msg__ReciveData * rhs)
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
my_msgs__msg__ReciveData__copy(
  const my_msgs__msg__ReciveData * input,
  my_msgs__msg__ReciveData * output)
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

my_msgs__msg__ReciveData *
my_msgs__msg__ReciveData__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  my_msgs__msg__ReciveData * msg = (my_msgs__msg__ReciveData *)allocator.allocate(sizeof(my_msgs__msg__ReciveData), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(my_msgs__msg__ReciveData));
  bool success = my_msgs__msg__ReciveData__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
my_msgs__msg__ReciveData__destroy(my_msgs__msg__ReciveData * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    my_msgs__msg__ReciveData__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
my_msgs__msg__ReciveData__Sequence__init(my_msgs__msg__ReciveData__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  my_msgs__msg__ReciveData * data = NULL;

  if (size) {
    data = (my_msgs__msg__ReciveData *)allocator.zero_allocate(size, sizeof(my_msgs__msg__ReciveData), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = my_msgs__msg__ReciveData__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        my_msgs__msg__ReciveData__fini(&data[i - 1]);
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
my_msgs__msg__ReciveData__Sequence__fini(my_msgs__msg__ReciveData__Sequence * array)
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
      my_msgs__msg__ReciveData__fini(&array->data[i]);
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

my_msgs__msg__ReciveData__Sequence *
my_msgs__msg__ReciveData__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  my_msgs__msg__ReciveData__Sequence * array = (my_msgs__msg__ReciveData__Sequence *)allocator.allocate(sizeof(my_msgs__msg__ReciveData__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = my_msgs__msg__ReciveData__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
my_msgs__msg__ReciveData__Sequence__destroy(my_msgs__msg__ReciveData__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    my_msgs__msg__ReciveData__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
my_msgs__msg__ReciveData__Sequence__are_equal(const my_msgs__msg__ReciveData__Sequence * lhs, const my_msgs__msg__ReciveData__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!my_msgs__msg__ReciveData__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
my_msgs__msg__ReciveData__Sequence__copy(
  const my_msgs__msg__ReciveData__Sequence * input,
  my_msgs__msg__ReciveData__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(my_msgs__msg__ReciveData);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    my_msgs__msg__ReciveData * data =
      (my_msgs__msg__ReciveData *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!my_msgs__msg__ReciveData__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          my_msgs__msg__ReciveData__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!my_msgs__msg__ReciveData__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
