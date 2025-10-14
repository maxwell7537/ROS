cmake_minimum_required(VERSION 3.8)
project(shoot)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 使用ament自动查找依赖（这会自动处理ROS2包）
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(tdt_interface REQUIRED)
find_package(SDL2 REQUIRED)

# 设置OpenCV路径
set(OpenCV_DIR "/home/tdt/tool/OpenCv/1.0/opencv-4.7.0/build")

# 自动查找所有在package.xml中声明的依赖
ament_auto_find_build_dependencies()

find_package(OpenCV REQUIRED)

# 创建可执行文件
ament_auto_add_executable(shoot_node src/shoot.cpp)



# 手动链接OpenCV库（因为ament_auto可能不会自动链接）
target_link_libraries(shoot_node ${OpenCV_LIBS} ${SDL2_LIBRARIES})

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)