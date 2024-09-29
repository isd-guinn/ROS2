FROM osrf/ros:humble-desktop-full
# Ubuntu 22.04 Jammy Jellyfish
# already installed with base image:
# - rviz2
# - python 3

WORKDIR /

# Update and upgrade existing packages
RUN apt-get update
RUN apt-get upgrade -y
# Install necessary packages
RUN apt-get install -y \
    sudo \
    nano \
    curl \
    git \
    x11-apps \
    ros-dev-tools \
    ros-humble-navigation2 \
    ros-humble-nav2-bringup \
    ros-humble-ros-gz

# Create workspace directory
RUN mkdir -p ~/ros2_ws/src
WORKDIR /ros2_ws
# Clone source code into workspace
COPY /src /ros2_ws/src
# Build workspace
RUN colcon build --symlink-install
# RUN source install/setup.bash # to be tested whether it works or not

# setup colcon_cd
WORKDIR /
RUN echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> ~/.bashrc
RUN echo "export _colcon_cd_root=/opt/ros/humble/" >> ~/.bashrc

# other setup
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN echo "export QT_QPA_PLATFORM=xcb" >> ~/.bashrc

# source the bashrc
RUN bash -c "source ~/.bashrc"

# Change to the working directory
WORKDIR /ros2_ws
