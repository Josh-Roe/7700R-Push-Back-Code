#pragma once

#include "autonomousfunction.h"
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "lemlib/chassis/chassis.hpp"
#include "main.h"
#include "mechanicalfunction.h"

inline void leftQualsAuto() {
  pros::lcd::print(6, "Running Left Quals Auto");
  chassis.setPose({47, -15, -90});
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
  chassis.moveToPoint(15, -18.5, 1500, {.forwards = false, .maxSpeed = 100});
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
  pros::delay(600);
  moveDistance(100000, 1700, 35);
  chassis.moveToPoint(30, -48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1500, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, -59, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(13, -59, 800, {.forwards= false});
  chassis.moveToPoint(13, -59, 10000, {.forwards = false});
  pros::delay(1000000);
}
inline void rightQualsAuto() {
  pros::lcd::print(6, "Running Right Quals Auto");
  chassis.setPose({50, 18, -90});
  startIMURotationTask();
    //RIGHT QUALS AUTO GOES HERE
  chassis.moveToPoint(23, 26, 1200);
  setScoringMode("INTAKE");
  pros::delay(530);
  scraper_tog();
  chassis.turnToHeading(-85, 600);
  chassis.moveToPoint(8,44, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(19, 23, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, -2, 700);
  scraper_tog();
  pros::delay(650);
  moveDistance(6, 1200);
  setScoringMode("OUTTAKE");
  pros::delay(1100);
  setScoringMode("NONE");
  chassis.moveToPoint(48, 46, 1700, {.forwards = false, .maxSpeed = 80});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  pros::delay(200);
  scraper_tog();
  pros::delay(400);
  moveDistance(100000, 1700, 30);
  chassis.moveToPoint(30, 48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 1500, {.forwards = false, .maxSpeed = 40});
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
  chassis.turnToHeading(92, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(600);
  moveDistance(100000, 150, 80);
  moveDistance(100000, 1000, 35);
  chassis.moveToPoint(30, 48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.setPose({30, 48, chassis.getPose().theta});
  setScoringMode("INTAKE");
  moveDistance(8, 800, 127, 30);
  chassis.turnToPoint(24,24,800);
  chassis.moveToPoint(24, 24, 1400, {.minSpeed = 40});
  pros::delay(500);
  scraper_tog();
  chassis.moveToPoint(24.5, -29, 2000, {.minSpeed = 40});
  pros::delay(300);
  scraper_tog();
  pros::delay(700);
  scraper_tog();
  chassis.moveToPoint(14, -11, 1300, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, 0, 600, {.forwards = false});
  setScoringMode("OUTTAKE");
  pros::delay(350);
  setScoringMode("MIDDLE");
  pros::delay(1300);
  setScoringMode("NONE");
  chassis.moveToPoint(50, -46, 1500, {.maxSpeed = 100});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  pros::delay(600);
  moveDistance(100000, 150, 80);
  moveDistance(100000, 1000, 35);
  chassis.moveToPoint(30, -46, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -46, 1000, {.forwards = false, .maxSpeed = 40});
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
  chassis.moveToPoint(8,-46.5, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(22, -30, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(48, -48.5, 700);
  chassis.moveToPoint(48, -48.5, 1700, {.maxSpeed = 80});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  pros::delay(600);
  moveDistance(100000, 1800, 30);
  chassis.moveToPoint(30, -49, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -49, 1750, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1750);
  chassis.moveToPoint(35, -60, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(13, -60, 800, {.forwards= false});
  chassis.moveToPoint(13, -60, 10000, {.forwards = false});
  pros::delay(1000000);
}

inline void rightElimsAuto() {
  pros::lcd::print(6, "Running Right Elims Auto");
  chassis.setPose({49, 17, -90});
  startIMURotationTask();
    //RIGHT ELIMS AUTO GOES HERE
  chassis.moveToPoint(24, 26, 1200);
  setScoringMode("INTAKE");
  pros::delay(530);
  scraper_tog();
  chassis.turnToPoint(48, 48, 600);
  // chassis.moveToPoint(9,44, 1400, {.maxSpeed = 90});
  // pros::delay(200);
  // scraper_tog();
  // pros::delay(600);
  // scraper_tog();
  // chassis.moveToPoint(32, 31, 1700, {.forwards = false, .maxSpeed = 100});
  // chassis.turnToPoint(48, 46, 700);
  chassis.moveToPoint(48, 48, 1700, {.maxSpeed = 80});
  chassis.turnToHeading(90, 600);
  setScoringMode("INTAKE");
  pros::delay(600);
  moveDistance(100000, 1600, 40);
  chassis.moveToPoint(30, 48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(700);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 1900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1900);
  chassis.moveToPoint(39, 35, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToHeading(90, 650);
  setScoringMode("NONE");
  pros::delay(650);
  moveDistance(-27, 2000);
  chassis.turnToHeading(75, 600);
  pros::delay(1000000);
}

inline void tuningAuto() {
  pros::lcd::print(6, "Running Tuning Auto");
  chassis.setPose({-29, -48,-90});
  //TUNING AUTO GOES HERE
  pros::delay(3000);
  distanceResetGoal();
  //chassis.moveToPoint(24, 24, 20000);
  pros::delay(1000000);
}

inline void skillsAuto() {
  pros::lcd::print(6, "Running Skills Auto");
  chassis.setPose({-45, 12.5, 90});
  // SKILLS AUTO GOES HERE

//SCORE 2 MIDDLE
  wing_tog();
  setScoringMode("INTAKE");
  chassis.moveToPoint(-25 , 21, 1500, {.maxSpeed = 90});
  chassis.turnToPoint(-15, 11, 800, {.forwards = false});
  setScoringMode("NONE");
  chassis.moveToPoint(-16, 12, 1200, {.forwards = false});
  pros::delay(400);
  setScoringMode("MIDDLE");
  pros::delay(1600);

  //GET FIRST MATCH LOADER
  chassis.moveToPoint(-48 , 48, 1400, {.maxSpeed = 100});
  chassis.turnToHeading(-90, 600);




  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 200, 80);
  moveDistance(100000, 2400, 40);
  chassis.moveToPoint(-32, 58, 1400, {.forwards = false, .maxSpeed = 100});

  //GO DOWN SIDE AND SCORE TOP
  chassis.turnToPoint(32, 57, 1000, {.forwards = false});
  setScoringMode("NONE");
  scraper_tog();
  chassis.moveToPoint(32, 57, 1900, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(42, 44, 1200, {.direction = AngularDirection::CCW_COUNTERCLOCKWISE});
  chassis.moveToPoint(42 , 44, 1500, {.minSpeed = 50});
  chassis.turnToPoint(30, 46, 800, {.forwards = false});
  chassis.moveToPoint(30, 46, 1300, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 1800, {.forwards = false, .maxSpeed = 40});
  pros::delay(1800);
  chassis.setPose({30, 48, chassis.getPose().theta});
  setScoringMode("INTAKE");
  scraper_tog();

  //GET SECOND MATCH LOADER
  chassis.moveToPoint(90, 45, 700, { .maxSpeed = 80});
  chassis.moveToPoint(90, 45, 2600, { .maxSpeed= 35});
  chassis.moveToPoint(30, 48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 2200, {.forwards = false, .maxSpeed = 40});
  scraper_tog();

  //LINE UP FOR PARK CLEAR AND SCORE
  chassis.moveToPoint(43 , 48, 1000);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.moveToPoint(62, 20, 1700, {.maxSpeed = 80});
  chassis.turnToPoint(70, 0, 800);
  chassis.moveToPoint(80, -200, 1300, {.minSpeed = 127});
  setScoringMode("INTAKE");
  pros::delay(100);
  scraper_tog();
  pros::delay(500);
  scraper_tog();
  pros::delay(1000);
  chassis.moveToPoint(80, -200, 1300);
  
  chassis.turnToHeading(195, 1000);
  
  pros::delay(1000);
  moveDistance(-500, 2000, 40);
  setScoringMode("NONE");
  resetOnPark();
  chassis.turnToPoint(24, -9 ,1400);
  chassis.moveToPoint(25, -21, 1500);
  setScoringMode("INTAKE");
  chassis.turnToPoint(17.5, -13.5, 800, {.forwards = false});
  chassis.moveToPoint(17.5, -13.5, 1200, {.forwards = false});
  setScoringMode("OUTTAKE");
  pros::delay(200);
  setScoringMode("NONE");
  pros::delay(200);
  setScoringMode("MIDDLE");
  pros::delay(4000);
  
  //GET THIRD MATCH LOADER
  chassis.moveToPoint(48, -49, 2000, {.maxSpeed = 100});
  chassis.turnToHeading(90, 600);
  setScoringMode("INTAKE");
  pros::delay(200);
  scraper_tog();
  pros::delay(400);
  moveDistance(100000, 200, 80);
  moveDistance(100000, 3000, 40);
  chassis.moveToPoint(32, -60, 1000, {.forwards = false, .maxSpeed = 80});
  pros::delay(600);
  scraper_tog();

  //GO DOWN THE SIDE AND SCORE TOP
  chassis.turnToPoint(-30, -59, 800, {.forwards= false});
  setScoringMode("NONE");
  chassis.moveToPoint(-30, -59, 1800, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(-44, -46, 1000);
  chassis.moveToPoint(-44, -46, 1500, {.minSpeed = 50});
  chassis.turnToPoint(-30, -46, 800, {.forwards = false});
  chassis.moveToPoint(-30, -46, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1800, {.forwards = false, .maxSpeed = 40});
  chassis.setPose({-30, -48, chassis.getPose().theta});
  //GET FOURTH MATCH LOADER
  chassis.moveToPoint(-90, -45, 600, { .maxSpeed = 80});
  scraper_tog();
  chassis.moveToPoint(-90, -45, 2200, { .maxSpeed = 30});
    setScoringMode("INTAKE");
  
  chassis.moveToPoint(-32, -48, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1800, {.forwards = false, .maxSpeed = 40});
  scraper_tog();

  // PARK
  chassis.moveToPoint(-43 , -48, 1000);
  chassis.setPose({-30, -48, chassis.getPose().theta});
  chassis.moveToPoint(-62, -17, 1700, {.maxSpeed = 80});
  chassis.turnToPoint(-70, 0, 1000);
  chassis.moveToPoint(-78, 200, 1100, {.minSpeed = 127});
  setScoringMode("INTAKE");
  pros::delay(100);
  scraper_tog();
  pros::delay(500);
  scraper_tog();

  pros::delay(1000000);
}

ASSET(leftrush_jerryio_txt);
inline void leftRush() {
  pros::lcd::print(6, "Running Left Rush Auto");
  chassis.setPose({50, -18, 180});
  startIMURotationTask();
  // LEFT RUSH AUTO GOES HERE
  RamseteToPoint(50, -47.5, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 1300, 30);
  chassis.moveToPoint(30, -47, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -47, 900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.moveToPoint(36, -59.5, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(12, -59.5, 800, {.forwards= false});
  chassis.moveToPoint(12, -59.5, 1400, {.forwards = false});
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
  pros::delay(400);
  moveDistance(100000, 1300, 30);
  chassis.moveToPoint(30, 47.5, 1000, {.forwards = false, .minSpeed = 80});
  pros::delay(600);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 47.5, 900, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.moveToPoint(36, 36.5, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToPoint(12, 36.5, 800, {.forwards= false});
  chassis.moveToPoint(12, 36.5, 1300, {.forwards = false});
  pros::delay(1000000);
}