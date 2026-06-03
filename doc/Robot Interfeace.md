Api For the Robot

Type of API 
Set
Get
## Connection Page
---

robot.connectOrinNX()
robot.connectNDT()

## Robot Setting
---
robot.init_lidar();
robot.init_imu();
robot.init_wheel();
robot.init_laser_profing();
robot.init_stm();

robot.reset_lidar();
robot.reset_imu();
robot.reset_wheel();
robot.reset_laser_profing();
robot.reset_stm();

robot.bypass_lidar();
robot.bypass_imu();
robot.bypass_wheel();
robot.bypass_laser_profing();
robot.bypass_stm();

robot.sensor_calibrate();



### NDT Setting


### Inspection Setting

robot.setInspectionSurface();
robot.setInspectionPipeDia():
robot.setProbCount();
robot.setRasterLength()
robot.setInspectionLength();
robot.setInspectionCustomLength();

### LiveNDT (Calibration)

robot.Ndtgainincrement():
robot.NdtgainDecrement():
robot.NDTFilterId();
robot.NDTVotage();

## Inspection Page

robot.btFreez()
robot.btResume()
robot.btStart();
robot.btAbout():
robot.inspectionToggleOperatioMode()



# Operator Point of View

Step 0 

git clone Trinetra-ws
cd Trinetra-ws
mkdir build 
cd build
cmake ..
make


## Connection
On the Robot it run One Program name ./robot  pid 1
1.  Make the connection with ui until every operation is lock
2. Start Watch Dog
3. If Connection Break Stop every Operation and Put the Robot in the  Connection Stage - > Reset State -> Ready State Lock the Operation
4. Wait For the Connection
## Configure 

==*Teleop*==
Power On L Motor 
Power On R Motor
Power On Stepper Motor
Power On Linear Actuator
Power On Proximity Sensor

Raster Length
Operation Length
Prob Count

Enable the Joy

==Behaviour Tree==

Power On L Motor 
Power On R Motor
Power On Stepper Motor
Power On Linear Actuator
Power On Proximity Sensor

Raster Length
Operation Length
Prob Count

BotLinearDistace

## Operate

==*Teleop

Just Move the Bot Using the Joysick

==Behaviour Tree==

Start
Freeze
Resume
Abort
Emergency Stop

## Shutdown

Kill all the Pid Created
Every Sensor Poweoff
Close the Program
