## About
ROS2 Humble (Ubuntu 22.04) using Docker on Rasberry Pi 5

## How to Use
### Set-up
- Download Docker Desktop
- Clone the repository
### Run the Docker
1. Open Docker Desktop
2. Change directory to the cloned repo
3. `docker build -t <image_name> .` 
4. Check whether the image is successfully built by `docker images`
5. `docker run -it <image_name>`
6. Check whether the docker is running by `docker ps` in another terminal
### Exit the Docker
Exit by `exit` or Ctrl+D in the terminal.

## Remarks
[Docker CLI Cheat Sheet](https://docs.docker.com/get-started/docker_cheatsheet.pdf)

Rasberry Pi 5's env:
- Ubuntu 24.04
- ROS2 Jazzy
