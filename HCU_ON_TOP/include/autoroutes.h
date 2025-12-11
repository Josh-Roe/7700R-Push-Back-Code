#pragma once

#include "autonomousfunction.h"
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "main.h"
#include "mechanicalfunction.h"

inline void leftQualsAuto() {
  pros::lcd::print(6, "Running Left Quals Auto");
  chassis.setPose({47, -15.5, -90});
  startIMURotationTask();
    //LEFT QUALS AUTO GOES HERE
  chassis.moveToPoint(23, -23, 1200);
  setScoringMode("INTAKE");
  pros::delay(500);
  scraper_tog();
  chassis.turnToHeading(-90, 600);
  chassis.moveToPoint(8,-46, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(12, -19, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, 1, 700, {.forwards = false});
  pros::delay(650);
  moveDistance(-6, 1200);
  setScoringMode("OUTTAKE");
  pros::delay(400);
  setScoringMode("MIDDLE");
  pros::delay(1100);
  setScoringMode("NONE");
  chassis.moveToPoint(48, -48, 1700, {.maxSpeed = 80});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  moveDistance(100000, 1700, 40);
  chassis.moveToPoint(30, -48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1500, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, -59.5, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(13, -59.5, 800, {.forwards= false});
  chassis.moveToPoint(13, -59.5, 10000, {.forwards = false});
  pros::delay(1000000);
}
inline void rightQualsAuto() {
  pros::lcd::print(6, "Running Right Quals Auto");
  chassis.setPose({50, 20.5, -90});
  startIMURotationTask();
    //RIGHT QUALS AUTO GOES HERE
  chassis.moveToPoint(23, 26, 1200);
  setScoringMode("INTAKE");
  pros::delay(530);
  scraper_tog();
  chassis.turnToHeading(-90, 600);
  chassis.moveToPoint(8,46, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(17, 21, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, -2, 700);
  scraper_tog();
  pros::delay(650);
  moveDistance(6, 1200);
  setScoringMode("OUTTAKE");
  pros::delay(1100);
  setScoringMode("NONE");
  chassis.moveToPoint(48, 46, 1700, {.forwards = false, .maxSpeed = 80});
  chassis.turnToHeading(90, 600);
  setScoringMode("INTAKE");
  pros::delay(200);
  scraper_tog();
  moveDistance(100000, 1700, 40);
  chassis.moveToPoint(30, 49, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 49, 1500, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, 36.5, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToPoint(13, 36.5, 800, {.forwards= false});
  chassis.moveToPoint(13, 36.5, 10000, {.forwards = false});
  pros::delay(1000000);
}
inline void fullWPAuto() {
  pros::lcd::print(6, "Running Full WP Auto");
  chassis.setPose({50, 17.5, 0});
    startIMURotationTask();
  // Full WP AUTO GOES HERE
  RamseteToPoint(50, 47, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  moveDistance(100000, 1300, 40);
  chassis.moveToPoint(30, 48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.setPose({30, 48, chassis.getPose().theta});
  setScoringMode("INTAKE");
  moveDistance(8, 1000);
  chassis.turnToPoint(24,24,800);
  chassis.moveToPoint(24, 24, 1400);
  pros::delay(500);
  scraper_tog();
  chassis.moveToPoint(23, -29, 2000);
  pros::delay(300);
  scraper_tog();
  pros::delay(500);
  scraper_tog();
  chassis.moveToPoint(14, -13, 1300, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, 0, 600, {.forwards = false});
  setScoringMode("OUTTAKE");
  pros::delay(400);
  setScoringMode("MIDDLE");
  pros::delay(1300);
  setScoringMode("NONE");
  chassis.moveToPoint(50, -47, 1500, {.maxSpeed = 90});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  moveDistance(100000, 1600, 40);
  chassis.moveToPoint(30, -47, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -47, 1000, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1000000);
}
inline void leftElimsAuto() {
  pros::lcd::print(6, "Running Left Elims Auto");
  chassis.setPose({47, -15.5, -90});
  startIMURotationTask();
    //LEFT ELIMS AUTO GOES HERE
  chassis.moveToPoint(23, -23, 1200);
  setScoringMode("INTAKE");
  pros::delay(500);
  scraper_tog();
  chassis.turnToHeading(-90, 600);
  chassis.moveToPoint(8,-46, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(22, -30, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(48, -48.5, 700);
  chassis.moveToPoint(48, -48.5, 1700, {.maxSpeed = 80});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  moveDistance(100000, 1800, 40);
  chassis.moveToPoint(30, -49, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1500, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, -60, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(13, -60, 800, {.forwards= false});
  chassis.moveToPoint(13, -60, 10000, {.forwards = false});
  pros::delay(1000000);
}

inline void rightElimsAuto() {
  pros::lcd::print(6, "Running Right Elims Auto");
  chassis.setPose({50, 18.5, -90});
  startIMURotationTask();
    //RIGHT ELIMS AUTO GOES HERE
  chassis.moveToPoint(23, 26, 1200);
  setScoringMode("INTAKE");
  pros::delay(530);
  scraper_tog();
  chassis.turnToHeading(-90, 600);
  chassis.moveToPoint(8,46, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(32, 31, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(48, 46, 700);
  chassis.moveToPoint(48, 47, 1700, {.maxSpeed = 80});
  chassis.turnToHeading(90, 600);
  setScoringMode("INTAKE");
  moveDistance(100000, 1800, 40);
  chassis.moveToPoint(30, 49, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 49, 1500, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, 36.5, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToPoint(13, 36.5, 800, {.forwards= false});
  chassis.moveToPoint(13, 36.5, 10000, {.forwards = false});
  pros::delay(1000000);
}

inline void tuningAuto() {
  pros::lcd::print(6, "Running Tuning Auto");
  chassis.setPose({0, 0, 0});
  //TUNING AUTO GOES HERE
  RamseteToPoint(-24,-24, 10000);
  //chassis.moveToPoint(24, 24, 20000);
  pros::delay(1000000);
}

inline void skillsAuto() {
  pros::lcd::print(6, "Running Skills Auto");
  chassis.setPose({-51, -13, -180});
  // SKILLS AUTO GOES HERE

  wing_tog();
  chassis.moveToPoint(-51 , -42, 1400);
  chassis.turnToPoint(-63, -48, 1000);
  pros::delay(200);
  setScoringMode("INTAKE");
  scraper_tog();
  chassis.moveToPoint(-60.5, -48, 1000, {.maxSpeed = 40});
  chassis.moveToPoint(-80, -48, 1500, { .maxSpeed = 8});
  chassis.moveToPoint(-36, -64, 1400, {.forwards = false, .maxSpeed = 60});
  scraper_tog();
  setScoringMode("NONE");
  chassis.turnToPoint(22, -55, 1000);
  chassis.moveToPoint(22, -55, 2000);

  chassis.moveToPoint(37 , -45, 1400);
  chassis.turnToPoint(20, -40, 1000,{.forwards = false});
  chassis.moveToPoint(20,-40, 1200, {.forwards = false, .minSpeed = 70});
  pros::delay(700);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -40, 2000, {.forwards = false, .maxSpeed = 20});
  chassis.moveToPoint(61, -48, 2500, {.maxSpeed = 40});
  chassis.setPose({31, -50, chassis.getPose().theta});
  scraper_tog();
  setScoringMode("INTAKE");
  chassis.moveToPoint(80, -48, 1500, { .maxSpeed = 8});
  chassis.moveToPoint(20, -50, 1200, {.forwards = false, .minSpeed = 70});
  pros::delay(700);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -50, 2000, {.forwards = false, .maxSpeed = 20});
  scraper_tog();
  chassis.setPose({31, -51, chassis.getPose().theta});
  chassis.moveToPoint(44, 42, 3000, {.maxSpeed = 90});
  chassis.turnToPoint(62, 48, 1000);
  pros::delay(200);
  setScoringMode("INTAKE");
  scraper_tog();
  chassis.moveToPoint(62, 48, 2700, {.maxSpeed = 40});
  pros::delay(2700);
  chassis.moveToPoint(36, 64, 1500, {.forwards = false, .maxSpeed = 80});
  scraper_tog();
  setScoringMode("NONE");
  chassis.turnToPoint(-22, 53, 1000);
  chassis.moveToPoint(-22, 53, 2000);
  chassis.moveToPoint(-37 , 42, 1400);
  chassis.turnToPoint(-20, 37, 1000,{.forwards = false});
  chassis.moveToPoint(-20,37, 1200, {.forwards = false, .minSpeed = 70});
  pros::delay(700);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 37, 2000, {.forwards = false, .maxSpeed = 20});
  chassis.moveToPoint(-61, 48, 2500, {.maxSpeed = 40});
  chassis.setPose({-31, 50, chassis.getPose().theta});
  scraper_tog();
  setScoringMode("INTAKE");
  chassis.moveToPoint(-80, 48, 1500, { .maxSpeed = 8});

  chassis.moveToPoint(-20, 50, 1200, {.forwards = false, .minSpeed = 70});
  pros::delay(700);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 50, 2000, {.forwards = false, .maxSpeed = 20});
  chassis.moveToPoint(-65, 30, 1500, {.maxSpeed = 70});
  chassis.setPose({-30, 48, chassis.getPose().theta});
  scraper_tog();
  chassis.turnToPoint(-70, -48, 1000);
  chassis.moveToPoint(-70, -48, 3000, {.minSpeed = 127});
  setScoringMode("OUTTAKE");
  pros::delay(1000000);
}

ASSET(leftrush_jerryio_txt);
inline void leftRush() {
  pros::lcd::print(6, "Running Left Rush Auto");
  chassis.setPose({50, -17, 180});
  startIMURotationTask();
  // LEFT RUSH AUTO GOES HERE
  RamseteToPoint(50, -47, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  moveDistance(100000, 1300, 40);
  chassis.moveToPoint(30, -48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.moveToPoint(36, -59.5, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(12, -59.5, 800, {.forwards= false});
  chassis.moveToPoint(12, -59.5, 1300, {.forwards = false});
  pros::delay(1000000);
}

inline void rightRush() {
  pros::lcd::print(6, "Running Right Rush Auto");
  chassis.setPose({50, 17, 0});
  startIMURotationTask();
  // RIGHT RUSH AUTO GOES HERE
  RamseteToPoint(50, 47, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  moveDistance(100000, 1300, 40);
  chassis.moveToPoint(30, 48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.moveToPoint(36, 36.5, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToPoint(12, 36.5, 800, {.forwards= false});
  chassis.moveToPoint(12, 36.5, 1300, {.forwards = false});
  pros::delay(1000000);
}