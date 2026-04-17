#include "auton_selector.h"

#include <algorithm>
#include <cstdio>
#include "settings.h"

AutonomousSelector selector;

namespace {
constexpr lv_coord_t kScreenWidth  = 480;
constexpr lv_coord_t kScreenHeight = 240;
constexpr lv_coord_t kLeftPaneWidth = 180;
constexpr lv_coord_t kPadding      = 10;
constexpr lv_coord_t kButtonHeight  = 42;

// Colors
constexpr uint32_t kColorBackground  = 0x111111;
constexpr uint32_t kColorPanel       = 0x1E1E1E;
constexpr uint32_t kColorBorder      = 0x3A3A3A;
constexpr uint32_t kColorDefault     = 0x8FC7FF;
constexpr uint32_t kColorSelected    = 0x2E8B57;
constexpr uint32_t kColorConfirmed   = 0x3A6EA5;
constexpr uint32_t kColorDisabled    = 0x777777;
constexpr uint32_t kColorText        = 0xFFFFFF;
} // namespace

void AutonomousSelector::add(Routine routine, const char* name) {
    entries.push_back({routine, name == nullptr ? "Unnamed Auto" : std::string(name)});
}

void AutonomousSelector::init(lemlib::Chassis* chassisPtr) {
    chassis = chassisPtr;

    // Start with the autonomous chosen in code
    if (!entries.empty() && RUN_THIS_AUTO - 1 >= 0 &&
        RUN_THIS_AUTO - 1 < static_cast<int>(entries.size())) {
        selectedIndex = RUN_THIS_AUTO - 1;
    } else {
        selectedIndex = entries.empty() ? -1 : 0;
    }

    buildUI();

    // Highlight the default selected auto immediately
    if (selectedIndex >= 0) {
        setButtonColor(selectedIndex, kColorSelected);
    }

    refreshPose();
    lv_timer_create(AutonomousSelector::poseTimerCallback, 100, this);
}

int AutonomousSelector::getSelectedIndex() const { return selectedIndex; }
int AutonomousSelector::getConfirmedIndex() const { return confirmedIndex; }
bool AutonomousSelector::isConfirmed() const { return confirmed; }

std::string AutonomousSelector::getSelectedName() const {
    if (entries.empty() || selectedIndex < 0 || selectedIndex >= static_cast<int>(entries.size()))
        return "None";
    return entries[selectedIndex].name;
}

std::string AutonomousSelector::getConfirmedName() const {
    if (confirmedIndex < 0 || confirmedIndex >= static_cast<int>(entries.size()))
        return "None";
    return entries[confirmedIndex].name;
}

void AutonomousSelector::runSelected() const {
    if (entries.empty()) return;

    int indexToRun = confirmed ? confirmedIndex : selectedIndex;
    if (indexToRun < 0 || indexToRun >= static_cast<int>(entries.size())) return;

    entries[indexToRun].routine();
}

