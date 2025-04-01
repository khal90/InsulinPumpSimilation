#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <ctime>
#include <memory>

/**
 * Base class for all pump events
 */
class Event {
public:
    enum EventType {
        BOLUS,
        BASAL_CHANGE,
        PROFILE_CHANGE,
        SUSPEND,
        RESUME,
        CGM_READING,
        ALARM,
        ERROR
    };
    
    Event(EventType type, time_t timestamp);
    virtual ~Event() = default;
    
    EventType getType() const;
    time_t getTimestamp() const;
    virtual std::string getDescription() const = 0;
    
protected:
    EventType type;
    time_t timestamp;
};

/**
 * Class representing a bolus insulin delivery event
 */
class BolusEvent : public Event {
public:
    enum BolusType {
        MANUAL,
        EXTENDED,
        QUICK,
        CORRECTION
    };
    
    BolusEvent(time_t timestamp, BolusType bolusType, float units, int durationMinutes = 0);
    
    BolusType getBolusType() const;
    float getUnits() const;
    int getDurationMinutes() const;
    bool isCancelled() const;
    void setCancelled(bool cancelled);
    std::string getDescription() const override;
    
private:
    BolusType bolusType;
    float units;
    int durationMinutes;
    bool cancelled;
};

/**
 * Class representing a basal rate change event
 */
class BasalChangeEvent : public Event {
public:
    BasalChangeEvent(time_t timestamp, float oldRate, float newRate, const std::string& reason);
    
    float getOldRate() const;
    float getNewRate() const;
    std::string getReason() const;
    std::string getDescription() const override;
    
private:
    float oldRate;
    float newRate;
    std::string reason;
};

/**
 * Class representing a profile change event
 */
class ProfileChangeEvent : public Event {
public:
    ProfileChangeEvent(time_t timestamp, const std::string& oldProfile, const std::string& newProfile);
    
    std::string getOldProfile() const;
    std::string getNewProfile() const;
    std::string getDescription() const override;
    
private:
    std::string oldProfile;
    std::string newProfile;
};

/**
 * Class representing a suspend insulin delivery event
 */
class SuspendEvent : public Event {
public:
    SuspendEvent(time_t timestamp, const std::string& reason);
    
    std::string getReason() const;
    std::string getDescription() const override;
    
private:
    std::string reason;
};

/**
 * Class representing a resume insulin delivery event
 */
class ResumeEvent : public Event {
public:
    ResumeEvent(time_t timestamp, const std::string& reason);
    
    std::string getReason() const;
    std::string getDescription() const override;
    
private:
    std::string reason;
};

/**
 * Class representing a CGM reading event
 */
class CGMReadingEvent : public Event {
public:
    CGMReadingEvent(time_t timestamp, float glucoseValue);
    
    float getGlucoseValue() const;
    std::string getDescription() const override;
    
private:
    float glucoseValue;
};

/**
 * Class representing an alarm event
 */
class AlarmEvent : public Event {
public:
    enum AlarmType {
        LOW_GLUCOSE,
        HIGH_GLUCOSE,
        LOW_INSULIN,
        LOW_BATTERY,
        OCCLUSION,
        CGM_DISCONNECTION
    };
    
    AlarmEvent(time_t timestamp, AlarmType alarmType, const std::string& details);
    
    AlarmType getAlarmType() const;
    std::string getDetails() const;
    std::string getDescription() const override;
    
private:
    AlarmType alarmType;
    std::string details;
};

/**
 * Class representing an error event
 */
class ErrorEvent : public Event {
public:
    ErrorEvent(time_t timestamp, const std::string& errorCode, const std::string& errorMessage);
    
    std::string getErrorCode() const;
    std::string getErrorMessage() const;
    std::string getDescription() const override;
    
private:
    std::string errorCode;
    std::string errorMessage;
};

#endif // EVENT_H
