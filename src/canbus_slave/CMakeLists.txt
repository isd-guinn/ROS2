cmake_minimum_required(VERSION 3.14.4) # minimum for ROS2 Humble
# cmake_minimum_required(VERSION 3.5)
project(canbus_slave)

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
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()
# find_package(rclcpp REQUIRED)
# find_package(ament_cmake REQUIRED)

find_package(rosidl_default_generators REQUIRED)
ament_export_dependencies(rosidl_default_runtime)
set(msg_files
"msg/EulerAngle.msg"
)
rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

###########
## Build ##
###########

include_directories(
  include
)

rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} rosidl_typesupport_cpp)

add_executable(s2m_receiver_can 
               src/canbus_s2m.cpp src/can_decode.cpp)
ament_target_dependencies(s2m_receiver_can 
                          rclcpp can_msgs ros2_socketcan std_msgs sensor_msgs serial_imu)
target_link_libraries(s2m_receiver_can "${cpp_typesupport_target}")

add_executable(m2s_sender_can 
               src/canbus_m2s.cpp)
ament_target_dependencies(m2s_sender_can 
                          rclcpp can_msgs ros2_socketcan std_msgs uart_slave)
target_link_libraries(m2s_sender_can "${cpp_typesupport_target}")

install(TARGETS 
        s2m_receiver_can
        m2s_sender_can
        DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()