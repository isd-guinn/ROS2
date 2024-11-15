cmake_minimum_required(VERSION 3.14.4)
project(ros2_socketcan)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# # Default to C++14
# if(NOT CMAKE_CXX_STANDARD)
#   set(CMAKE_CXX_STANDARD 14)
# endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(rclcpp REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(ament_cmake_auto REQUIRED)
# ament_auto_find_build_dependencies()

find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)

ament_export_dependencies(rosidl_default_runtime)

set(msg_files
"msg/CanFrame.msg"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES std_msgs builtin_interfaces
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  # set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  # set(ament_cmake_cpplint_FOUND TRUE)
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

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/socket_can_common.cpp
  src/socket_can_id.cpp
  src/socket_can_receiver.cpp
  src/socket_can_sender.cpp)

ament_auto_add_library(socket_can_receiver_node SHARED
  src/socket_can_receiver_node.cpp
)

target_link_libraries(socket_can_receiver_node
  ${PROJECT_NAME}
)

rclcpp_components_register_node(socket_can_receiver_node
  PLUGIN "drivers::socketcan::SocketCanReceiverNode"
  EXECUTABLE socket_can_receiver_node_exe
)

ament_auto_add_library(socket_can_sender_node SHARED
  src/socket_can_sender_node.cpp
)

target_link_libraries(socket_can_sender_node
  ${PROJECT_NAME}
)

rclcpp_components_register_node(socket_can_sender_node
  PLUGIN "drivers::socketcan::SocketCanSenderNode"
  EXECUTABLE socket_can_sender_node_exe
)

# add_executable( processor 
#                 src/imu_process.cpp src/manipulate.cpp)
# ament_target_dependencies(processor 
#                           rclcpp std_msgs sensor_msgs serial_imu)
# target_link_libraries(processor "${cpp_typesupport_target}")

# install(TARGETS 
#         processor  
#         DESTINATION lib/${PROJECT_NAME})
# install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_auto_package(INSTALL_TO_SHARE
  launch
)

# ament_package()
