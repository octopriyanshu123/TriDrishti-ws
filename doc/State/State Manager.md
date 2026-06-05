
Save the Status OF the Robot in the Json

```json
{

"lastUpdatedAt": "2026-06-02T18:53:15",

"commandCount": 0,

"connection": {

"state": "FULLY_CONNECTED",



"orinConnected": true,

"ndtConnected": true

},

"runState": "INITIALISED",

"sensors": {

"lidar": { "initialised": true, "bypassed": false, "calibrated": true },

"imu": { "initialised": true, "bypassed": false, "calibrated": true },

"wheel": { "initialised": true, "bypassed": false, "calibrated": true },

"laserProfiling": { "initialised": true, "bypassed": false, "calibrated": true },

"stm": { "initialised": true, "bypassed": false, "calibrated": true },

"allCalibrated": true

},

"ndt": {

"gain": 1.5000,

"voltage": 12.0000,

"filterId": 2

},

"inspection": {

"surface": "PIPE",

"pipeDiaMm": 0.0000,

"probCount": 1,

"rasterLengthMm": 0.0000,

"lengthMm": 0.0000,

"customLengthMm": 0.0000

},

"runtime": {

"running": false,

"frozen": false,

"mode": "MANUAL"

},

"lastError": {

"source": "",

"message": "",

"timestamp": ""

}

}
```


RobotState {

Ideal
Ready
Running
Connectiong
Fault
}

On every Control Command We update the File 
it aslo save every error corrent mode
 