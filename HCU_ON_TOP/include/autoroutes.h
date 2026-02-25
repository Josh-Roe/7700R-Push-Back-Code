#pragma once

#include "autonomousfunction.h"
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "lemlib/chassis/chassis.hpp"
#include "main.h"
#include "mechanicalfunction.h"

inline void leftQualsAuto() {
  pros::lcd::print(6, "Running Left Quals Auto");
  chassis.setPose({46.5, -13, -90});
  startFixPoseTask();
    //LEFT QUALS AUTO GOES HERE
  chassis.moveToPoint(23, -23, 1200);
  setScoringMode("INTAKE");
  pros::delay(500);
  scraper_tog();
  chassis.turnToHeading(-90, 600);
  chassis.moveToPoint(8,-44, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(18, -17, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, 2, 650, {.forwards = false});
  pros::delay(650);
  moveDistance(-10, 1200);
  setScoringMode("MIDDLE");
  pros::delay(800);
  setScoringMode("NONE");
  chassis.moveToPoint(48, -48, 1700, {.maxSpeed = 80});
  chassis.turnToHeading(88, 600);
  setScoringMode("INTAKE");
  pros::delay(600);
  moveDistance(100000, 1100, 35);
  chassis.moveToPoint(30, -48, 1400, {.forwards = false, .maxSpeed = 80});
  pros::delay(900);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 800, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, -37, 1500);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(11, -37, 800, {.forwards= false});
  chassis.moveToPoint(11, -37, 1500, {.forwards = false});
  chassis.turnToHeading(115, 600);
  pros::delay(1000000);
}
inline void rightQualsAuto() {
  pros::lcd::print(6, "Running Right Quals Auto");
  chassis.setPose({47.5, 16, -90});
  startFixPoseTask();
    //RIGHT QUALS AUTO GOES HERE
  chassis.moveToPoint(23, 22, 1200);
  setScoringMode("INTAKE");
  pros::delay(530);
  scraper_tog();
  chassis.turnToHeading(-90, 600);
  chassis.moveToPoint(8,42, 1400, {.maxSpeed = 90});
  pros::delay(200);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(19, 21, 1500, {.forwards = false, .maxSpeed = 100});
  chassis.turnToPoint(0, -2, 700);
  scraper_tog();
  pros::delay(650);
  moveDistance(6, 1200);
  setScoringMode("OUTTAKE");
  pros::delay(1100);
  setScoringMode("NONE");
  chassis.moveToPoint(48, 46, 1700, {.forwards = false, .maxSpeed = 90});
  chassis.turnToHeading(90, 800, {.maxSpeed = 80});
  setScoringMode("INTAKE");
  pros::delay(400);
  scraper_tog();
  pros::delay(400);
  moveDistance(100000, 1100, 40);
  chassis.moveToPoint(20, 48, 1400, {.forwards = false, .maxSpeed = 90});
  pros::delay(900);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 800, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(1500);
  chassis.moveToPoint(35, 59, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToPoint(13, 59, 800, {.forwards= false});
  setScoringMode("NONE");
  chassis.moveToPoint(13, 59, 10000, {.forwards = false});
  chassis.turnToHeading(105, 600);
  pros::delay(1000000);
}
inline void fullWPAuto() {
  pros::lcd::print(6, "Running Full WP Auto");
  chassis.setPose({50, 16.5, 0});
    startFixPoseTask();
    wing_tog();
  // Full WP AUTO GOES HERE
  moveDistance(31, 850, 110);
  chassis.turnToHeading(88, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 825, 40);
  chassis.moveToPoint(20, 48, 1000, {.forwards = false, .maxSpeed = 90});
  pros::delay(300);
  setScoringMode("NONE");
  pros::delay(100);
  setScoringMode("OUTTAKE");
  pros::delay(100);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 300, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(600);
  chassis.setPose({30, 48, chassis.getPose().theta});
  setScoringMode("INTAKE");
  moveDistance(8, 600);
  chassis.turnToPoint(23, 23, 600);
  chassis.moveToPoint(23, 23, 1400, {.minSpeed = 40});
  pros::delay(500);
  scraper_tog();
  chassis.moveToPoint(25.5, -26, 2000, {.minSpeed = 40});
  pros::delay(300);
  scraper_tog();
  pros::delay(600);
  scraper_tog();
  chassis.moveToPoint(48, -45.5, 1500, {.maxSpeed = 100});
  chassis.turnToPoint(28, -45, 600, {.forwards = false});
  chassis.moveToPoint(28, -45, 1900, {.forwards = false, .maxSpeed = 90});
  setScoringMode("NONE");
  pros::delay(200);
  setScoringMode("OUTTAKE");
  pros::delay(100);
  setScoringMode("TOP");
  pros::delay(1500);
  chassis.moveToPoint(90, -46, 600, {.maxSpeed = 80});
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.moveToPoint(90, -46, 675, {.maxSpeed = 40});
  pros::delay(600);
  setScoringMode("INTAKE");
  chassis.moveToPoint(12.5, -12, 1600, {.forwards = false, .maxSpeed = 100});
  pros::delay(1300);
  setScoringMode("MIDDLE");
  pros::delay(1100);
  moveDistance(13, 800, 127, 60);
  midgoal_tog();
  moveDistance(-13, 800, 127, 60);
  midgoal_tog();
  pros::delay(1000000);
}
inline void leftElimsAuto() {
  pros::lcd::print(6, "Running Left Elims Auto");
  chassis.setPose({50, -22.5, 180});
  startFixPoseTask();
  // LEFT ELIMS AUTO GOES HERE
  moveDistance(31, 1000);
  chassis.turnToHeading(91, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 925, 40);
  chassis.moveToPoint(20, -48, 1400, {.forwards = false, .maxSpeed = 80});
  pros::delay(300);
  setScoringMode("NONE");
  pros::delay(100);
  setScoringMode("OUTTAKE");
  pros::delay(100);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 600, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(600);
  chassis.setPose({30, -48, chassis.getPose().theta});
  moveDistance(8, 800);
  setScoringMode("INTAKE");
  chassis.turnToPoint(19,-24,650);
  chassis.moveToPoint(19, -24, 1400, {.minSpeed = 40});
  pros::delay(500);
  scraper_tog();
  chassis.turnToPoint(12.25, -11.5, 700, {.forwards = false});
  pros::delay(700);
  moveDistance(-20.5, 1000);
  setScoringMode("MIDDLE");
  pros::delay(1200);
  setScoringMode("NONE");
  scraper_tog();
  moveDistance(13, 800);
  midgoal_tog();
  moveDistance(-11, 800, 127, 70);
  chassis.setPose({12, -12, chassis.getPose().theta});
  chassis.moveToPoint(30, -37.5, 1200);
  midgoal_tog();
  chassis.turnToHeading(90, 800);
  pros::delay(800);
  moveDistance(-22, 1200);
  chassis.turnToHeading(105, 600);
  pros::delay(1000000);
}

inline void rightElimsAuto() {
  pros::lcd::print(6, "Running Right Elims Auto");
  chassis.setPose({50, 12.5, 0});
  startFixPoseTask();
  // RIGHT ELIMS AUTO GOES HERE
  moveDistance(31, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 850, 40);
  chassis.moveToPoint(20, 48, 1400, {.forwards = false, .maxSpeed = 80});
  pros::delay(900);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 600, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(600);
  chassis.setPose({30, 48, chassis.getPose().theta});
  moveDistance(8, 800);
  setScoringMode("INTAKE");
  chassis.turnToPoint(25,22,800);
  chassis.moveToPoint(25, 22, 1400, {.minSpeed = 40});
  pros::delay(400);
  scraper_tog();
  chassis.turnToPoint(0, -2, 600);
  pros::delay(600);
  scraper_tog();
  moveDistance(12, 1000);
  setScoringMode("OUTTAKE");
  pros::delay(1200);
  moveDistance(-13, 800);
  moveDistance(13, 800, 127, 50);
  chassis.moveToPoint(30, 36.5, 1200, {.forwards = false});
  chassis.turnToHeading(-90, 800);
  pros::delay(800);
  moveDistance(21, 1200);
  chassis.turnToHeading(-105, 600);
  pros::delay(1000000);
}

inline void tuningAuto() {
  pros::lcd::print(6, "Running Tuning Auto");
  startFixPoseTask();
  chassis.setPose({0, 0,0});
  //TUNING AUTO GOES HERE
  moveDistance(24, 10000);
  //chassis.moveToPoint(24, 24, 20000);
  pros::delay(1000000);
}

inline void skillsAuto() {
  pros::lcd::print(6, "Running Skills Auto");
  startFixPoseTask();
  chassis.setPose({-40, 15.25, 90});
  // SKILLS AUTO GOES HERE

//SCORE 2 MIDDLE
  wing_tog();
  setScoringMode("INTAKE");
  chassis.moveToPoint(-18.5 , 22, 1700, {.maxSpeed = 70});
  chassis.turnToPoint(-11, 8.5, 800 ,{.forwards = false});
  setScoringMode("NONE");
  chassis.moveToPoint(-11,8.5, 1200, {.forwards = false});
  pros::delay(500);
  setScoringMode("SKILLS");
  pros::delay(1500);  
  chassis.moveToPoint(-48 , 48, 1400, {.maxSpeed = 90});
  
    //GET FIRST MATCH LOADER

  chassis.turnToHeading(-91, 600);
    setScoringMode("INTAKE");
pros::delay(200);
scraper_tog();
pros::delay(400);
  moveDistance(10000, 2600, 40);
  chassis.moveToPoint(-44, 58.5, 1400, {.forwards = false, .maxSpeed = 100});
  //GO DOWN SIDE AND SCORE TOP
  chassis.turnToPoint(34, 59.5, 1300, {.maxSpeed = 70});
  setScoringMode("NONE");
  chassis.moveToPoint(34, 58.5, 1900, {.maxSpeed = 100});
  setScoringMode("OUTTAKE");
  pros::delay(100);
  setScoringMode("INTAKE");
  pros::delay(300);
  setScoringMode("NONE");
  pros::delay(600);
  setScoringMode("INTAKE");
  chassis.turnToHeading(90, 800);
  chassis.moveToPoint(42, 46, 1500, {.minSpeed = 50});
  chassis.setPose({chassis.getPose().x, 59.75f - static_cast<float>(distance_sensor_left.get()/25.4), chassis.getPose().theta});
  chassis.turnToPoint(30, 48, 800, {.forwards = false});
  chassis.moveToPoint(30, 48, 1300, {.forwards = false, .minSpeed = 80});
  pros::delay(400);
  setScoringMode("OUTTAKE");
  pros::delay(200);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 1950, {.forwards = false, .maxSpeed = 40});
  pros::delay(1950);
  chassis.setPose({30, 48, chassis.getPose().theta});
  setScoringMode("INTAKE");
  //GET SECOND MATCH LOADER
  chassis.moveToPoint(80, 46.5, 700, { .maxSpeed = 70});
  chassis.moveToPoint(80, 46.5, 2700, { .maxSpeed= 30});
  chassis.moveToPoint(30, 48.5, 1000, {.forwards = false, .maxSpeed = 80});
  setScoringMode("NONE");
  pros::delay(700);
  setScoringMode("OUTTAKE");
  pros::delay(150);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48.5, 2300, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(2300);

  // LINE UP FOR PARK CLEAR AND SCORE
  chassis.setPose({30, 48, chassis.getPose().theta});
  moveDistance(23, 1200);
  chassis.moveToPoint(66, 20, 1700, {.maxSpeed = 80});
  chassis.swingToPoint(76, 0, lemlib::DriveSide::RIGHT,700, {.maxSpeed = 100});
chassis.moveToPoint(82, -100, 2800, {.maxSpeed = 75});
  setScoringMode("INTAKE");
  pros::delay(100);
  scraper_tog();
  pros::delay(250);
  scraper_tog();
  chassis.turnToHeading(160, 800);
  pros::delay(800);

  moveDistance(-10, 1000);
  setScoringMode("NONE");
  chassis.turnToHeading(-90, 800);
  pros::delay(800);
  resetOnPark();
  chassis.moveToPoint(17, -19.5, 1600, {.maxSpeed = 90});
  setScoringMode("INTAKE");
  chassis.turnToPoint(11, -8, 800, {.forwards = false});
  chassis.moveToPoint(11, -8, 1300, {.forwards = false});
    scraper_tog();
  pros::delay(600);
  setScoringMode("OUTTAKE");
  pros::delay(150);
  setScoringMode("SKILLS");
  pros::delay(3000);
  setScoringMode("NONE");
  chassis.turnToHeading(135, 800);
  moveDistance(-100000, 600, 30, 30);
  // chassis.setPose({30, 48, chassis.getPose().theta});
  // moveDistance(8, 800, 127, 40);
  // setScoringMode("INTAKE");
  // chassis.turnToPoint(26,23,650);
  // chassis.moveToPoint(26, 23, 1400, {.minSpeed = 40});
  // pros::delay(500);
  // scraper_tog();
  // chassis.moveToPoint(28.5, -29, 2000, {.minSpeed = 40});
  // pros::delay(300);
  // scraper_tog();
  // pros::delay(550);
  // scraper_tog();
  // chassis.moveToPoint(15, -8.5, 1300, {.forwards = false, .maxSpeed = 100});
  // pros::delay(800);
  // setScoringMode("SKILLS");
  // pros::delay(1200);
  // setScoringMode("NONE");
  // pros::delay(300);

  //GET THIRD MATCH LOADER
  chassis.moveToPoint(48, -47.5, 2300, {.maxSpeed = 100});
  pros::delay(300);
  midgoal_tog();
  pros::delay(300);
  midgoal_tog();
  chassis.turnToHeading(88, 700);
  setScoringMode("INTAKE");
  moveDistance(10000, 2600, 40);
  chassis.moveToPoint(44, -59.5, 1000, {.forwards = false, .maxSpeed = 80});
  pros::delay(600);

  //GO DOWN THE SIDE AND SCORE TOP
  chassis.turnToPoint(-28, -60.5, 1200, {.maxSpeed = 70});
  chassis.moveToPoint(-28, -59.5, 1800, {.maxSpeed = 100});
  setScoringMode("NONE");
  setScoringMode("OUTTAKE");
  pros::delay(200);
  setScoringMode("INTAKE");
  pros::delay(300);
  setScoringMode("NONE");
  pros::delay(600);
  setScoringMode("INTAKE");
  chassis.turnToHeading(-90, 800);
  chassis.moveToPoint(-42, -45, 1500, { .minSpeed = 50});
  chassis.setPose({chassis.getPose().x, -60.75f + static_cast<float>(distance_sensor_left.get()/25.4), chassis.getPose().theta});
  chassis.turnToPoint(-28, -48, 800, {.forwards = false});
  chassis.moveToPoint(-28, -48, 1000, {.forwards = false, .maxSpeed = 80});
  setScoringMode("NONE");
  pros::delay(450);
  setScoringMode("OUTTAKE");
  pros::delay(150);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1800, {.forwards = false, .maxSpeed = 40});
  chassis.setPose({-30, -48, chassis.getPose().theta});
  //GET FOURTH MATCH LOADER
  chassis.moveToPoint(-80, -45.5, 600, { .maxSpeed = 80});
  chassis.moveToPoint(-80, -45.5, 2200, { .maxSpeed = 40});
    setScoringMode("INTAKE");
  
  chassis.moveToPoint(-28, -48.5, 1000, {.forwards = false, .maxSpeed = 80});
  pros::delay(400);
  setScoringMode("NONE");
  pros::delay(400);
  setScoringMode("OUTTAKE");
  pros::delay(150);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 1800, {.forwards = false, .maxSpeed = 40});
  pros::delay(1800);
  // PARK
    chassis.setPose({-30, -48, chassis.getPose().theta});
  moveDistance(20, 1200);
  scraper_tog();
  chassis.moveToPoint(-64, -18, 1700, {.maxSpeed = 80});
  chassis.swingToPoint(-72, 0, lemlib::DriveSide::RIGHT,1000);
  chassis.moveToPoint(-84, 100, 1050, {.maxSpeed = 80, .minSpeed = 80});
  setScoringMode("OUTTAKE");
  pros::delay(100);
  scraper_tog();
  pros::delay(250);
  scraper_tog();

  pros::delay(1000000);
}

