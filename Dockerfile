#ver: newtest:v1
FROM hinpak/ros2_with_ros2_control:v1

# Clone source code into workspace
WORKDIR /ros2_ws
COPY /src /ros2_ws/src
# Build workspace
RUN apt-get update
RUN apt-get install -y ament-cmake
RUN apt-get install -y ros-humble-imu-tools
# RUN apt-get install -y python3-colcon-common-extensions
# install dependency
RUN rosdep install --from-paths src --ignore-src -r -y
RUN bash -c "source /opt/ros/humble/setup.bash" && \
    colcon build --symlink-install

# source
RUN bash -c "source ~/.bashrc"
RUN . /opt/ros/${ROS_DISTRO}/setup.sh

# Change to the working directory
WORKDIR /ros2_ws

# Run these code after building the image
# WORKDIR /ros2_ws
# RUN colcon build --packages-select <package_name>
# RUN bash -c "source ~/ros2_ws/install/setup.bash"

##################
# Log
##################
# for "Findament_cmake.cmake" error, install ament-cmake
