FROM hinpak/ros2_with_ros2_control:v1

# Clone source code into workspace
WORKDIR /ros2_ws
COPY /src /ros2_ws/src
# Build workspace
RUN colcon build --symlink-install

WORKDIR /
RUN apt-get update
RUN apt-get install -y ros-humble-imu-tools

# source
RUN bash -c "source ~/.bashrc"
RUN . /opt/ros/${ROS_DISTRO}/setup.sh

# Change to the working directory
WORKDIR /ros2_ws

# Run these code after building the image
# WORKDIR /ros2_ws
# RUN colcon build --packages-select serial_test_py
# RUN bash -c "source ~/ros2_ws/install/setup.bash"