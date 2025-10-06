// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from project1:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef PROJECT1__MSG__DETAIL__MSG1__BUILDER_HPP_
#define PROJECT1__MSG__DETAIL__MSG1__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "project1/msg/detail/msg1__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace project1
{

namespace msg
{

namespace builder
{

class Init_Msg1_num
{
public:
  explicit Init_Msg1_num(::project1::msg::Msg1 & msg)
  : msg_(msg)
  {}
  ::project1::msg::Msg1 num(::project1::msg::Msg1::_num_type arg)
  {
    msg_.num = std::move(arg);
    return std::move(msg_);
  }

private:
  ::project1::msg::Msg1 msg_;
};

class Init_Msg1_s
{
public:
  explicit Init_Msg1_s(::project1::msg::Msg1 & msg)
  : msg_(msg)
  {}
  Init_Msg1_num s(::project1::msg::Msg1::_s_type arg)
  {
    msg_.s = std::move(arg);
    return Init_Msg1_num(msg_);
  }

private:
  ::project1::msg::Msg1 msg_;
};

class Init_Msg1_header
{
public:
  Init_Msg1_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Msg1_s header(::project1::msg::Msg1::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_Msg1_s(msg_);
  }

private:
  ::project1::msg::Msg1 msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::project1::msg::Msg1>()
{
  return project1::msg::builder::Init_Msg1_header();
}

}  // namespace project1

#endif  // PROJECT1__MSG__DETAIL__MSG1__BUILDER_HPP_
