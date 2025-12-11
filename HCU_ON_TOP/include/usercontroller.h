#pragma once
#include "main.h"
#include "mechanicalfunction.h"
#include "autonomousfunction.h"

inline bool wingFlag = true;
inline bool parkFlag = true;
inline bool parkclampFlag = true;
inline bool colorFlag = true;

inline void usercontrol() {
  startIMURotationTask();
  chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);

  while (true) {
    chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);
    // ===== DRIVE (unchanged) =====
    int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
    int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
    chassis.arcade(leftY, rightX);

    // ===== SCORING MODE SELECTION =====
    // Pick one mode per loop depending on which button is pressed.
    // Priority order: L1 > L2 > R1 > R2, then NONE when nothing held.

    if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
      // Long goal
      setScoringMode("TOP");
    } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
      // Mid goal
      setScoringMode("MIDDLE");
    } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
      // Intake
      setScoringMode("INTAKE");
    } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
      // Outtake
      setScoringMode("OUTTAKE");
    } else {
      // No scoring buttons held
      setScoringMode("NONE");
    }

    // ===== SCRAPER (momentary) =====
    // Y: scraper
    bool scraperState = controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y);
    scraper.set_value(scraperState);

    // ===== WING / PARK / PARKCLAMP TOGGLES (unchanged) =====
    if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) && wingFlag) {
      wing_tog();
      wingFlag = false;
    } else if (!controller.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) &&
               !wingFlag) {
      wingFlag = true;
    }

    if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B) && parkFlag) {
      park_tog();
      parkFlag = false;
    } else if (!controller.get_digital(pros::E_CONTROLLER_DIGITAL_B) &&
               !parkFlag) {
      parkFlag = true;
    }

    if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) &&
        parkclampFlag) {
      parkclamp_tog();
      parkclampFlag = false;
    } else if (!controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) &&
               !parkclampFlag) {
      parkclampFlag = true;
    }

    pros::delay(10);
  }
}
