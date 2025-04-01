#ifndef TSLIM_X2_PUMP_H
#define TSLIM_X2_PUMP_H

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <memory>

// Forward declarations
class Profile;
class Event;
class CGMData;

/**
 * Class representing the t:slim X2 Insulin Pump
 */
class TSlimX2Pump {
public:
    // Pump states
    enum State {
        OFF,
        ON,
        SLEEP,
        DELIVERING_BOLUS,
        DELIVERING_BASAL,
        SUSPENDED,
        ERROR
    };
    
    // Error types
    enum ErrorType {
        NONE,
        LOW_BATTERY,
        LOW_INSULIN,
        OCCLUSION,
        CGM_DISCONNECTION,
        CRITICAL_ERROR
    };

    TSlimX2Pump();
    ~TSlimX2Pump();
    
    // Basic pump functions
    bool powerOn();
    bool powerOff();
    bool sleep();
    bool wake();
    
    // Battery and insulin management
    float getBatteryLevel() const;
    float getInsulinLevel() const;
    bool chargeBattery(float amount);
    bool refillInsulin(float amount);
    
    // Profile management (CRUD operations)
    bool createProfile(const std::string& name);
    std::shared_ptr<Profile> getProfile(const std::string& name);
    std::vector<std::string> getAllProfileNames() const;
    bool updateProfile(const std::string& name, std::shared_ptr<Profile> profile);
    bool deleteProfile(const std::string& name);
    bool activateProfile(const std::string& name);
    std::string getActiveProfileName() const;
    std::shared_ptr<Profile> getActiveProfile() const;
    
    // Insulin delivery functions
    bool deliverBolus(float units, bool extended = false, int durationMinutes = 0);
    bool cancelBolus();
    bool startBasal();
    bool stopBasal();
    bool resumeBasal();
    float getInsulinOnBoard() const;
    
    // Control IQ technology functions
    bool enableControlIQ();
    bool disableControlIQ();
    bool isControlIQEnabled() const;
    float calculateSuggestedBolus(float currentGlucose, float carbIntake);
    
    // CGM integration
    bool connectCGM();
    bool disconnectCGM();
    bool isCGMConnected() const;
    float getCurrentGlucose() const;
    void updateCGMData(float glucoseValue);
    
    // History and data storage
    std::vector<std::shared_ptr<Event>> getHistory(time_t startTime, time_t endTime);
    std::vector<std::shared_ptr<Event>> getRecentEvents(int count);
    float getLastBolusAmount() const;
    time_t getLastBolusTime() const;
    
    // Error handling
    ErrorType getErrorState() const;
    std::string getErrorMessage() const;
    bool clearError();
    
    // Pump state
    State getState() const;
    
private:
    // Private implementation details
    State currentState;
    ErrorType currentError;
    std::string errorMessage;
    
    float batteryLevel;
    float insulinLevel;
    float insulinOnBoard;
    time_t lastBolusTime;
    float lastBolusAmount;
    
    bool controlIQEnabled;
    bool cgmConnected;
    float currentGlucose;
    
    std::string activeProfileName;
    std::map<std::string, std::shared_ptr<Profile>> profiles;
    std::vector<std::shared_ptr<Event>> eventHistory;
    
    // Helper methods
    void logEvent(std::shared_ptr<Event> event);
    void updateInsulinOnBoard();
    bool checkSafety() const;
    void simulateInsulinAbsorption();
};

#endif // TSLIM_X2_PUMP_H
