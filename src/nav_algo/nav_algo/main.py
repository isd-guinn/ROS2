# done integration for this file

# main.py
import time
from . import mapping
from . import navigation 
import math

# Global variables representing sensor data
R_ang = navigation.R_ang
L_ang = navigation.L_ang
# L_ang = 0
# R_ang = 0
acel = navigation.acel
direction = navigation.direction
x  = navigation.x
y = navigation.y

def run_nav_algo(foc_left, foc_right, is_moving, arg=None):
     
    try:
        map_instance = mapping.Map()
        #x, y= 1, 1  # Initial coordinates
        
        action = 0  # Action to be returned
        L_ang = foc_left
        R_ang = foc_right
        
        while is_moving == True:
            x, y, direction = navigation.read_sensors(map_instance)  # Update global sensor values
            #print("x, y after reading sensors: ", x, ", ", y)
            x, y, direction, action = navigation.navigate(map_instance)  # Call navigation function
            print("x, y after navigation: ", x, ", ", y)
            print("action: ", action)
            time.sleep(1)  # Control loop frequency
    except KeyboardInterrupt:
        print("Exiting program.")
    finally:
        # return action
        return action
        # Clean up resources if needed
        pass