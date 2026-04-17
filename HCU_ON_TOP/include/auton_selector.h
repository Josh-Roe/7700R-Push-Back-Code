#pragma once

#include "main.h"
#include "lemlib/api.hpp"
#include <string>
#include <vector>

class AutonomousSelector {
public:
    using Routine = void (*)();

    void add(Routine routine, const char* name);
    void init(lemlib::Chassis* chassisPtr);
    void runSelected() const;

    int getSelectedIndex() const;
    int getConfirmedIndex() const;
    bool isConfirmed() const;
    std::string getSelectedName() const;
    std::string getConfirmedName() const;

private:
    struct Entry {
        Routine routine;
        std::string name;
    };

    std::vector<Entry> entries;
    lemlib::Chassis* chassis = nullptr;
    lemlib::Pose lastPose{0, 0, 0};

    int selectedIndex = -1;
    int confirmedIndex = -1;
    bool confirmed = false;

    pros::Task* poseTask = nullptr;

    lv_obj_t* screen = nullptr;
    lv_obj_t* poseXLabel = nullptr;
    lv_obj_t* poseYLabel = nullptr;
    lv_obj_t* poseThetaLabel = nullptr;
    lv_obj_t* confirmButton = nullptr;
    lv_obj_t* statusLabel = nullptr;
    lv_obj_t* listContainer = nullptr;
    std::vector<lv_obj_t*> listButtons;

    void buildUI();
    void refreshListStyles();
    void refreshStatus();
    void refreshPose();
    void confirmSelection();
    void disableListInteraction();
    void setButtonColor(int index, uint32_t color);

    static void onListButtonPressed(lv_event_t* e);
    static void onConfirmPressed(lv_event_t* e);
    static void poseTimerCallback(lv_timer_t* timer);
};

extern AutonomousSelector selector;
