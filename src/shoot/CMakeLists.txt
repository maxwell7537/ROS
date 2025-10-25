cmake_minimum_required(VERSION 3.8)
project(shoot)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找必需的依赖
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tdt_interface REQUIRED)

# 创建两个不同的可执行文件，分别使用不同的瞄准方式
add_executable(shoot_pnp src/shoot.cpp)
target_compile_definitions(shoot_pnp PRIVATE USE_PNP_WORLD_COORDINATES)
target_include_directories(shoot_pnp PRIVATE include)
# 使用 ament_target_dependencies 而不是直接链接
ament_target_dependencies(shoot_pnp rclcpp std_msgs tdt_interface)

add_executable(shoot_pixel src/shoot.cpp src/shoot.hpp)
target_compile_definitions(shoot_pixel PRIVATE USE_PIXEL_CENTER_AIM)
target_include_directories(shoot_pixel PRIVATE include)
# 使用 ament_target_dependencies 而不是直接链接
ament_target_dependencies(shoot_pixel rclcpp std_msgs tdt_interface)

# 安装两个可执行文件
install(TARGETS
  shoot_pnp
  shoot_pixel
  DESTINATION lib/${PROJECT_NAME}
)

# 安装包含目录（如果有头文件需要安装）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
  install(DIRECTORY include/
    DESTINATION include/${PROJECT_NAME}
  )
endif()

# 导出依赖信息
ament_export_dependencies(rclcpp std_msgs tdt_interface)
ament_export_include_directories(include)

# 生成包配置
ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)