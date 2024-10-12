## About
ROS2 Humble (Ubuntu 22.04) using Docker on Rasberry Pi 5

## How to Use
### Set-up
- Download [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- Clone the repository
### Run the Docker (Linux on RasPi5)
- Open Docker Desktop / Start Docker Engine
- Change directory to the cloned repo
- If you don't have a docker builder yet: `docker buildx create --name mybuilder --use`
```
docker buildx build --load --platform linux/arm64 -t <image_name> .
docker images #to check whether image is successfully built
xhost +
docker run -it --privileged -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev:/dev -v /sys:/sys -e DISPLAY=:0 <image_name>
```
    > For windows host env: 
    > `docker run -e DISPLAY=host.docker.internal:0.0 --privileged -it <image_name>`
    > To name the container specifically, add `-d --name <container_name>`

The docker should be able to access to Raspberry Pi GPIO Pins with `--privileged` (catch all for all devices on host)
- backup approach: `-v /sys:/sys`
- to test: `cd /sys/class/gpio`

Should be also able to access the usb port with `-v /dev:/dev`. To test: 
```
cd /dev
--> check if there's ttyUSB0 after plugging in the IMU
```

To open a new terminal in the same docker container:
- `docker ps` to check the container_id
- `docker exec -it <container_id> bash`

To save the container as a new image:
- `docker login`
- `docker ps` to check container ID
- `docker commit <container_ID> <hub-user>/<repo-name>:<tag>`
- `docker images` to check whether it is committed to local successfully 
To further push the image to the Docker Hub:
- `docker push <hub-user>/<repo-name>:<tag>`

To rename the container for clarity:
`docker rename <old> <new>`

To check whether the docker is arm64 or amd64: `dpkg --print-architecture`

### For Rviz2 and Gazebo
Below steps needs to be done before running the docker.
> Linux
- `xhost local:root` (to enable X11 server)
> Windows 
- install [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
- Set display number as 0 in XLaunch

### For Nav2
Before running anything using nav2, set key env variables below:
```
export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:/opt/ros/humble/share/turtlebot3_gazebo/models
```
For testing, you can try:
`ros2 launch nav2_bringup tb3_simulation_launch.py headless:=False`

### Exit the Docker
Exit by typing `exit` in the docker terminal.

## Remarks
[Docker CLI Cheat Sheet](https://docs.docker.com/get-started/docker_cheatsheet.pdf)

Rasberry Pi 5's env:
- Ubuntu 24.04
- ROS2 Jazzy
- host platform = linux/arm64/v8

Reference for windows GUI setting: https://www.youtube.com/watch?v=qWuudNxFGOQ&t=748s

For tb4: https://turtlebot.github.io/turtlebot4-user-manual/tutorials/navigation.html

For RasPi GPIO pins access: https://stackoverflow.com/questions/30059784/docker-access-to-raspberry-pi-gpio-pins

For ROS1-ROS2 migration: https://docs.ros.org/en/humble/How-To-Guides/Migrating-from-ROS1.html
https://docs.ros.org/en/humble/How-To-Guides/Migrating-from-ROS1/Migrating-CPP-Packages.html