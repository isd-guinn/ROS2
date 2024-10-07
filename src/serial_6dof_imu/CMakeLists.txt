# done porting
# ros1 ref: https://wiki.ros.org/catkin/CMakeLists.txt
# ros2 ref: https://docs.ros.org/en/humble/How-To-Guides/Ament-CMake-Documentation.html
# https://docs.ros.org/en/jazzy/Tutorials/Beginner-Client-Libraries/Custom-ROS2-Interfaces.html#cmakelists-txt 
# original code: https://code.corvin.cn/corvin_zhang/mini_6dof_imu/src/master/serial_6dof_imu

cmake_minimum_required(VERSION 3.14.4)
project(serial_6dof_imu)

set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
# find_package(<dependency> REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)
# find_package(message_package REQUIRED)

################################################
## Declare ROS messages, services and actions ##
################################################

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/get_yaw_data.srv"
  "srv/set_yaw_zero.srv"
  "srv/set_iic_arrv.srv"
  DEPENDENCIES sensor_msgs std_msgs
)

#############
##  Build  ##
#############

# old ros1 code #
# add_dependencies(serial_6dof_imu_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

add_executable(serial_6dof_imu_node src/serial_imu_node.cpp src/proc_serial_data.cpp) # serial_6dof_imu_node is the name of the executable
target_include_directories(serial_6dof_imu_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
## Specify libraries to link a library or executable target against
## not sure have bug or not
target_link_libraries(serial_6dof_imu_node PUBLIC
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  ${sensor_msgs_TARGETS}
  ${rosidl_default_generators_TARGETS}
)
target_compile_features(serial_6dof_imu_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
## not sure have bug or not
ament_target_dependencies(serial_6dof_imu_node
  rclcpp
  std_msgs
  sensor_msgs
)

# Install executables
install(TARGETS serial_6dof_imu_node
  DESTINATION lib/${PROJECT_NAME})

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_dependencies(rosidl_default_runtime)
ament_package()