## About
This is the ROS2-related source code for a year project related to underwater cleaning robot done by a group of HKUST ISD students. 

- `documentation`: storing (maybe) useful reference
- `src`: storing active packages
- `src_dump`: storing unused/outdated packages

>### Environment
- Rasberry Pi 5 (linux/arm64/v8)
- ROS2 Humble Docker (Ubuntu 22.04)

## Usage
>### Auto-start
```
sudo systemctl <enable/disable> ros2_docker.service
sudo systemctl <start/stop/restart> ros2_docker.service
sudo systemctl status ros2_docker.service
```
(similar for the can0 auto-start service "systemd-networkd" )

>### Launch the ROS2 nodes manually
Before launch, make sure  
1. Current directory is /ros2_ws
2. `source /ros2_ws/install/setup.bash`

```
ros2 launch ros2_socketcan socket_can_receiver.launch.py interface:=can0 interval_sec:=1.0
ros2 launch ros2_socketcan socket_can_sender.launch.py interface:=can0 timeout_sec:=1.0
ros2 launch canbus_slave canbus_slave.launch.py
ros2 launch joystick joystick.launch.py
ros2 launch imu_process imu_processor.launch.py
ros2 launch nav_algo nav_algo.launch.py
```

If the package is not found, the package may not be built yet:
```
colcon build --packages-select <package_name>
```
Package_name is same as the package's folder name.

>### Directly execute a running container
```
docker exec -it <container_id> bash
```

## Maintanance
1. Upload the latest code to a docker container
2. Built all the required packages
3. Save that container as new docker image
4. Update `docker-compose.yml`
5. (if needed) Update `ros2_docker.service` in `/etc/systemd/system`

>### Upload code from host to container
```
docker cp ~/guinn-ROS2/src <container_id>:/ros2_ws
```

>### Save current container as new docker image
```
docker login
docker commit <container_id> <hub-user>/<repo-name>:<tag>
```
- Format of the tag: "status-MMDD-purpose"
    - status: 'base' (core image) or 'built' (packages are already built)
    - MMDD: date
    - purpose: 'maintain' (general) or others (e.g. 'debug' or 'test')

- To push image to Docker Hub:
```
docker push <hub-user>/<repo-name>:<tag>
```
Project-related docker image can be found at `hinpak/ros2` Docker Hub repo.

## Monitor & Debug
>### Monitor the auto-started service:
```
docker attach <container_name or container_id>
```

>### CAN device
If can0 is not bring up, it is likely because either:  
- mcp2515 failed to init
- the can0 interface is down

```
sudo dmesg | grep -i mcp
ip -details -statistics link show can0
sudo ip link set can0 up type can bitrate 100000 loopback off restart-ms 100
```

To send dummy message: `cansend can0 000#00.00` (or use USB-to-CAN)  
To show upcoming received message:
`candump can0`

>### ROS2 topic
```
ros2 topic echo <topic> (--field <message_subtype>)
ros2 topic echo /Imu_euler_angle
ros2 topic echo /Imu_data_can --field angular_velocity
ros2 topic echo /Imu_data_can --field linear_acceleration
```

## Others
>### Start a container
```
docker run -it --privileged -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev:/dev -v /sys:/sys -e DISPLAY=:0 --network=host <image_name>
```
Or if using Windows host:
```
docker run -e DISPLAY=host.docker.internal:0.0 --privileged -it --platform linux/arm64 <image_name>
```

>### Docker CLI Quick Reference
Check container's ID: `docker ps`  
List local docker images: `docker images`  
Exit a docker container: `exit`

If seems stucked, try `sudo systemctl restart docker`

*[More info available in `documentation` folder.]*