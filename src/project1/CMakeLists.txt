cmake_minimum_required(VERSION 3.8)
project(project1)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 使用ament自动查找依赖（这会自动处理ROS2包）
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)

# 设置OpenCV路径
set(OpenCV_DIR "/home/tdt/tool/OpenCv/1.0/opencv-4.7.0/build")

# 自动查找所有在package.xml中声明的依赖
ament_auto_find_build_dependencies()

# 手动查找OpenCV（绕过组件检查）
find_package(OpenCV REQUIRED)

# 创建可执行文件
ament_auto_add_executable(talker1_node src/talker1.cpp)
ament_auto_add_executable(listener1_node src/listener1.cpp)

# 手动链接OpenCV库（因为ament_auto可能不会自动链接）
target_link_libraries(talker1_node ${OpenCV_LIBS})
target_link_libraries(listener1_node ${OpenCV_LIBS})

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)