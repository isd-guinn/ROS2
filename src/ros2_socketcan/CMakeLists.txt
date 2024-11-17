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

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# find_package(rclcpp REQUIRED)
# find_package(rclcpp_components REQUIRED)
# find_package(rclcpp_lifecycle REQUIRED)
# find_package(lifecycle_msgs REQUIRED)
# find_package(can_msgs REQUIRED)
find_package(ros2_socketcan_msgs REQUIRED)
# find_package(ament_cmake REQUIRED)

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

# create a shared library named after the PROJECT_NAME
ament_auto_add_library(${PROJECT_NAME} SHARED
  src/socket_can_common.cpp
  src/socket_can_id.cpp
  src/socket_can_receiver.cpp
  src/socket_can_sender.cpp)

# shared library named socket_can_receiver_node
ament_auto_add_library(socket_can_receiver_node SHARED
  src/socket_can_receiver_node.cpp
)
# link so that the shared library "socket_can_receiver_node" can use the shared library "PROJECT_NAME"
target_link_libraries(socket_can_receiver_node
  ${PROJECT_NAME}
)
# Register an rclcpp component with the ament resource index and create an executable. 
# socket_can_receiver_node = the name of the shared library
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

ament_auto_package(INSTALL_TO_SHARE
  launch
)
