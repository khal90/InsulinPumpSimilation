#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <vector>
#include <map>

/**
 * Class representing a user profile for insulin delivery settings
 */
class Profile {
public:
    // Constructor with name
    Profile(const std::string& name);
    
    // Getters and setters
    std::string getName() const;
    void setName(const std::string& name);
    
    // Basal rate settings (in units per hour)
    void addBasalRate(int startHour, int startMinute, float rate);
    float getBasalRate(int hour, int minute) const;
    std::map<int, float> getAllBasalRates() const;
    
    // Carbohydrate ratio (grams of carbs per unit of insulin)
    void addCarbRatio(int startHour, int startMinute, float ratio);
    float getCarbRatio(int hour, int minute) const;
    std::map<int, float> getAllCarbRatios() const;
    
    // Correction factor (mmol/L drop per unit of insulin)
    void addCorrectionFactor(int startHour, int startMinute, float factor);
    float getCorrectionFactor(int hour, int minute) const;
    std::map<int, float> getAllCorrectionFactors() const;
    
    // Target glucose levels (mmol/L)
    void addTargetGlucose(int startHour, int startMinute, float target);
    float getTargetGlucose(int hour, int minute) const;
    std::map<int, float> getAllTargetGlucoses() const;
    
    // Insulin duration (in hours)
    void setInsulinDuration(float hours);
    float getInsulinDuration() const;
    
    // Profile validation
    bool isValid() const;
    std::string getValidationMessage() const;
    
private:
    std::string name;
    std::map<int, float> basalRates;        // Key is minutes since midnight
    std::map<int, float> carbRatios;        // Key is minutes since midnight
    std::map<int, float> correctionFactors; // Key is minutes since midnight
    std::map<int, float> targetGlucoses;    // Key is minutes since midnight
    float insulinDuration;
    
    // Helper methods
    int timeToMinutes(int hour, int minute) const;
    float getValueAtTime(const std::map<int, float>& settings, int hour, int minute) const;
};

#endif // PROFILE_H
