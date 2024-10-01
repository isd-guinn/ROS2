FROM ros:humble-ros-base-jammy
# osrf/ros:humble-desktop-full DOES NOT WORK FOR linux/arm64
# Ubuntu 22.04 Jammy Jellyfish

WORKDIR /

# install ros2 packages - so that it's same as humble-desktop
RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    ros-humble-desktop=0.10.0-1* \
    && rm -rf /var/lib/apt/lists/*
# Install necessary packages
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y \
    sudo \
    nano \
    curl \
    git \
    x11-apps \
    ros-dev-tools \
    ros-humble-navigation2 \
    ros-humble-nav2-bringup \
    ros-humble-cartographer \
    ros-humble-cartographer-ros \
    ros-humble-dynamixel-sdk \
    ros-humble-turtlebot3-msgs \
    ros-humble-turtlebot3 \
    ros-humble-turtlebot4-desktop \
    ros-humble-turtlebot4-simulator \
    ros-humble-ros-gz

# install gazebo fortress
RUN sudo apt-get install -y lsb-release gnupg
RUN sudo curl https://packages.osrfoundation.org/gazebo.gpg --output /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
RUN sudo apt-get update
RUN sudo apt-get install -y ignition-fortress

SHELL ["/bin/bash", "-c"]

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
RUN echo "export LIBGL_ALWAYS_SOFTWARE=1" >> ~/.bashrc

# source the bashrc
RUN bash -c "source ~/.bashrc"

# Change to the working directory
WORKDIR /ros2_ws
