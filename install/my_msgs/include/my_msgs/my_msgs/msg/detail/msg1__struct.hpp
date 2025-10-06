// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from my_msgs:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__MSG1__STRUCT_HPP_
#define MY_MSGS__MSG__DETAIL__MSG1__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__my_msgs__msg__Msg1 __attribute__((deprecated))
#else
# define DEPRECATED__my_msgs__msg__Msg1 __declspec(deprecated)
#endif

namespace my_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Msg1_
{
  using Type = Msg1_<ContainerAllocator>;

  explicit Msg1_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->s = "";
      this->num = 0l;
    }
  }

  explicit Msg1_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : s(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->s = "";
      this->num = 0l;
    }
  }

  // field types and members
  using _s_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _s_type s;
  using _num_type =
    int32_t;
  _num_type num;

  // setters for named parameter idiom
  Type & set__s(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->s = _arg;
    return *this;
  }
  Type & set__num(
    const int32_t & _arg)
  {
    this->num = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    my_msgs::msg::Msg1_<ContainerAllocator> *;
  using ConstRawPtr =
    const my_msgs::msg::Msg1_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<my_msgs::msg::Msg1_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<my_msgs::msg::Msg1_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      my_msgs::msg::Msg1_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<my_msgs::msg::Msg1_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      my_msgs::msg::Msg1_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<my_msgs::msg::Msg1_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<my_msgs::msg::Msg1_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<my_msgs::msg::Msg1_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__my_msgs__msg__Msg1
    std::shared_ptr<my_msgs::msg::Msg1_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__my_msgs__msg__Msg1
    std::shared_ptr<my_msgs::msg::Msg1_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Msg1_ & other) const
  {
    if (this->s != other.s) {
      return false;
    }
    if (this->num != other.num) {
      return false;
    }
    return true;
  }
  bool operator!=(const Msg1_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Msg1_

// alias to use template instance with default allocator
using Msg1 =
  my_msgs::msg::Msg1_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace my_msgs

#endif  // MY_MSGS__MSG__DETAIL__MSG1__STRUCT_HPP_
