# what action to return when entered the go_to function?
# how to pass the action right out from here?

import math
import time
import mapping

# Create an instance of the Map class
#map_instance = mapping.Map()

# Global variables representing sensor data
R_ang = 0
L_ang = 0
acel = 0
x = 1
y = 1
direction = 0
wall_flag = False
last_wall_is_r = False
first_loop_done = False

STOP = 0
FORWARD = 1
BACKWARD = 2
ANTICLOCKWISE = 3
CLOCKWISE = 4

def read_sensors(map_instance):
    global R_ang, L_ang, acel  # Declare all globals that will be modified

    # Read sensor data
    try:
        R_ang = int(input("Enter right angle (R_ang): "))
        L_ang = int(input("Enter left angle (L_ang): "))
        acel = int(input("Enter acceleration (acel): "))
        #x = int(input("Enter global x-coordinate: "))
        #y = int(input("Enter global y-coordinate: "))
    except ValueError:
        print("Invalid input. Defaulting to 0 for all.")
        R_ang = L_ang = acel = 0
    #print("x is now: ", x, "    y is now: ", y)
    return x, y, direction

def go_straight():
    global x, y
    new_x, new_y = calculate_coordinates(x, y, "front", direction)
    print("Going straight")
    x, y = new_x, new_y
    return 

def go_right():
    global x, y, direction
    new_x, new_y = calculate_coordinates(x, y, "right", direction)
    print("Going right")
    x, y = new_x, new_y
    direction = (direction + 90)%360

def go_left():
    global x, y, direction
    new_x, new_y = calculate_coordinates(x, y, "left", direction)
    print("Going left")
    x, y = new_x, new_y
    direction = (direction + 360 - 90)%360

def calculate_coordinates(x, y, direction, car_direction):
    # Implementation remains unchanged
    #print("In calculate coordinate, car_direction: ", car_direction)
    #print("In calculate coordinate, direction: ", direction)
    #print("In calculate_coordinate: ", x,", ", y)
    new_x, new_y = x, y
    if car_direction == 0:
        if(direction == "front"):
            new_x = x 
            new_y = y - 1
        if(direction == "left"):
            new_x = x - 1
            new_y = y 
        if(direction == "right"):
            new_x = x + 1
            new_y = y

    elif car_direction == 90:
        if(direction == "front"):
            new_x = x + 1
            new_y = y 
        if(direction == "left"):
            new_x = x 
            new_y = y - 1
        if(direction == "right"):
            new_x = x 
            new_y = y + 1

    elif car_direction == 180:
        if(direction == "front"):
            new_x = x 
            new_y = y + 1
        if(direction == "left"):
            new_x = x + 1
            new_y = y 
        if(direction == "right"):
            new_x = x - 1
            new_y = y

    elif car_direction == 270:
        if(direction == "front"):
            new_x = x - 1
            new_y = y 
        if(direction == "left"):
            new_x = x
            new_y = y + 1 
        if(direction == "right"):
            new_x = x 
            new_y = y - 1
    else:
        raise ValueError("Invalid move_direction. Use 'front', 'back', 'left', or 'right'.")

    #print("Returning ", new_x, ", ", new_y)
    return new_x, new_y

def mark_cells(map_instance, x, y):
    """Mark cells based on current sensor readings."""
    global wall_flag, last_wall_is_r

    map_instance.update_map(x, y, 'floor')
    if acel == 0: # sth in front
        front_x, front_y = calculate_coordinates(x, y, "front", direction)
        map_instance.update_map(front_x, front_y, 'wall')
        #print("Marked ", front_x, ",", front_y, "as wall")
    
    if L_ang >= 100: # sth on the left
        left_x, left_y = calculate_coordinates(x, y, "left", direction)
        map_instance.update_map(left_x, left_y, 'wall')
        #print("Marked ", left_x, ",", left_y, "as wall")
        wall_flag = True
        last_wall_is_r = False

    if R_ang >= 100: # sth on the right
        right_x, right_y = calculate_coordinates(x, y, "right", direction)
        map_instance.update_map(right_x, right_y, 'wall')
        #print("Marked ", right_x, ",", right_y, "as wall")
        wall_flag = True
        last_wall_is_r = True

    if acel > 0: # moving -> front clear
        front_x, front_y = calculate_coordinates(x, y, "front", direction)
        map_instance.update_map(front_x, front_y, 'floor')
        #print("Marked ", front_x, ",", front_y, "as floor")

    if L_ang < 90: # left clear
        left_x, left_y = calculate_coordinates(x, y, "left", direction)
        map_instance.update_map(left_x, left_y, 'floor')
        #print("Marked ", left_x, ",", left_y, "as floor")

    if R_ang < 90: # right clear
        right_x, right_y = calculate_coordinates(x, y, "right", direction)
        map_instance.update_map(right_x, right_y, 'floor')
        #print("Marked ", right_x, ",", right_y, "as floor")

