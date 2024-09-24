FROM osrf/ros:humble-desktop-full
# Ubuntu 22.04 Jammy Jellyfish
# already installed with base image:
# - rviz2
# - python 3

# Update and upgrade existing packages
RUN apt-get update && apt-get upgrade -y

# Install necessary packages
RUN apt-get update && apt-get install -y \
    sudo \
    curl \
    git \
    x11-apps \
    ros-dev-tools \
    ros-humble-navigation2 \
    ros-humble-nav2-bringup \
    ros-humble-ros-gz

# Install VSCode

ENV SHELL=/bin/bash
# ENV LANG=en_US.UTF-8
# -------------------------------------------------------
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
# Create workspace directory
RUN mkdir -p /catkin_ws/src
# Clone source code into workspace
COPY /src /catkin_ws/src
# Build workspace
RUN /bin/bash -c '. /opt/ros/humble/setup.bash; cd /catkin_ws; catkin_make; cd'
RUN echo "source /catkin_ws/devel/setup.bash" >> ~/.bashrc
RUN echo "export QT_QPA_PLATFORM=xcb" >> ~/.bashrc
RUN source /.bashrc

# Set the working directory
WORKDIR /catkin_ws