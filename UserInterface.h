#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "TSlimX2Pump.h"
#include <memory>
#include <string>

/**
 * Class representing the user interface for interacting with the t:slim X2 Insulin Pump
 */
class UserInterface {
public:
    // Constructor taking a shared pointer to the pump
    UserInterface(std::shared_ptr<TSlimX2Pump> pump);
    
    // Main UI loop
    void run();
    
private:
    std::shared_ptr<TSlimX2Pump> pump;
    bool running;
    bool locked;
    std::string pin;
    
    // Display methods
    void displayHomeScreen();
    void displayMenu();
    void displayBatteryStatus();
    void displayInsulinStatus();
    void displayIOBStatus();
    void displayCGMData();
    void displayError();
    
    // Action methods
    void handleBolus();
    void handleBasalControl();
    void handleProfileManagement();
    void handleSettings();
    void handleHistoryReview();
    void handleLockScreen();
    
    // Profile management helpers
    void createNewProfile();
    void viewProfile();
    void updateProfile();
    void deleteProfile();
    void activateProfile();
    
    // Helper methods
    void clearScreen();
    std::string getUserInput(const std::string& prompt);
    float getNumericInput(const std::string& prompt, float min, float max);
    int getIntegerInput(const std::string& prompt, int min, int max);
    bool getConfirmation(const std::string& prompt);
    void showMessage(const std::string& message);
    void waitForKey();
};

#endif // USER_INTERFACE_H
