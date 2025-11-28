// pio lib install "paulo-raca/Yet Another Arduino PcInt Library"
#include "globals.hpp"
#include <TaskScheduler.h>
#include <SensorSonar.h>
#include <AllMoves.h>
#include <DriveUnit.h>
#include <Queue.h>
#include <GPSInterface.h>
#include <IMUInterface.h>
#include <LineFollower.h>

#include "Serial_mon.h"

unsigned int _distance;

SerialSetup s(115200);
 
SonarQueue SonarData; 

//Scheduler and Tasks
Scheduler TS;
DriveUnit drivingUnit(&TS,WheelUpdateRate);
sSonar sonarA0(&TS,25, &SonarData ,SONARTRIG, SONARECHO);

// GPS and IMU sensors
GPSInterface gps;
IMUInterface imu;

// Line follower controller
LineFollower lineFollower(&TS, &gps, &imu, &drivingUnit); 

void setMainTargetSpeed(movement m) { 
   drivingUnit.setTargetSpeed( m.leftSpeed, m.rightSpeed,m.mSec);
};

AllMovements moves(&TS, setMainTargetSpeed);

void setup() {

   Serial.begin(115200);
   Serial.println("Mower Control System Starting...");

   // Initialize sensors
   gps.begin();
   imu.begin(true);  // true = enable magnetometer for compass heading
   imu.calibrate();  // Calibrate gyro (must be stationary)

   // ===== EXAMPLE 1: Use predefined movement patterns =====
   // Uncomment to use circle pattern
   // moves.setCurrentPattern(CIRCLE);

   // ===== EXAMPLE 2: Use line following =====
   // Define a line from (0,0) to (10,0) - 10 meters straight ahead (INTEGER-ONLY VERSION)
   lineFollower.setLineMeters(0, 0, 10, 0);

   // Configure line follower parameters (optional, defaults are reasonable)
   // Gains are scaled by 1000 (1000 = 1.0, 2000 = 2.0)
   lineFollower.setCrossTrackGain(1000);      // 1.0 - How aggressively to correct cross-track error
   lineFollower.setHeadingGain(2000);         // 2.0 - How aggressively to correct heading error
   lineFollower.setLookaheadDistanceMeters(1);  // Look 1 meter ahead on the line
   lineFollower.setBaseSpeed(Speed50);        // Base forward speed (50%)
   lineFollower.setCompletionThresholdMM(300); // Stop when within 300mm (30cm) of endpoint

   // Set initial position and heading for testing (normally from GPS/IMU)
   gps.setPositionTenthsOfMeters(0, -10);  // Start 1 meter to the left of the line
   imu.setHeadingDegrees(45);      // Facing 45 degrees (Northeast)

   // Enable line follower (starts following the line)
   lineFollower.enable();

   Serial.println("Line follower enabled - following line from (0,0) to (10,0)");
   Serial.println("Starting position: (0, -1), heading: 45 degrees");
};

void loop() {
   // Main task scheduler - handles all periodic tasks
   TS.execute();

   // Update sensors periodically
   static unsigned long lastSensorUpdate = 0;
   if (millis() - lastSensorUpdate > 50) {  // Update at ~20Hz
      gps.update();
      imu.update();
      lastSensorUpdate = millis();
   }

   // Monitor line follower status
   static unsigned long lastStatusPrint = 0;
   if (millis() - lastStatusPrint > 1000) {  // Print every 1 second
      if (lineFollower.isEnabled() && !lineFollower.isComplete()) {
         Serial.print("Line following - CTE: ");
         Serial.print(lineFollower.getCrossTrackError(), 3);
         Serial.print("m, Position: (");
         Serial.print(gps.getPosition().x, 2);
         Serial.print(", ");
         Serial.print(gps.getPosition().y, 2);
         Serial.print("), Heading: ");
         Serial.print(imu.getHeading(), 1);
         Serial.println(" deg");
      } else if (lineFollower.isComplete()) {
         Serial.println("Line following COMPLETE!");
      }
      lastStatusPrint = millis();
   }

/*
   // ===== Optional: Sonar-based obstacle avoidance =====
   // Uncomment when using sonar with line following

   if(!sonarA0.isEnabled()) {
      // Enable sonar when line following
      if (lineFollower.isEnabled()) {
         sonarA0.enableDelayed(2);
      }
   }

   if (SonarData.pull(_distance)) { // We have a measured distance to handle
      if (_distance < sSonar::MMtoMeasure(500)) {
         Serial.println("Obstacle detected - slowing down");
         lineFollower.setBaseSpeed(Speed20);  // Slow down
      }
      else if (_distance < sSonar::MMtoMeasure(150)) {
         // Stop line following and avoid obstacle
         Serial.println("Obstacle too close - stopping");
         lineFollower.disable();
         sonarA0.Stop();
         moves.setCurrentPattern(AVOIDOBSTACLE);
      }
   }
*/
};
