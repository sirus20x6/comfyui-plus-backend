// app/include/comfyui_plus_backend/utils/DateTimeUtils.h
#pragma once

#include <trantor/utils/Date.h>
#include <chrono>
#include <string>
#include <optional>

namespace comfyui_plus_backend
{
namespace app
{
namespace utils
{

class DateTimeUtils
{
public:
    // Convert a database timestamp string to a trantor::Date
    static trantor::Date dbStringToDate(const std::string &dbDateString) {
        try {
            // Try trantor's built-in conversion first
            return trantor::Date::fromDbStringLocal(dbDateString);
        } catch (...) {
            // If that fails, try other formats
            try {
                // If it's a timestamp in milliseconds stored as string
                auto ms = std::stoll(dbDateString);
                return trantor::Date(ms);
            } catch (...) {
                // Default to current time if all conversions fail
                return trantor::Date::now();
            }
        }
    }
    
    // Convert a chrono time_point to trantor::Date
    template<typename Clock, typename Duration>
    static trantor::Date timePointToDate(const std::chrono::time_point<Clock, Duration>& tp) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            tp.time_since_epoch()).count();
        return trantor::Date(ms);
    }
    
    // Convert a potentially null database timestamp to trantor::Date
    // Returns current time if the value is null
    template<typename NullableTimestamp>
    static trantor::Date nullableToDate(const NullableTimestamp& nullableTs, bool useCurrentTimeIfNull = true) {
        if (nullableTs) {
            return timePointToDate(*nullableTs);
        } else if (useCurrentTimeIfNull) {
            return trantor::Date::now();
        } else {
            // Use Unix epoch start if not using current time
            return trantor::Date(0);
        }
    }
};

} // namespace utils
} // namespace app
} // namespace comfyui_plus_backend