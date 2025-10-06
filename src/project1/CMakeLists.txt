cmake_minimum_required(VERSION 3.8)
project(project1)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(my_msgs REQUIRED)
ament_auto_find_build_dependencies()
set(msg_files
  "msg/Msg1.msg"
  # 若有多个消息文件，继续添加，如"msg/AnotherMessage.msg"
)

# 生成消息接口代码（核心步骤）
rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES std_msgs  # 依赖的其他消息包（如std_msgs）
  ADD_LINTER_TESTS
)

# 安装消息相关文件
ament_export_dependencies(rosidl_default_runtime)


# 自动创建可执行文件并处理依赖
ament_auto_add_executable(talker1_node src/talker1.cpp)
ament_auto_add_executable(listener1_node src/listener1.cpp)

# 自动处理包配置和安装
ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)