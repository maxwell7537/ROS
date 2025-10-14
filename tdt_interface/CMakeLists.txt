cmake_minimum_required(VERSION 3.8)
project(tdt_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找必要的构建工具和依赖
find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# 生成ROS 2消息和服务接口
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SendData.msg"
  "msg/ReceiveData.msg"
  DEPENDENCIES builtin_interfaces std_msgs
)

# 导出依赖项供其他包使用
ament_export_dependencies(rosidl_default_runtime)

# 完成包配置
ament_package()