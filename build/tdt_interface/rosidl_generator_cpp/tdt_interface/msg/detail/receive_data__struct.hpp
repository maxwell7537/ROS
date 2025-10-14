// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from tdt_interface:msg/ReceiveData.idl
// generated code does not contain a copyright notice

#ifndef TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__STRUCT_HPP_
#define TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'timestamp'
#include "builtin_interfaces/msg/detail/time__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__tdt_interface__msg__ReceiveData __attribute__((deprecated))
#else
# define DEPRECATED__tdt_interface__msg__ReceiveData __declspec(deprecated)
#endif

namespace tdt_interface
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ReceiveData_
{
  using Type = ReceiveData_<ContainerAllocator>;

  explicit ReceiveData_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : timestamp(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->yaw = 0.0f;
      this->pitch = 0.0f;
    }
  }

  explicit ReceiveData_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : timestamp(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->yaw = 0.0f;
      this->pitch = 0.0f;
    }
  }

  // field types and members
  using _yaw_type =
    float;
  _yaw_type yaw;
  using _pitch_type =
    float;
  _pitch_type pitch;
  using _timestamp_type =
    builtin_interfaces::msg::Time_<ContainerAllocator>;
  _timestamp_type timestamp;

  // setters for named parameter idiom
  Type & set__yaw(
    const float & _arg)
  {
    this->yaw = _arg;
    return *this;
  }
  Type & set__pitch(
    const float & _arg)
  {
    this->pitch = _arg;
    return *this;
  }
  Type & set__timestamp(
    const builtin_interfaces::msg::Time_<ContainerAllocator> & _arg)
  {
    this->timestamp = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    tdt_interface::msg::ReceiveData_<ContainerAllocator> *;
  using ConstRawPtr =
    const tdt_interface::msg::ReceiveData_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      tdt_interface::msg::ReceiveData_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      tdt_interface::msg::ReceiveData_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__tdt_interface__msg__ReceiveData
    std::shared_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__tdt_interface__msg__ReceiveData
    std::shared_ptr<tdt_interface::msg::ReceiveData_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ReceiveData_ & other) const
  {
    if (this->yaw != other.yaw) {
      return false;
    }
    if (this->pitch != other.pitch) {
      return false;
    }
    if (this->timestamp != other.timestamp) {
      return false;
    }
    return true;
  }
  bool operator!=(const ReceiveData_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ReceiveData_

// alias to use template instance with default allocator
using ReceiveData =
  tdt_interface::msg::ReceiveData_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace tdt_interface

#endif  // TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__STRUCT_HPP_
