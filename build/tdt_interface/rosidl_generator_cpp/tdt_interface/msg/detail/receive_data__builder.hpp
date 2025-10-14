// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from tdt_interface:msg/ReceiveData.idl
// generated code does not contain a copyright notice

#ifndef TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__BUILDER_HPP_
#define TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "tdt_interface/msg/detail/receive_data__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace tdt_interface
{

namespace msg
{

namespace builder
{

class Init_ReceiveData_timestamp
{
public:
  explicit Init_ReceiveData_timestamp(::tdt_interface::msg::ReceiveData & msg)
  : msg_(msg)
  {}
  ::tdt_interface::msg::ReceiveData timestamp(::tdt_interface::msg::ReceiveData::_timestamp_type arg)
  {
    msg_.timestamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::tdt_interface::msg::ReceiveData msg_;
};

class Init_ReceiveData_pitch
{
public:
  explicit Init_ReceiveData_pitch(::tdt_interface::msg::ReceiveData & msg)
  : msg_(msg)
  {}
  Init_ReceiveData_timestamp pitch(::tdt_interface::msg::ReceiveData::_pitch_type arg)
  {
    msg_.pitch = std::move(arg);
    return Init_ReceiveData_timestamp(msg_);
  }

private:
  ::tdt_interface::msg::ReceiveData msg_;
};

class Init_ReceiveData_yaw
{
public:
  Init_ReceiveData_yaw()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ReceiveData_pitch yaw(::tdt_interface::msg::ReceiveData::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_ReceiveData_pitch(msg_);
  }

private:
  ::tdt_interface::msg::ReceiveData msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::tdt_interface::msg::ReceiveData>()
{
  return tdt_interface::msg::builder::Init_ReceiveData_yaw();
}

}  // namespace tdt_interface

#endif  // TDT_INTERFACE__MSG__DETAIL__RECEIVE_DATA__BUILDER_HPP_
