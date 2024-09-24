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
    code \
    x11-apps \
    ros-dev-tools \
    ros-humble-navigation2 \
    ros-humble-nav2-bringup \
    ros-humble-ros-gz

ENV SHELL /bin/bash
# ENV LANG en_US.UTF-8

RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN source ~/.bashrc

# Create workspace directory
RUN mkdir -p /ws/src
COPY /src /ws/src

# Set the working directory
WORKDIR /ws

CMD ["/bin/bash"]