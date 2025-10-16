cmake_minimum_required(VERSION 3.10)
project(model VERSION 1.0)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(OpenCV_DIR "/home/tdt/tool/OpenCv/1.0/opencv-4.7.0/build")
find_package(OpenCV REQUIRED)

# 添加头文件包含路径
include_directories(${OpenCV_INCLUDE_DIRS} ./include)

# 添加可执行文件
add_executable(model_node model.cpp ./include/yolo.cpp ./include/light.cpp ./include/kalman_filter.cpp)

ament_target_dependencies(model_node
  rclcpp
  sensor_msgs
  cv_bridge
  OpenCV
)

# 链接 OpenCV 库
target_link_libraries(model_node ${OpenCV_LIBS})

install(TARGETS
  model_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)