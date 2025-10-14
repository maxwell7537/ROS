// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from my_msgs:msg/SendData.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__SEND_DATA__BUILDER_HPP_
#define MY_MSGS__MSG__DETAIL__SEND_DATA__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "my_msgs/msg/detail/send_data__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace my_msgs
{

namespace msg
{

namespace builder
{

class Init_SendData_if_shoot
{
public:
  explicit Init_SendData_if_shoot(::my_msgs::msg::SendData & msg)
  : msg_(msg)
  {}
  ::my_msgs::msg::SendData if_shoot(::my_msgs::msg::SendData::_if_shoot_type arg)
  {
    msg_.if_shoot = std::move(arg);
    return std::move(msg_);
  }

private:
  ::my_msgs::msg::SendData msg_;
};

class Init_SendData_pitch
{
public:
  explicit Init_SendData_pitch(::my_msgs::msg::SendData & msg)
  : msg_(msg)
  {}
  Init_SendData_if_shoot pitch(::my_msgs::msg::SendData::_pitch_type arg)
  {
    msg_.pitch = std::move(arg);
    return Init_SendData_if_shoot(msg_);
  }

private:
  ::my_msgs::msg::SendData msg_;
};

class Init_SendData_yaw
{
public:
  Init_SendData_yaw()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_SendData_pitch yaw(::my_msgs::msg::SendData::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_SendData_pitch(msg_);
  }

private:
  ::my_msgs::msg::SendData msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::my_msgs::msg::SendData>()
{
  return my_msgs::msg::builder::Init_SendData_yaw();
}

}  // namespace my_msgs

#endif  // MY_MSGS__MSG__DETAIL__SEND_DATA__BUILDER_HPP_
