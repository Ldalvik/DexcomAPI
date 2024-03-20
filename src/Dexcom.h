#ifndef Dexcom_h
#define Dexcom_h

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include "HTTPClient.h"
#include <WiFi.h>
#endif

#include <Const.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

class Dexcom
{
public:
    Dexcom(const char *username, const char *password, bool ous = false);
    Dexcom(bool ous = false);
    bool createSession();
    bool update();
    // bool updateList(int minutes, int maxCount);
    void setLowThreshold(int threshold);
    void setHighThreshold(int threshold);
    void setUrgentLowThreshold(int low);
    void setUrgentHighThreshold(int low);
      
    int currentGlucose = 0;
    GlucoseTrend_t     currentTrend;
    AdvGlucoseTrend_t  currentAdvTrend;
    GlucoseRange_t     currentRange;
    DexcomStatus_t     currentStatus = LOGGED_OUT;

    int highThreshold       = 250; // default value for above range
    int urgentHighThreshold = 300; // default high threshold urgent alarm
    int lowThreshold        = 90;  // default value for below range
    int urgentLowThreshold  = 80;  // default low threshold for urgent alarm

    const char *_username;
    const char *_password;

private:
    String            post(const char *url, const char *postData);
    String            getAccountId();
    String            getSessionId(String accountId);
    String            stripToken(String data);
    GlucoseTrend_t    getTrendType(String trend);
    AdvGlucoseTrend_t getAdvTrendType();
    GlucoseRange_t    getRange();

    String _session_id;
    String _base_url = DEXCOM_BASE_URL;
};

#endif