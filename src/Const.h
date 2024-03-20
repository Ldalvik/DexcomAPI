#ifndef Const_h
#define Const_h

enum DexcomStatus_t
{
  LOGGED_OUT,          // Logged out; default value
  LOGGED_IN,           // Succesfully logged in
  SESSION_NOT_VALID,   // Session ID NULL or empty
  SESSION_NOT_FOUND,   // Session not found
  ACCOUNT_NOT_FOUND,   // Account not found
  PASSWORD_INVALID,    // Password incorrect/not valid
  MAX_ATTEMPTS,        // Maximum authentication attempts exceeded
  USERNAME_NULL_EMPTY, // Username NULL or empty
  PASSWORD_NULL_EMPTY  // Password NULL or empty
};

enum GlucoseTrend_t
{
    DOUBLE_UP,        // Rising Fast
    SINGLE_UP,        // Rising
    FORTYFIVE_UP,     // Slightly Rising
    FLAT,             // Steady
    FORTYFIVE_DOWN,   // Slightly Falling
    SINGLE_DOWN,      // Falling
    DOUBLE_DOWN,      // Falling Fast
    NOT_COMPUTABLE,   // Value not computable
    RATE_OUT_OF_RANGE // Out of range
};

enum GlucoseRange_t
{
    URGENT_HIGH,
    TOO_HIGH,
    IN_RANGE,
    TOO_LOW,
    URGENT_LOW,
};

// This is not needed but im gonna keep it, for now
enum AdvGlucoseTrend_t
{
    DOUBLE_UP_HIGH,
    DOUBLE_UP_INRANGE,
    DOUBLE_UP_LOW,

    SINGLE_UP_HIGH,
    SINGLE_UP_INRANGE,
    SINGLE_UP_LOW,

    FORTYFIVE_UP_HIGH,
    FORTYFIVE_UP_INRANGE,
    FORTYFIVE_UP_LOW,

    FLAT_HIGH,
    FLAT_INRANGE,
    FLAT_LOW,

    FORTYFIVE_DOWN_HIGH,
    FORTYFIVE_DOWN_INRANGE,
    FORTYFIVE_DOWN_LOW,

    SINGLE_DOWN_HIGH,
    SINGLE_DOWN_INRANGE,
    SINGLE_DOWN_LOW,

    DOUBLE_DOWN_HIGH,
    DOUBLE_DOWN_INRANGE,
    DOUBLE_DOWN_LOW,

    UNKNOWN
};

static const char *DEXCOM_BASE_URL = "share2.dexcom.com";
static const char *DEXCOM_BASE_URL_OUS = "shareous1.dexcom.com";
static const char *DEXCOM_LOGIN_ID_ENDPOINT = "/ShareWebServices/Services/General/LoginPublisherAccountById";
static const char *DEXCOM_AUTHENTICATE_ENDPOINT = "/ShareWebServices/Services/General/AuthenticatePublisherAccount";
static const char *DEXCOM_VERIFY_SERIAL_NUMBER_ENDPOINT = "/ShareWebServices/Services/Publisher/CheckMonitoredReceiverAssignmentStatus";
static const char *DEXCOM_GLUCOSE_READINGS_ENDPOINT = "/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues";
static const char *DEXCOM_APPLICATION_ID = "d89443d2-327c-4a6f-89e5-496bbb0317db";

static const char *trendStrings[] = {
    "None",          // NONE
    "DoubleUp",      // DOUBLE_UP
    "SingleUp",      // SINGLE_UP
    "FortyFiveUp",   // FORTYFIVE_UP
    "Flat",          // FLAT
    "FortyFiveDown", // FORTYFIVE_DOWN
    "SingleDown",    // SINGLE_DOWN
    "DoubleDown",    // DOUBLE_DOWN
    "NotComputable", // NOT_COMPUTABLE
    "RateOutOfRange" // RATE_OUT_OF_RANGE
};

static const char *DEXCOM_TREND_ARROWS[] = {"", "↑↑", "↑", "↗", "→", "↘", "↓", "↓↓", "?", "-"};
static const float MMOL_L_CONVERTION_FACTOR = 0.0555;

#endif