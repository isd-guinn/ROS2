## About
ROS2 Humble (Ubuntu 22.04) using Docker on Rasberry Pi 5

## How to Use
### Set-up
- Download [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- Clone the repository
### Run the Docker
- Open Docker Desktop
- Change directory to the cloned repo
- `docker build -t  <image_name> .`
    > For multi-platform:
    > `docker buildx create --name mybuilder --use`
    > `docker buildx build --platform linux/arm64,windows/amd64 .`
- Check whether the image is successfully built by `docker images`
- `docker run -e DISPLAY=host.docker.internal:0.0 -it <image_name>`

### For Rviz2 and Gazebo
Below steps needs to be done before running the docker.
> Windows 
- install [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
- Set display number as 0 in XLaunch
> Linux (to be tested)
- `xhost local:root` (to enable X11 server)

### Exit the Docker
Exit by `exit` or Ctrl+D in the terminal.

## Remarks
[Docker CLI Cheat Sheet](https://docs.docker.com/get-started/docker_cheatsheet.pdf)

Rasberry Pi 5's env:
- Ubuntu 24.04
- ROS2 Jazzy

reference: https://www.youtube.com/watch?v=qWuudNxFGOQ&t=748s
