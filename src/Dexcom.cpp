#include "Dexcom.h"

Dexcom::Dexcom(bool ous)
{
  if (ous)
  {
    _base_url = DEXCOM_BASE_URL_OUS;
  }
}

bool Dexcom::createSession(const String &username, const String &password)
{
  String accountId = getAccountId(username, password);
  _session_id = getSessionId(accountId, password);

  if (accountId != "" && _session_id != "")
  {
    accountStatus = DexcomStatus::LoggedIn;
    return true;
  }
  return false;
}

String Dexcom::getAccountId(const String &username, const String &password)
{
  String postData = "{\"accountName\":\"" + username +
                    "\",\"password\":\"" + password +
                    "\",\"applicationId\":\"" + DEXCOM_APPLICATION_ID + "\"}";

  String result = this->post(DEXCOM_AUTHENTICATE_ENDPOINT, postData.c_str());
  return this->stripToken(result);
}

String Dexcom::getSessionId(const String &accountId, const String &password)
{
  String postData = "{\"accountId\":\"" + accountId +
                    "\",\"password\":\"" + password +
                    "\",\"applicationId\":\"" + DEXCOM_APPLICATION_ID + "\"}";

  String result = this->post(DEXCOM_LOGIN_ID_ENDPOINT, postData.c_str());
  return this->stripToken(result);
}

String Dexcom::stripToken(String data)
{
  String token = "";
  int bodyStartIndex = data.indexOf("\"");
  int bodyEndIndex = data.lastIndexOf("\"");
  if (bodyStartIndex != -1 && bodyEndIndex != -1)
  {
    token = data.substring(bodyStartIndex + 1, bodyEndIndex);
  }
  return token;
}

String Dexcom::post(const char *url, const char *postData)
{
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(_base_url.c_str(), 443))
  {
    Serial.println("Connection to host failed");
    return "";
  }

  char httpRequest[512];
  snprintf(httpRequest, sizeof(httpRequest),
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Content-Type: application/json\r\n"
           "Accept: application/json\r\n"
           "Content-Length: %d\r\n"
           "Connection: close\r\n"
           "\r\n"
           "%s\r\n",
           url, _base_url.c_str(), strlen(postData), postData);

  client.print(httpRequest);

  bool isChunked = false;
  int responseCode = 0;
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    line.trim();

    if (line.startsWith("HTTP/1.1"))
    {
      int firstSpace = line.indexOf(' ');
      responseCode = line.substring(firstSpace + 1, firstSpace + 4).toInt();
    }
    else if (line.equalsIgnoreCase("Transfer-Encoding: chunked"))
    {
      isChunked = true;
    }
  
    if (line.length() == 0)
      break;
  }

  String body;
  if (isChunked)
  {
    while (true)
    {
      String lenStr = client.readStringUntil('\n');
      lenStr.trim();
      int chunkSize = strtol(lenStr.c_str(), NULL, 16);
      if (chunkSize <= 0)
        break;

      for (int i = 0; i < chunkSize; i++)
      {
        while (!client.available())
          delay(1);
        body += (char)client.read();
      }
      client.read();
      client.read();
    }
  }
  else
  {
    while (client.available())
    {
      body += client.readString();
    }
  }
  client.stop();

  if (responseCode == 500)
  {
    String json = body.substring(body.indexOf('{'), body.lastIndexOf('}') + 1);

    if (json.indexOf("SessionNotValid") != -1) accountStatus = DexcomStatus::SessionNotValid;
    if (json.indexOf("sessionIdNotFound") != -1) accountStatus = DexcomStatus::SessionNotFound;
    if (json.indexOf("SSO_AuthenticateAccountNotFound") != -1) accountStatus = DexcomStatus::AccountNotFound;
    if (json.indexOf("AccountPasswordInvalid") != -1) accountStatus = DexcomStatus::PasswordInvalid;
    if (json.indexOf("SSO_AuthenticateMaxAttemptsExceeed") != -1) accountStatus = DexcomStatus::MaxAttempts;

    if (json.indexOf("InvalidArgument") != -1)
    {
      if (json.indexOf("accountName") != -1) accountStatus = DexcomStatus::UsernameNullEmpty;
      else if (json.indexOf("password") != -1) accountStatus = DexcomStatus::PasswordNullEmpty;
    }
    return "";
  }

  if (responseCode != 200) return "";
  return body;
}
std::vector<GlucoseData> Dexcom::getGlucose(int minutes, int maxCount)
{
  String postData = "{\"sessionId\":\"" + _session_id +
                    "\",\"minutes\":" + minutes + ",\"maxCount\":" + maxCount + "}";
  String results = this->post(DEXCOM_GLUCOSE_READINGS_ENDPOINT, postData.c_str());

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, results);
  if (error)
  {
    Serial.printf("JSON parse error: %s\n", error.c_str());
    return {};
  }

  JsonArray array = doc.as<JsonArray>();
  if (array.isNull() || array.size() == 0)
  {
    Serial.println("No data");
    return {};
  }

  size_t count = array.size();
  std::vector<GlucoseData> data;
  data.reserve(count);

  for (size_t i = 0; i < count; i++)
  {
    JsonObject obj = array[i];

    int glucose = obj["Value"] | -1;
    String strTrend = obj["Trend"] | "";
    GlucoseTrend trend = getTrendType(strTrend);

    unsigned long long timestamp = 0;
    String wtStr = obj["WT"] | "";
    if (wtStr.startsWith("Date("))
    {
      String inner = wtStr.substring(5, wtStr.indexOf(')'));
      inner.replace("-", "");
      inner.replace("+", "");
      timestamp = strtoull(inner.c_str(), nullptr, 10);
    }

    GlucoseRange range = getRange(glucose);
    GlucoseAdvTrend advTrend = getAdvTrendType(glucose, trend);

    data.push_back({glucose, trend, advTrend, range, timestamp});
  }

  return data;
}

