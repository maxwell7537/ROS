// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from my_msgs:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__MSG1__BUILDER_HPP_
#define MY_MSGS__MSG__DETAIL__MSG1__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "my_msgs/msg/detail/msg1__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace my_msgs
{

namespace msg
{

namespace builder
{

class Init_Msg1_image
{
public:
  explicit Init_Msg1_image(::my_msgs::msg::Msg1 & msg)
  : msg_(msg)
  {}
  ::my_msgs::msg::Msg1 image(::my_msgs::msg::Msg1::_image_type arg)
  {
    msg_.image = std::move(arg);
    return std::move(msg_);
  }

private:
  ::my_msgs::msg::Msg1 msg_;
};

class Init_Msg1_num
{
public:
  explicit Init_Msg1_num(::my_msgs::msg::Msg1 & msg)
  : msg_(msg)
  {}
  Init_Msg1_image num(::my_msgs::msg::Msg1::_num_type arg)
  {
    msg_.num = std::move(arg);
    return Init_Msg1_image(msg_);
  }

private:
  ::my_msgs::msg::Msg1 msg_;
};

class Init_Msg1_s
{
public:
  Init_Msg1_s()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Msg1_num s(::my_msgs::msg::Msg1::_s_type arg)
  {
    msg_.s = std::move(arg);
    return Init_Msg1_num(msg_);
  }

private:
  ::my_msgs::msg::Msg1 msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::my_msgs::msg::Msg1>()
{
  return my_msgs::msg::builder::Init_Msg1_s();
}

}  // namespace my_msgs

#endif  // MY_MSGS__MSG__DETAIL__MSG1__BUILDER_HPP_
