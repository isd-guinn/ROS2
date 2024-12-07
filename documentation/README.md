## Documentations & References
List of refences:
- ros2_docker.service
- 80-can.network
- CH105M IMU/VRU/AHRS Module Specsheet (in traditional and simplified Chinese)
- Raspberry Pi 5 Pinout

`ros2_docker.service` is used for auto-starting all the ROS2 nodes in Raspberry Pi 5 once it is powered up, and is located at `/etc/systemd/system`. 

`80-can.network` is used for auto-setting the can0 interface when at boot. It is located at `etc/systemd/network`.
- To update the changes: 
    ```
    sudo systemctl restart systemd-networkd
    ```
- To enable: 
    ```
    sudo systemctl enable systemd-networkd
    ```