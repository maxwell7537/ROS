cmake_minimum_required(VERSION 3.8)
project(my_msgs)

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(sensor_msgs REQUIRED)

# 生成接口
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Msg1.msg"
  "msg/ReciveData.msg"
  "msg/SendData.msg"
  DEPENDENCIES std_msgs sensor_msgs
)

ament_package()