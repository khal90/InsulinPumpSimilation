#include "TSlimX2Pump.h"
#include "Profile.h"
#include "Event.h"
#include "CGMData.h"
#include <algorithm>
#include <cmath>
#include <iostream>

TSlimX2Pump::TSlimX2Pump() : 
    currentState(OFF),
    currentError(NONE),
    errorMessage(""),
    batteryLevel(100.0),
    insulinLevel(0.0),
    insulinOnBoard(0.0),
    lastBolusTime(0),
    lastBolusAmount(0.0),
    controlIQEnabled(false),
    cgmConnected(false),
    currentGlucose(0.0),
    activeProfileName("")
{
    // Create a default profile
    std::shared_ptr<Profile> defaultProfile = std::make_shared<Profile>("Default");
    
    // Set up default basal rates (0.5 U/hr)
    for (int hour = 0; hour < 24; hour++) {
        defaultProfile->addBasalRate(hour, 0, 0.5);
    }
    
    // Set up default carb ratios (15g/U)
    for (int hour = 0; hour < 24; hour++) {
        defaultProfile->addCarbRatio(hour, 0, 15.0);
    }
    
    // Set up default correction factors (2 mmol/L per unit)
    for (int hour = 0; hour < 24; hour++) {
        defaultProfile->addCorrectionFactor(hour, 0, 2.0);
    }
    
    // Set up default target glucose (6.7 mmol/L)
    for (int hour = 0; hour < 24; hour++) {
        defaultProfile->addTargetGlucose(hour, 0, 6.7);
    }
    
    // Set default insulin duration (5 hours)
    defaultProfile->setInsulinDuration(5.0);
    
    // Add default profile to profiles map
    profiles["Default"] = defaultProfile;
    activeProfileName = "Default";
}

TSlimX2Pump::~TSlimX2Pump() {
    // Nothing to clean up specifically
}

bool TSlimX2Pump::powerOn() {
    if (currentState == OFF) {
        if (batteryLevel <= 0) {
            currentError = LOW_BATTERY;
            errorMessage = "Cannot power on: Battery depleted";
            return false;
        }
        
        currentState = ON;
        
        // Log power on event
        auto event = std::make_shared<ResumeEvent>(time(nullptr), "Power on");
        logEvent(event);
        
        return true;
    }
    return false; // Already on
}

bool TSlimX2Pump::powerOff() {
    if (currentState != OFF) {
        // Log any active delivery
        if (currentState == DELIVERING_BOLUS || currentState == DELIVERING_BASAL) {
            auto event = std::make_shared<SuspendEvent>(time(nullptr), "Power off");
            logEvent(event);
        }
        
        currentState = OFF;
        return true;
    }
    return false; // Already off
}

bool TSlimX2Pump::sleep() {
    if (currentState == ON || currentState == DELIVERING_BASAL) {
        currentState = SLEEP;
        return true;
    }
    return false;
}

bool TSlimX2Pump::wake() {
    if (currentState == SLEEP) {
        currentState = ON;
        return true;
    }
    return false;
}

float TSlimX2Pump::getBatteryLevel() const {
    return batteryLevel;
}

float TSlimX2Pump::getInsulinLevel() const {
    return insulinLevel;
}

bool TSlimX2Pump::chargeBattery(float amount) {
    if (amount <= 0) return false;
    
    batteryLevel += amount;
    if (batteryLevel > 100.0) {
        batteryLevel = 100.0;
    }
    
    if (currentError == LOW_BATTERY && batteryLevel > 15.0) {
        currentError = NONE;
        errorMessage = "";
    }
    
    return true;
}

bool TSlimX2Pump::refillInsulin(float amount) {
    if (amount <= 0) return false;
    if (currentState == OFF) return false;
    
    float maxCapacity = 300.0; // 300 unit cartridge
    float newLevel = insulinLevel + amount;
    
    if (newLevel > maxCapacity) {
        insulinLevel = maxCapacity;
    } else {
        insulinLevel = newLevel;
    }
    
    if (currentError == LOW_INSULIN && insulinLevel > 50.0) {
        currentError = NONE;
        errorMessage = "";
    }
    
    return true;
}

bool TSlimX2Pump::createProfile(const std::string& name) {
    if (name.empty() || profiles.find(name) != profiles.end()) {
        return false; // Invalid name or profile already exists
    }
    
    profiles[name] = std::make_shared<Profile>(name);
    
    return true;
}

