// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from project1:msg/Msg1.idl
// generated code does not contain a copyright notice

#ifndef PROJECT1__MSG__DETAIL__MSG1__TRAITS_HPP_
#define PROJECT1__MSG__DETAIL__MSG1__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "project1/msg/detail/msg1__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace project1
{

namespace msg
{

inline void to_flow_style_yaml(
  const Msg1 & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: s
  {
    out << "s: ";
    rosidl_generator_traits::value_to_yaml(msg.s, out);
    out << ", ";
  }

  // member: num
  {
    out << "num: ";
    rosidl_generator_traits::value_to_yaml(msg.num, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Msg1 & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: s
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "s: ";
    rosidl_generator_traits::value_to_yaml(msg.s, out);
    out << "\n";
  }

  // member: num
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "num: ";
    rosidl_generator_traits::value_to_yaml(msg.num, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Msg1 & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace project1

namespace rosidl_generator_traits
{

[[deprecated("use project1::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const project1::msg::Msg1 & msg,
  std::ostream & out, size_t indentation = 0)
{
  project1::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use project1::msg::to_yaml() instead")]]
inline std::string to_yaml(const project1::msg::Msg1 & msg)
{
  return project1::msg::to_yaml(msg);
}

template<>
inline const char * data_type<project1::msg::Msg1>()
{
  return "project1::msg::Msg1";
}

template<>
inline const char * name<project1::msg::Msg1>()
{
  return "project1/msg/Msg1";
}

template<>
struct has_fixed_size<project1::msg::Msg1>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<project1::msg::Msg1>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<project1::msg::Msg1>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // PROJECT1__MSG__DETAIL__MSG1__TRAITS_HPP_
