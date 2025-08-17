#ifndef Const_h
#define Const_h

// Represents the Dexcom session or login status
enum class DexcomStatus
{
    LoggedOut,          // Default, logged out
    LoggedIn,           // Successfully logged in
    SessionNotValid,    // Session ID is null or empty
    SessionNotFound,    // Session not found
    AccountNotFound,    // Account not found
    PasswordInvalid,    // Incorrect or invalid password
    MaxAttempts,        // Maximum authentication attempts exceeded
    UsernameNullEmpty,  // Username null or empty
    PasswordNullEmpty   // Password null or empty
};

enum class GlucoseAdvTrend
{
    DoubleUpHigh,
    DoubleUpInRange,
    DoubleUpLow,

    SingleUpHigh,
    SingleUpInRange,
    SingleUpLow,

    FortyFiveUpHigh,
    FortyFiveUpInRange,
    FortyFiveUpLow,

    FlatHigh,
    FlatInRange,
    FlatLow,

    FortyFiveDownHigh,
    FortyFiveDownInRange,
    FortyFiveDownLow,

    SingleDownHigh,
    SingleDownInRange,
    SingleDownLow,

    DoubleDownHigh,
    DoubleDownInRange,
    DoubleDownLow,

    Unknown
};

// Represents the trend direction of glucose values
enum class GlucoseTrend
{
    DoubleUp,        // Rising Fast
    SingleUp,        // Rising
    FortyFiveUp,     // Slightly Rising
    Flat,            // Steady
    FortyFiveDown,   // Slightly Falling
    SingleDown,      // Falling
    DoubleDown,      // Falling Fast
    NotComputable,   // Value not computable
    RateOutOfRange   // Out of range
};

enum class GlucoseRange
{
    UrgentHigh,
    TooHigh,
    InRange,
    TooLow,
    UrgentLow
};

struct GlucoseData
{
    int glucose = -1; 
    GlucoseTrend trend = GlucoseTrend::NotComputable;
    GlucoseAdvTrend advTrend = GlucoseAdvTrend::Unknown;
    GlucoseRange range = GlucoseRange::InRange;
    unsigned long long timestamp = 0; 
};

static const char *DEXCOM_BASE_URL = "share2.dexcom.com";
static const char *DEXCOM_BASE_URL_OUS = "shareous1.dexcom.com";
static const char *DEXCOM_LOGIN_ID_ENDPOINT = "/ShareWebServices/Services/General/LoginPublisherAccountById";
static const char *DEXCOM_AUTHENTICATE_ENDPOINT = "/ShareWebServices/Services/General/AuthenticatePublisherAccount";
static const char *DEXCOM_VERIFY_SERIAL_NUMBER_ENDPOINT = "/ShareWebServices/Services/Publisher/CheckMonitoredReceiverAssignmentStatus";
static const char *DEXCOM_GLUCOSE_READINGS_ENDPOINT = "/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues";
static const char *DEXCOM_APPLICATION_ID = "d89443d2-327c-4a6f-89e5-496bbb0317db";

static const char *trendStrings[] = {
    "None",         
    "DoubleUp",     
    "SingleUp",     
    "FortyFiveUp",  
    "Flat",         
    "FortyFiveDown",
    "SingleDown",   
    "DoubleDown",   
    "NotComputable",
    "RateOutOfRange"
};

static const char *DEXCOM_TREND_ARROWS[] = {"", "↑↑", "↑", "↗", "→", "↘", "↓", "↓↓", "?", "-"};
static const float MMOL_L_CONVERTION_FACTOR = 0.0555;

#endif