void AutonomousSelector::buildUI() {
    screen = lv_obj_create(nullptr);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(kColorBackground), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_screen_load(screen);

    lv_obj_t* leftPanel = lv_obj_create(screen);
    lv_obj_set_size(leftPanel, kLeftPaneWidth, kScreenHeight - 2 * kPadding);
    lv_obj_set_pos(leftPanel, kPadding, kPadding);
    lv_obj_remove_flag(leftPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(leftPanel, 8, 0);
    lv_obj_set_style_bg_color(leftPanel, lv_color_hex(kColorPanel), 0);
    lv_obj_set_style_border_color(leftPanel, lv_color_hex(kColorBorder), 0);
    lv_obj_set_style_border_width(leftPanel, 2, 0);

    lv_obj_t* poseTitleLabel = lv_label_create(leftPanel);
    lv_label_set_text(poseTitleLabel, "Robot Pose");
    lv_obj_align(poseTitleLabel, LV_ALIGN_TOP_LEFT, 8, 8);

    poseXLabel = lv_label_create(leftPanel);
    lv_label_set_text(poseXLabel, "X: 0.00");
    lv_obj_align(poseXLabel, LV_ALIGN_TOP_LEFT, 8, 38);

    poseYLabel = lv_label_create(leftPanel);
    lv_label_set_text(poseYLabel, "Y: 0.00");
    lv_obj_align(poseYLabel, LV_ALIGN_TOP_LEFT, 8, 62);

    poseThetaLabel = lv_label_create(leftPanel);
    lv_label_set_text(poseThetaLabel, "Theta: 0.00");
    lv_obj_align(poseThetaLabel, LV_ALIGN_TOP_LEFT, 8, 86);

    confirmButton = lv_button_create(leftPanel);
    lv_obj_set_size(confirmButton, kLeftPaneWidth - 22, kButtonHeight - 4);
    lv_obj_align(confirmButton, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_obj_add_event_cb(confirmButton, AutonomousSelector::onConfirmPressed, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(confirmButton, lv_color_hex(kColorSelected), 0);

    lv_obj_t* confirmButtonLabel = lv_label_create(confirmButton);
    lv_label_set_text(confirmButtonLabel, "Confirm");
    lv_obj_center(confirmButtonLabel);

    statusLabel = lv_label_create(leftPanel);
    lv_obj_set_width(statusLabel, kLeftPaneWidth - 16);
    lv_label_set_long_mode(statusLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(kColorText), 0);
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, 0);

    listContainer = lv_obj_create(screen);
    lv_obj_set_size(listContainer,
                    kScreenWidth - kLeftPaneWidth - 3 * kPadding,
                    kScreenHeight - 2 * kPadding);
    lv_obj_set_pos(listContainer, kLeftPaneWidth + 2 * kPadding, kPadding);
    lv_obj_set_style_radius(listContainer, 8, 0);
    lv_obj_set_style_bg_color(listContainer, lv_color_hex(kColorPanel), 0);
    lv_obj_set_style_border_color(listContainer, lv_color_hex(kColorBorder), 0);
    lv_obj_set_style_border_width(listContainer, 2, 0);
    lv_obj_set_flex_flow(listContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(listContainer, 8, 0);
    lv_obj_set_style_pad_row(listContainer, 8, 0);
    lv_obj_set_scroll_dir(listContainer, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(listContainer, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_remove_flag(listContainer, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_remove_flag(listContainer, LV_OBJ_FLAG_SCROLL_ELASTIC);

    listButtons.clear();
    for (std::size_t i = 0; i < entries.size(); ++i) {
        lv_obj_t* button = lv_button_create(listContainer);
        lv_obj_set_width(button, lv_pct(100));
        lv_obj_set_height(button, 44);
        lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(button, AutonomousSelector::onListButtonPressed, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(button, reinterpret_cast<void*>(i));
        lv_obj_set_style_bg_color(button, lv_color_hex(kColorDefault), 0);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);

        lv_obj_t* label = lv_label_create(button);
        lv_label_set_text(label, entries[i].name.c_str());
        lv_obj_center(label);

        listButtons.push_back(button);
    }

    refreshStatus();
}

void AutonomousSelector::setButtonColor(int index, uint32_t color) {
    if (index < 0 || index >= static_cast<int>(listButtons.size())) return;
    lv_obj_set_style_bg_color(listButtons[index], lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(listButtons[index], LV_OPA_COVER, 0);
}

void AutonomousSelector::refreshStatus() {
    if (entries.empty()) {
        lv_label_set_text(statusLabel, "No autonomous routines added");
        lv_obj_add_state(confirmButton, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(confirmButton, lv_color_hex(kColorDisabled), LV_STATE_DISABLED);
        return;
    }

    if (confirmed) {
        lv_label_set_text(statusLabel, "Running...");
        lv_obj_add_state(confirmButton, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(confirmButton, lv_color_hex(kColorDisabled), LV_STATE_DISABLED);
    } else {
        lv_label_set_text(statusLabel, "Selecting...");
        lv_obj_remove_state(confirmButton, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(confirmButton, lv_color_hex(kColorSelected), 0);
    }
}

void AutonomousSelector::refreshPose() {
    if (chassis == nullptr || poseXLabel == nullptr) return;

    lemlib::Pose pose = chassis->getPose();
    char buffer[64];

    if (pose.x != lastPose.x) {
        std::snprintf(buffer, sizeof(buffer), "X: %.4f", pose.x);
        lv_label_set_text(poseXLabel, buffer);
    }
    if (pose.y != lastPose.y) {
        std::snprintf(buffer, sizeof(buffer), "Y: %.4f", pose.y);
        lv_label_set_text(poseYLabel, buffer);
    }
    if (pose.theta != lastPose.theta) {
        std::snprintf(buffer, sizeof(buffer), "Theta: %.4f", pose.theta);
        lv_label_set_text(poseThetaLabel, buffer);
    }

    lastPose = pose;
}

void AutonomousSelector::confirmSelection() {
    if (entries.empty() || confirmed || selectedIndex < 0) return;

    confirmed = true;
    confirmedIndex = selectedIndex;

    setButtonColor(confirmedIndex, kColorConfirmed);

    disableListInteraction();
    refreshStatus();
}

void AutonomousSelector::disableListInteraction() {
    for (lv_obj_t* button : listButtons) {
        lv_obj_add_state(button, LV_STATE_DISABLED);
    }
}

void AutonomousSelector::onListButtonPressed(lv_event_t* e) {
    auto* self = static_cast<AutonomousSelector*>(lv_event_get_user_data(e));
    if (self == nullptr || self->confirmed) return;

    lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
    std::size_t index = reinterpret_cast<std::size_t>(lv_obj_get_user_data(target));
    if (index >= self->entries.size()) return;

    int newIndex = static_cast<int>(index);
    if (newIndex == self->selectedIndex) return;

    self->setButtonColor(self->selectedIndex, kColorDefault);
    self->setButtonColor(newIndex, kColorSelected);

    self->selectedIndex = newIndex;
    self->refreshStatus();
}

void AutonomousSelector::onConfirmPressed(lv_event_t* e) {
    auto* self = static_cast<AutonomousSelector*>(lv_event_get_user_data(e));
    if (self == nullptr) return;
    self->confirmSelection();
}

void AutonomousSelector::poseTimerCallback(lv_timer_t* timer) {
    auto* self = static_cast<AutonomousSelector*>(timer->user_data);
    if (self == nullptr) return;
    self->refreshPose();
}