ASSET(leftrush_jerryio_txt);
inline void leftRush() {
  pros::lcd::print(6, "Running Left Rush Auto");
  chassis.setPose({50, -18, 180});
  startFixPoseTask();
  // LEFT RUSH AUTO GOES HERE
  moveDistance(30, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 850, 40);
  chassis.moveToPoint(20, -48, 1300, {.forwards = false, .maxSpeed = 80});
  pros::delay(800);
  setScoringMode("TOP");
  chassis.moveToPoint(0, -48, 200, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.moveToPoint(36, -37, 1200);
  chassis.setPose({30, -48, chassis.getPose().theta});
  chassis.turnToPoint(12, -37, 600, {.forwards= false});
  setScoringMode("NONE");
  chassis.moveToPoint(12, -37, 1400, {.forwards = false});
  chassis.turnToHeading(115, 600);
  scraper_tog();
  pros::delay(1000000);
}

inline void rightRush() {
  pros::lcd::print(6, "Running Right Rush Auto");
  chassis.setPose({50, 12.5, 0});
  startFixPoseTask();
  // RIGHT RUSH AUTO GOES HERE
  moveDistance(31, 1000);
  chassis.turnToHeading(90, 600);
  pros::delay(200);
  scraper_tog();
  setScoringMode("INTAKE");
  pros::delay(400);
  moveDistance(100000, 850, 40);
  chassis.moveToPoint(20, 48, 1400, {.forwards = false, .minSpeed = 80});
  pros::delay(900);
  setScoringMode("TOP");
  chassis.moveToPoint(0, 48, 400, {.forwards = false, .maxSpeed = 40});
  scraper_tog();
  pros::delay(900);
  chassis.moveToPoint(36, 59, 1500);
  chassis.setPose({30, 48, chassis.getPose().theta});
  chassis.turnToPoint(12, 59, 800, {.forwards= false});
  setScoringMode("NONE");
  chassis.moveToPoint(12, 59, 1300, {.forwards = false});
  chassis.turnToHeading(105, 600);
  pros::delay(1000000);
}