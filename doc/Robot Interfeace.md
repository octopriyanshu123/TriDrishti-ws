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


