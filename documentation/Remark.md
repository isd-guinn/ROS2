Here listed some common issues for Docker and other ROS2 packages.

## Docker in General
[Docker CLI Cheat Sheet](https://docs.docker.com/get-started/docker_cheatsheet.pdf)

To build the docker image from dockerfile:
1. Create a Docker Builder: `docker buildx create --name mybuilder --use`
2. `docker buildx build --load --platform linux/arm64 -t <preferred_image_name> .`

To check docker's architecture: `dpkg --print-architecture`  
To name the container when using `docker run`: add `-d --name <container_name>`  
To rename container: `docker rename <old_container_name> <new_container_name>`

## Hardware Access of Docker
When running the docker, tags can be used to bind the hardware (Raspberry Pi 5 in this case).  
- RasPi GPIO pins (all): `--privileged`  
- USB ports: `-v /dev:/dev`  
    - If this does not work, try also add `-v /sys:/sys`

To verify, check directories such as /sys/class/gpio or /dev in the docker container.

> ### List of paths for RasPi5 devices:
- Using USB port: /dev/ttyUSB* or /dev/ttyACM*
- Using UART: /dev/ttyAMA0 (GPIO) or /dev/ttyAMA10 (onboard UART)
- Using CAN: /sys/bus/spi/devices/spi0.0/net/can*

> ### Enable GPIO UART on RasPi 5
Reference: https://stackoverflow.com/questions/30059784/docker-access-to-raspberry-pi-gpio-pins

1. Check /boot/config.txt and /boot/cmdline.txt using `sudo nano <file_path>`
2. Enter configuration using `raspi-config` and enable serial port, then reboot 
3. Check config.txt which should now have `dtparam=uart0=on`, and ttyAMA0 should show up in /dev

> ### GUI Display (for packages e.g. Rviz2 and Gazebo)
Follow the steps below before running the docker container.
- Linux
    1. Setup X11 
    2. `xhost local:root` (to enable X11 server)
    3. Enable xhost in the corresponding terminal using `xhost +`
- Windows ([Reference](https://www.youtube.com/watch?v=qWuudNxFGOQ&t=748s))
    1. Install [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
    2. Set display number as 0 in XLaunch

## Nav2
Set key environment variables using command below before running nav2:  
`export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/humble/share/turtlebot3_gazebo/models`  
If successful, below command should run smoothly:  
`ros2 launch nav2_bringup tb3_simulation_launch.py headless:=False`

## SSH to RasPi 5
1. Check RasPi 5's IP address: `hostname -I` (should be the 10.89.xxx.xxx one)
2. At RasPi 5, run `sudo service ssh start`
3. At host machine, run `ssh <username>@<ip_address>`