GlucoseData Dexcom::getLastGlucose()
{
  auto data = getGlucose(10, 1);
  if (data.empty())
    return {-1, GlucoseTrend::NotComputable, GlucoseAdvTrend::Unknown, GlucoseRange::InRange, 0};

  return data[0];
}

GlucoseTrend Dexcom::getTrendType(const String &trend)
{
  if (trend == "DoubleUp") return GlucoseTrend::DoubleUp;
  if (trend == "SingleUp") return GlucoseTrend::SingleUp;
  if (trend == "FortyFiveUp") return GlucoseTrend::FortyFiveUp;
  if (trend == "Flat") return GlucoseTrend::Flat;
  if (trend == "FortyFiveDown") return GlucoseTrend::FortyFiveDown;
  if (trend == "SingleDown") return GlucoseTrend::SingleDown;
  if (trend == "DoubleDown") return GlucoseTrend::DoubleDown;
  if (trend == "NotComputable") return GlucoseTrend::NotComputable;
  if (trend == "RateOutOfRange") return GlucoseTrend::RateOutOfRange;
  return GlucoseTrend::NotComputable;
}

GlucoseAdvTrend Dexcom::getAdvTrendType(int glucose, GlucoseTrend trend)
{
  switch (trend)
  {
  case GlucoseTrend::DoubleUp:
    if (glucose > HIGH_THRESHOLD) return GlucoseAdvTrend::DoubleUpHigh;
    else if (glucose > LOW_THRESHOLD) return GlucoseAdvTrend::DoubleUpInRange;
    else return GlucoseAdvTrend::DoubleUpLow;

  case GlucoseTrend::SingleUp:
    if (glucose > HIGH_THRESHOLD)return GlucoseAdvTrend::SingleUpHigh;
    else if (glucose > LOW_THRESHOLD)return GlucoseAdvTrend::SingleUpInRange;
    else return GlucoseAdvTrend::SingleUpLow;

  case GlucoseTrend::FortyFiveUp:
    if (glucose > HIGH_THRESHOLD) return GlucoseAdvTrend::FortyFiveUpHigh;
    else if (glucose > LOW_THRESHOLD) return GlucoseAdvTrend::FortyFiveUpInRange;
    else return GlucoseAdvTrend::FortyFiveUpLow;

  case GlucoseTrend::Flat:
    if (glucose > HIGH_THRESHOLD) return GlucoseAdvTrend::FlatHigh;
    else if (glucose > LOW_THRESHOLD) return GlucoseAdvTrend::FlatInRange;
    else return GlucoseAdvTrend::FlatLow;

  case GlucoseTrend::FortyFiveDown:
    if (glucose > HIGH_THRESHOLD) return GlucoseAdvTrend::FortyFiveDownHigh;
    else if (glucose > LOW_THRESHOLD) return GlucoseAdvTrend::FortyFiveDownInRange;
    else return GlucoseAdvTrend::FortyFiveDownLow;

  case GlucoseTrend::SingleDown:
    if (glucose > HIGH_THRESHOLD) return GlucoseAdvTrend::SingleDownHigh;
    else if (glucose > LOW_THRESHOLD) return GlucoseAdvTrend::SingleDownInRange;
    else return GlucoseAdvTrend::SingleDownLow;

  case GlucoseTrend::DoubleDown:
    if (glucose > HIGH_THRESHOLD) return GlucoseAdvTrend::DoubleDownHigh;
    else if (glucose > LOW_THRESHOLD) return GlucoseAdvTrend::DoubleDownInRange;
    else return GlucoseAdvTrend::DoubleDownLow;

  default: return GlucoseAdvTrend::Unknown;
  }
}

GlucoseRange Dexcom::getRange(int glucose)
{
  if (glucose > URGENT_HIGH_THRESHOLD) return GlucoseRange::UrgentHigh;
  if (glucose < URGENT_LOW_THRESHOLD) return GlucoseRange::UrgentLow;
  if (glucose >= HIGH_THRESHOLD) return GlucoseRange::TooHigh;
  if (glucose >= LOW_THRESHOLD) return GlucoseRange::InRange;

  return GlucoseRange::TooLow;
}
