## About
ROS2 Humble (Ubuntu 22.04) using Docker on Rasberry Pi 5
/src: source file for codes

## How to Run Docker
1. clone this repo
2. change directory to the cloned repo
3. `docker build -t ros2 .` (where "ros2" = github repo folder name)
4. `docker run -it 127.0.0.1:3000:3000 getting-started`

To check whether the docker is running, use `docker ps`

### Remarks
Rasberry Pi 5's env:
- Ubuntu 24.04
- ROS2 Jazzy
