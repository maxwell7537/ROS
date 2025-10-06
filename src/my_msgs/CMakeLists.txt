cmake_minimum_required(VERSION 3.8)
project(my_msgs)

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# 生成接口
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Msg1.msg"
  # 添加多个消息文件用空格分隔
  # "msg/Msg2.msg" "msg/Msg3.msg"
)



ament_package()