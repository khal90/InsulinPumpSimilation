#ifndef CGM_DATA_H
#define CGM_DATA_H

#include <vector>
#include <ctime>

/**
 * Class representing Continuous Glucose Monitoring data
 */
class CGMData {
public:
    struct GlucoseReading {
        time_t timestamp;
        float value;     // Glucose value in mmol/L
        bool isValid;    // Flag for valid reading
    };
    
    CGMData();
    
    // Add a new glucose reading
    void addReading(float value, time_t timestamp = time(nullptr));
    
    // Get the most recent reading
    GlucoseReading getCurrentReading() const;
    
    // Get readings within a time range
    std::vector<GlucoseReading> getReadings(time_t startTime, time_t endTime) const;
    
    // Get trend information
    float calculateTrend() const; // Returns rate of change in mmol/L per minute
    
    // Predict future glucose based on current trend
    float predictGlucose(int minutesAhead) const;
    
    // Check if readings indicate low or high glucose
    bool isLowGlucose(float threshold = 3.9) const;
    bool isHighGlucose(float threshold = 10.0) const;
    
    // Get historical statistics
    float getAverageGlucose(time_t startTime, time_t endTime) const;
    float getStandardDeviation(time_t startTime, time_t endTime) const;
    float getTimeInRange(float lowerBound, float upperBound, time_t startTime, time_t endTime) const;
    
private:
    std::vector<GlucoseReading> readings;
};

#endif // CGM_DATA_H
