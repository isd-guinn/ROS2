## About
ROS2 Humble (Ubuntu 22.04) using Docker on Rasberry Pi 5

## How to Use
### Set-up
- Download [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- Clone the repository
### Run the Docker
- Open Docker Desktop
- Change directory to the cloned repo
- If you don't have a docker builder yet: `docker buildx create --name mybuilder --use`
- `docker buildx build --load --platform linux/amd64 -t <image_name> .`
    > For linux host env: `docker run --privileged -d -it --env="DISPLAY" --env="QT_X11_NO_MITSHM=1" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" <image_name>`
- Check whether the image is successfully built by `docker images`
- `docker run -e DISPLAY=host.docker.internal:0.0 --privileged -d -it <image_name>`

The docker should be able to access to Raspberry Pi GPIO Pins.

To open a new terminal in the same docker container:
- `docker ps` to check the container_id
- `docker exec -it <container_id> bash`

### For Rviz2 and Gazebo
Below steps needs to be done before running the docker.
> Windows 
- install [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
- Set display number as 0 in XLaunch
> Linux (to be tested)
- `xhost local:root` (to enable X11 server)

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
- linux/amd64

Reference for windows GUI setting: https://www.youtube.com/watch?v=qWuudNxFGOQ&t=748s

For tb4: https://turtlebot.github.io/turtlebot4-user-manual/tutorials/navigation.html

For RasPi GPIO pins access: https://stackoverflow.com/questions/30059784/docker-access-to-raspberry-pi-gpio-pins