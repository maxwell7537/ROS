cmake_minimum_required(VERSION 3.8)
project(image)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 使用ament自动查找依赖（这会自动处理ROS2包）
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(X11 REQUIRED)

ament_auto_find_build_dependencies()# 自动查找所有在package.xml中声明的依赖
# 设置OpenCV路径
set(OpenCV_DIR "/home/tdt/tool/OpenCv/1.0/opencv-4.7.0/build")
find_package(OpenCV REQUIRED)

# 创建可执行文件
ament_auto_add_executable(image_node src/image.cpp)

ament_target_dependencies(image_node
  rclcpp
  sensor_msgs
  cv_bridge
  OpenCV
)

target_link_libraries(image_node ${OpenCV_LIBS} ${X11_LIBRARIES})

install(TARGETS
  image_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)

