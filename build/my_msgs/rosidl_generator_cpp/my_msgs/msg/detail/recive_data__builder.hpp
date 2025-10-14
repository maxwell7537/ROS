// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from my_msgs:msg/ReciveData.idl
// generated code does not contain a copyright notice

#ifndef MY_MSGS__MSG__DETAIL__RECIVE_DATA__BUILDER_HPP_
#define MY_MSGS__MSG__DETAIL__RECIVE_DATA__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "my_msgs/msg/detail/recive_data__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace my_msgs
{

namespace msg
{

namespace builder
{

class Init_ReciveData_timestamp
{
public:
  explicit Init_ReciveData_timestamp(::my_msgs::msg::ReciveData & msg)
  : msg_(msg)
  {}
  ::my_msgs::msg::ReciveData timestamp(::my_msgs::msg::ReciveData::_timestamp_type arg)
  {
    msg_.timestamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::my_msgs::msg::ReciveData msg_;
};

class Init_ReciveData_pitch
{
public:
  explicit Init_ReciveData_pitch(::my_msgs::msg::ReciveData & msg)
  : msg_(msg)
  {}
  Init_ReciveData_timestamp pitch(::my_msgs::msg::ReciveData::_pitch_type arg)
  {
    msg_.pitch = std::move(arg);
    return Init_ReciveData_timestamp(msg_);
  }

private:
  ::my_msgs::msg::ReciveData msg_;
};

class Init_ReciveData_yaw
{
public:
  Init_ReciveData_yaw()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ReciveData_pitch yaw(::my_msgs::msg::ReciveData::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_ReciveData_pitch(msg_);
  }

private:
  ::my_msgs::msg::ReciveData msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::my_msgs::msg::ReciveData>()
{
  return my_msgs::msg::builder::Init_ReciveData_yaw();
}

}  // namespace my_msgs

#endif  // MY_MSGS__MSG__DETAIL__RECIVE_DATA__BUILDER_HPP_