def is_undiscovered(map_instance, x, y): # (front, left, right) bool for undiscovered
    """Check for walls in front, left, and right of the robot, including all cells in that direction."""
    # print("In have wall: ", x, ", ", y)
    # Define direction offsets for checking
    offsets = {
        0: {
            'front': (0, -1),
            'left': (-1, 0),
            'right': (1, 0)
        },
        90: {
            'front': (1, 0),
            'left': (0, -1),
            'right': (0, 1)
        },
        180: {
            'front': (0, 1),
            'left': (1, 0),
            'right': (-1, 0)
        },
        270: {
            'front': (-1, 0),
            'left': (0, 1),
            'right': (0, -1)
        }
    }

    discoverable = {}
    
    for position in ['front', 'left', 'right']:
        dx, dy = offsets[direction][position]
        current_x, current_y = x + dx, y + dy
        
        # Check all cells in that direction
        while 0 <= current_x < len(map_instance.map[0]) and 0 <= current_y < len(map_instance.map):
            # WIthin the map boundary
            #print("Checking ", current_y, ", ", current_x, " which is ", map_instance.map[current_y][current_x])
            #print("T/F? ", map_instance.map[current_y][current_x].status == 'w')
            if map_instance.map[current_y][current_x].status == 'u':
                discoverable[position] = True
                break
            if map_instance.map[current_y][current_x].status == 'w':
                #print("Wall found in ", position)
                discoverable[position] = False  # Wall found
                break
            current_x += dx
            current_y += dy

            discoverable[position] = True  # No walls found in that direction

    return discoverable

def find_nearest_undiscovered_grid(map_instance):
    """Find the coordinates of the nearest undiscovered grid ('u') from the current position."""
    global x, y
    
    print("Finding nearest undiscovered")
    # Start checking layers from distance 1 outward
    distance = 1
    
    while True:
        # Check all positions at the current distance
        for dx in range(-distance, distance + 1):
            for dy in range(-distance, distance + 1):
                if abs(dx) + abs(dy) == distance:  # To keep within the diamond shape
                    check_x = x + dx
                    check_y = y + dy
                    
                    # Check bounds
                    if 0 <= check_x < len(map_instance.map[0]) and 0 <= check_y < len(map_instance.map):
                        if map_instance.map[check_y][check_x].status == 'u':
                            return check_x, check_y  # Return the coordinates of the first undiscovered grid found

        # Increment distance for the next layer
        distance += 1
        
        # Check if we've reached the boundaries of the map
        if distance > max(len(map_instance.map), len(map_instance.map[0])):
            break  # Stop if we've exceeded the map boundaries

    return None  # If no undiscovered grid found

def is_closed_wall(map_instance, wall_x, wall_y):
    """Check if the wall loop is closed at the given coordinates."""
    
    # Define the directions to check (8 neighboring cells)
    directions = [
        (-1, -1), (-1, 0), (-1, 1),  # Top-left, Top, Top-right
        (0, -1),            (0, 1),   # Left, Right
        (1, -1), (1, 0), (1, 1)       # Bottom-left, Bottom, Bottom-right
    ]
    
    # Count adjacent walls
    wall_neighbor_count = 0
    
    # Check each direction
    for dx, dy in directions:
        check_x = wall_x + dx
        check_y = wall_y + dy
        
        # Check bounds before accessing the neighbor
        if 0 <= check_x < len(map_instance.map[0]) and 0 <= check_y < len(map_instance.map):
            if map_instance.map[check_y][check_x].status == 'w':
                wall_neighbor_count += 1

    # A wall is considered closed if it has 8 neighboring walls (or fewer depending on its position)
    # Here, we can adjust the threshold based on the wall's position in the grid
    if wall_neighbor_count >= 2:  # Adjust the threshold as needed
        return True
    else:
        return False

def find_nearest_unclosed_wall(map_instance, robot_x, robot_y):
    """Find the nearest unclosed wall from the robot's location."""
    
    # Check each grid cell in the surrounding area
    for j in range(max(0, robot_y - 1), min(len(map_instance.map), robot_y + 2)):
        for i in range(max(0, robot_x - 1), min(len(map_instance.map[0]), robot_x + 2)):
            # Check if the current cell is a wall
            if map_instance.map[j][i].status == 'w':
                # Use the is_closed_wall function to check if this wall is closed
                if not is_closed_wall(map_instance, i, j):
                    return (i, j)  # Return the coordinates of the unclosed wall

    return None  # Return None if no unclosed wall is found

