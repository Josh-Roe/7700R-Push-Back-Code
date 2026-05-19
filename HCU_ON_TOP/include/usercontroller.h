#pragma once
#include "main.h"
#include "mechanicalfunction.h"
#include "autonomousfunction.h"

inline bool wingFlag = true;
inline bool midDescoreFlag = true;

inline void usercontrol()
{
    startFixPoseTask();
    start_scoring_task();
    chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);
    preroller_tog();

    bool lastR1 = false;

    while (true)
    {
        chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);

        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        chassis.arcade(leftY, rightX);

        bool r1Now = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
        bool l1Now = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1);
        bool l2Now = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);
        bool r2Now = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);

        if (l1Now)
        {
            setScoringMode("TOP");
        }
        else if (l2Now)
        {
            setScoringMode("MIDDLE");
        }
        else if (r1Now)
        {
            setScoringMode("INTAKE");
        }
        else if (r2Now)
        {
            setScoringMode("OUTTAKE");
        }
        else
        {
            setScoringMode("NONE");
        }

        // Reset intake top roller latch when R1 is released
        if (lastR1 && !r1Now)
        {
            notifyIntakeReleased();
        }
        lastR1 = r1Now;

        bool scraperState = controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y);
        scraper.set_value(scraperState);

        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) && wingFlag)
        {
            wing_tog();
            wingFlag = false;
        }
        else if (!controller.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) && !wingFlag)
        {
            wingFlag = true;
        }

        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) && midDescoreFlag)
        {
            preroller_tog();
            midDescoreFlag = false;
        }
        else if (!controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) && !midDescoreFlag)
        {
            midDescoreFlag = true;
        }

        pros::delay(10);
    }
}