std::shared_ptr<Profile> TSlimX2Pump::getProfile(const std::string& name) {
    auto it = profiles.find(name);
    if (it != profiles.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> TSlimX2Pump::getAllProfileNames() const {
    std::vector<std::string> names;
    for (const auto& pair : profiles) {
        names.push_back(pair.first);
    }
    return names;
}

bool TSlimX2Pump::updateProfile(const std::string& name, std::shared_ptr<Profile> profile) {
    if (name.empty() || profiles.find(name) == profiles.end() || !profile) {
        return false;
    }
    
    profiles[name] = profile;
    
    // If this is the active profile, we need to log the change
    if (name == activeProfileName) {
        auto event = std::make_shared<ProfileChangeEvent>(time(nullptr), name, name);
        logEvent(event);
    }
    
    return true;
}

bool TSlimX2Pump::deleteProfile(const std::string& name) {
    if (name == "Default" || name.empty() || profiles.find(name) == profiles.end()) {
        return false; // Cannot delete default profile or profile doesn't exist
    }
    
    // If this is the active profile, switch to Default
    if (name == activeProfileName) {
        activateProfile("Default");
    }
    
    profiles.erase(name);
    return true;
}

bool TSlimX2Pump::activateProfile(const std::string& name) {
    if (name.empty() || profiles.find(name) == profiles.end()) {
        return false; // Profile doesn't exist
    }
    
    // Log profile change
    auto event = std::make_shared<ProfileChangeEvent>(time(nullptr), activeProfileName, name);
    logEvent(event);
    
    std::string oldProfileName = activeProfileName;
    activeProfileName = name;
    
    // Update basal rate if we're currently delivering
    if (currentState == DELIVERING_BASAL) {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        
        auto oldProfile = getProfile(oldProfileName);
        auto newProfile = getProfile(name);
        
        float oldRate = oldProfile->getBasalRate(timeinfo->tm_hour, timeinfo->tm_min);
        float newRate = newProfile->getBasalRate(timeinfo->tm_hour, timeinfo->tm_min);
        
        if (oldRate != newRate) {
            auto event = std::make_shared<BasalChangeEvent>(now, oldRate, newRate, "Profile change");
            logEvent(event);
        }
    }
    
    return true;
}

std::string TSlimX2Pump::getActiveProfileName() const {
    return activeProfileName;
}

std::shared_ptr<Profile> TSlimX2Pump::getActiveProfile() const {
    auto it = profiles.find(activeProfileName);
    if (it != profiles.end()) {
        return it->second;
    }
    return nullptr;
}

bool TSlimX2Pump::deliverBolus(float units, bool extended, int durationMinutes) {
    // Check various safety conditions
    if (currentState == OFF || currentState == SLEEP || currentState == ERROR) {
        return false;
    }
    
    if (units <= 0 || insulinLevel < units) {
        return false;
    }
    
    if (extended && durationMinutes <= 0) {
        return false;
    }
    
    // Set up the bolus type
    BolusEvent::BolusType bolusType = extended ? 
        BolusEvent::EXTENDED : BolusEvent::MANUAL;
    
    // Log the bolus event
    auto event = std::make_shared<BolusEvent>(time(nullptr), bolusType, units, durationMinutes);
    logEvent(event);
    
    // Update pump state
    currentState = DELIVERING_BOLUS;
    insulinLevel -= units;
    insulinOnBoard += units;
    lastBolusTime = time(nullptr);
    lastBolusAmount = units;
    
    // Check if insulin is running low
    if (insulinLevel < 50.0 && currentError == NONE) {
        currentError = LOW_INSULIN;
        errorMessage = "Low insulin reservoir";
    }
    
    // After bolus delivery is complete, return to basal delivery
    // In a real implementation, this would be handled by a timer or thread
    // For this simulation, we'll just transition immediately
    if (!extended) {
        currentState = DELIVERING_BASAL;
    }
    
    return true;
}

bool TSlimX2Pump::cancelBolus() {
    if (currentState != DELIVERING_BOLUS) {
        return false; // No active bolus to cancel
    }
    
    // Find the most recent uncancelled bolus event
    for (auto it = eventHistory.rbegin(); it != eventHistory.rend(); ++it) {
        auto bolusEvent = std::dynamic_pointer_cast<BolusEvent>(*it);
        if (bolusEvent && !bolusEvent->isCancelled()) {
            // Cancel this bolus
            bolusEvent->setCancelled(true);
            
            // Calculate how much insulin was actually delivered
            // For simplicity, we'll assume half was delivered if cancelled
            float undeliveredInsulin = bolusEvent->getUnits() / 2.0;
            
            // Add the undelivered insulin back to the reservoir
            insulinLevel += undeliveredInsulin;
            
            // Adjust insulin on board
            insulinOnBoard -= undeliveredInsulin;
            
            // Log the cancellation
            auto event = std::make_shared<SuspendEvent>(time(nullptr), "Bolus cancelled");
            logEvent(event);
            
            // Return to basal delivery
            currentState = DELIVERING_BASAL;
            
            return true;
        }
    }
    
    return false; // No bolus found to cancel
}

bool TSlimX2Pump::startBasal() {
    if (currentState == OFF || currentState == SLEEP || currentState == ERROR) {
        return false;
    }
    
    if (insulinLevel <= 0) {
        currentError = LOW_INSULIN;
        errorMessage = "Cannot start basal: No insulin";
        return false;
    }
    
    currentState = DELIVERING_BASAL;
    
    // Log the basal start event
    auto profile = getActiveProfile();
    if (profile) {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        float rate = profile->getBasalRate(timeinfo->tm_hour, timeinfo->tm_min);
        
        auto event = std::make_shared<BasalChangeEvent>(now, 0.0, rate, "Basal started");
        logEvent(event);
    }
    
    return true;
}

bool TSlimX2Pump::stopBasal() {
    if (currentState != DELIVERING_BASAL && currentState != DELIVERING_BOLUS) {
        return false; // Not delivering insulin
    }
    
    currentState = SUSPENDED;
    
    // Log the stop event
    auto event = std::make_shared<SuspendEvent>(time(nullptr), "User stopped insulin");
    logEvent(event);
    
    return true;
}

bool TSlimX2Pump::resumeBasal() {
    if (currentState != SUSPENDED) {
        return false; // Not suspended
    }
    
    if (insulinLevel <= 0) {
        currentError = LOW_INSULIN;
        errorMessage = "Cannot resume basal: No insulin";
        return false;
    }
    
    currentState = DELIVERING_BASAL;
    
    // Log the resume event
    auto profile = getActiveProfile();
    if (profile) {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        float rate = profile->getBasalRate(timeinfo->tm_hour, timeinfo->tm_min);
        
        auto event = std::make_shared<ResumeEvent>(now, "User resumed insulin");
        logEvent(event);
        
        auto basal_event = std::make_shared<BasalChangeEvent>(now, 0.0, rate, "Basal resumed");
        logEvent(basal_event);
    }
    
    return true;
}

float TSlimX2Pump::getInsulinOnBoard() const {
    return insulinOnBoard;
}

bool TSlimX2Pump::enableControlIQ() {
    if (currentState == OFF || currentState == ERROR) {
        return false;
    }
    
    if (!cgmConnected) {
        return false; // Control IQ requires CGM
    }
    
    controlIQEnabled = true;
    return true;
}

bool TSlimX2Pump::disableControlIQ() {
    controlIQEnabled = false;
    return true;
}

bool TSlimX2Pump::isControlIQEnabled() const {
    return controlIQEnabled;
}

float TSlimX2Pump::calculateSuggestedBolus(float currentGlucose, float carbIntake) {
    auto profile = getActiveProfile();
    if (!profile) {
        return 0.0;
    }
    
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    // Get current settings from profile
    float carbRatio = profile->getCarbRatio(timeinfo->tm_hour, timeinfo->tm_min);
    float correctionFactor = profile->getCorrectionFactor(timeinfo->tm_hour, timeinfo->tm_min);
    float targetGlucose = profile->getTargetGlucose(timeinfo->tm_hour, timeinfo->tm_min);
    
    // Calculate food component
    float foodBolus = carbIntake / carbRatio;
    
    // Calculate correction component
    float glucoseDifference = currentGlucose - targetGlucose;
    float correctionBolus = 0.0;
    if (glucoseDifference > 0) {
        correctionBolus = glucoseDifference / correctionFactor;
    }
    
    // Account for