def follow_wall(map_instance):
    global wall_flag
    # Robot should be still touching the wall right now
    # Get wall coordinates
    if last_wall_is_r:
        wall_x, wall_y = calculate_coordinates(x, y, "right", direction)
    else:
        wall_x, wall_y = calculate_coordinates(x, y, "left", direction)
    while not is_closed_wall(map_instance, wall_x, wall_y):
        if acel == 0:
            if L_ang > 90:
                go_right()
            elif R_ang > 90:
                go_left()
        elif acel > 0:
            # When lose touch with a wall
            if L_ang == 45 and R_ang == 45: 
                print("Finding wall")
                if last_wall_is_r:
                    go_right()
                else:
                    go_left()
            else: 
                go_straight()
        map_instance.print_map(x, y, direction)
        read_sensors(map_instance)
        mark_cells(map_instance, x, y)

    wall_flag = False
    
def go_to(map_instance, target_x, target_y):
    global x, y, direction
    """Navigate to a specific coordinate (target_x, target_y)."""
    print(f"Going to ({target_x}, {target_y})")
    
    while (x != target_x) or (y != target_y):
        # Move in the x-direction
        if x < target_x:  # Move right
            direction = 90
            for i in range( x + 1, target_x + 1):
                if map_instance.map[y][i].status == 'w':
                    break  # Stop if a wall is encountered
                if map_instance.map[y][i].status == 'u':
                    read_sensors(map_instance)
                    mark_cells(map_instance, x, y)
                x = i  # Move to the next position
                map_instance.print_map(x, y, direction)
            
        elif x > target_x:  # Move left
            direction = 270  
            for i in range(x - 1, target_x - 1, -1):
                if map_instance.map[y][i].status == 'w':
                    break
                if map_instance.map[y][i].status == 'u':
                    read_sensors(map_instance)
                    mark_cells(map_instance, x, y)
                x = i
                map_instance.print_map(x, y, direction)
            
        
        # Move in the y-direction
        if y < target_y:  # Move down
            for j in range(y + 1, target_y + 1):
                direction = 180
                if map_instance.map[j][x].status == 'w':
                    break 
                if map_instance.map[y][i].status == 'u':
                    read_sensors(map_instance)
                    mark_cells(map_instance, x, y)
                y = j  # Move to the next position
                map_instance.print_map(x, y, direction)
        elif y > target_y:  # Move up
            for j in range(y - 1, target_y - 1, -1):
                direction = 0
                if map_instance.map[j][x].status == 'w':
                    break
                if map_instance.map[y][i].status == 'u':
                    read_sensors(map_instance)
                    mark_cells(map_instance, x, y)
                y = j
                map_instance.print_map(x, y, direction)
        print("Going now: ", x, ", ", y)

    return target_x, target_y

def navigate(map_instance):
    global x, y, first_loop_done, action

    """Main navigation function to decide actions based on sensor data."""
    
    mark_cells(map_instance, x, y) # mark wall/ floor
    
    
    if wall_flag:
        follow_wall(map_instance)
        # wall_flag back to False
        mapping.convert_corners_to_walls(map_instance)

    # #front_x, front_y = calculate_coordinates(x, y, "front", direction)
    # if first_loop_done: # Done first loop will start going inside
    #     if map_instance.map[x][y].status != 'u':
    #         target_x, target_y = find_nearest_undiscovered_grid(map_instance)
    #         go_to(map_instance, target_x, target_y)


    undiscovered = is_undiscovered(map_instance, x, y) # check grid around
    print(undiscovered) 
    if undiscovered["front"]:
        go_straight()
        action = FORWARD
        return action
    elif undiscovered["left"]:
        go_left()
        action = ANTICLOCKWISE
        return action
    elif undiscovered["right"]:
        go_right()
        action = CLOCKWISE
        return action
    else:
        print("Navigating to nearest undiscovered areas...")
        nearest = find_nearest_undiscovered_grid(map_instance)
        if nearest:
            target_x, target_y = nearest
            print(f"Going to nearest undiscovered grid at ({target_x}, {target_y}).")
            x, y = go_to(map_instance, target_x, target_y)
        else:
            print("No more undiscovered area")
    #print("Navigation returning: ", x, ", ", y)
    map_instance.print_map(x, y, direction)
    return x, y, direction  # Return updated coordinates