#include "Dexcom.h"

Dexcom::Dexcom(const char *username, const char *password, bool ous)
{
  _username = username;
  _password = password;

  if (ous)
  {
    _base_url = DEXCOM_BASE_URL_OUS;
  }
}

bool Dexcom::createSession()
{
  String accountId = this->getAccountId();
  _session_id = this->getSessionId(accountId);

  if (accountId != "" && _session_id != "")
  {
    currentStatus = LOGGED_IN;
    return true;
  }
  return false;
}

String Dexcom::getAccountId()
{
  char postData[256];
  snprintf(postData,
           sizeof(postData),
           "{\"accountName\":\"%s\",\"password\":\"%s\",\"applicationId\":\"%s\"}",
           _username, _password, DEXCOM_APPLICATION_ID);

  String result = this->post(DEXCOM_AUTHENTICATE_ENDPOINT, postData);
  return this->stripToken(result);
}

String Dexcom::getSessionId(String accountId)
{
  char postData[256];
  snprintf(postData,
           sizeof(postData),
           "{\"accountId\":\"%s\",\"password\":\"%s\",\"applicationId\":\"%s\"}",
           accountId.c_str(), _password, DEXCOM_APPLICATION_ID);

  String result = this->post(DEXCOM_LOGIN_ID_ENDPOINT, postData);
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

  client.println(httpRequest);

  String response;
  while (client.connected())
  {
    if (client.available())
    {
      response += client.readStringUntil('\n');
      response += '\n';
    }
  }
  client.stop();

  int responseCode = response.substring(response.indexOf(" ") + 1, response.indexOf(" ") + 4).toInt();

  if (responseCode == 500)
  {
    String json = response.substring(response.indexOf('{'), response.lastIndexOf('}'));

    if (json.indexOf("SessionNotValid") != -1)
    {
      currentStatus = SESSION_NOT_VALID;
    }
    if (json.indexOf("sessionIdNotFound") != -1)
    {
      currentStatus = SESSION_NOT_FOUND;
    }
    if (json.indexOf("SSO_AuthenticateAccountNotFound") != -1)
    {
      currentStatus = ACCOUNT_NOT_FOUND;
    }
    if (json.indexOf("AccountPasswordInvalid") != -1)
    {
      currentStatus = PASSWORD_INVALID;
    }
    if (json.indexOf("SSO_AuthenticateMaxAttemptsExceeed") != -1)
    {
      currentStatus = MAX_ATTEMPTS;
    }
    if (json.indexOf("InvalidArgument") != -1)
    {
      if (json.indexOf("accountName") != -1)
      {
        currentStatus = USERNAME_NULL_EMPTY;
      }
      else if (json.indexOf("password") != -1)
      {
        currentStatus = PASSWORD_NULL_EMPTY;
      }
    }
    return "";
  }

  if (responseCode != 200)
    return "";
  return response;
}

// bool Dexcom::updateList(int minutes, int maxCount)
// {
//   String postData = "{\"sessionId\":\"" + _session_id +
//                     "\",\"minutes\":" + minutes + ",\"maxCount\":" + maxCount + "}";
//   String results = this->post(DEXCOM_GLUCOSE_READINGS_ENDPOINT, postData.c_str());

//   String json = results.substring(results.lastIndexOf('['), results.lastIndexOf(']') + 1);
//   DynamicJsonDocument doc(1024);
//   DeserializationError error = deserializeJson(doc, json.c_str());
//   if (error)
//   {
//     Serial.println(String(error.f_str()));
//     return false;
//   }

//   JsonArray array = doc.as<JsonArray>();

//   int *glucose = {};
//   String *trend = {};
//   TrendTypes *currentTrend = {};
//   AdvTrendTypes *currentAdvTrend = {};

//   for (JsonVariant v : array)
//   {
//     JsonObject object = v.as<JsonObject>();
//     // currentGlucose = obj["Value"];
//     // String trend = obj["Trend"];
//     // currentTrend = this->getTrendType(trend);
//     // currentAdvTrend = this->getAdvTrendType(currentTrend, currentGlucose);
//   }

//   return true;
// }

bool Dexcom::update()
{
  String postData = "{\"sessionId\":\"" + _session_id +
                    "\",\"minutes\":" + 10 + ",\"maxCount\":" + 1 + "}";
  String results = this->post(DEXCOM_GLUCOSE_READINGS_ENDPOINT, postData.c_str());

  String json = results.substring(results.lastIndexOf('['), results.lastIndexOf(']') + 1);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json.c_str());
  if (error)
  {
    Serial.println(String(error.f_str()));
    return false;
  }

  currentGlucose = doc[0]["Value"];
  String trend = doc[0]["Trend"];
  currentTrend = this->getTrendType(trend);
  currentAdvTrend = this->getAdvTrendType();
  currentRange = this->getRange();

  return currentGlucose > 0;
}

TrendTypes Dexcom::getTrendType(String trend)
{
  if (trend == "DoubleUp")
    return DOUBLE_UP;
  if (trend == "SingleUp")
    return SINGLE_UP;
  if (trend == "FortyFiveUp")
    return FORTYFIVE_UP;
  if (trend == "Flat")
    return FLAT;
  if (trend == "FortyFiveDown")
    return FORTYFIVE_DOWN;
  if (trend == "SingleDown")
    return SINGLE_DOWN;
  if (trend == "DoubleDown")
    return DOUBLE_DOWN;
  if (trend == "NotComputable")
    return NOT_COMPUTABLE;
  if (trend == "RateOutOfRange")
    return RATE_OUT_OF_RANGE;
  return NOT_COMPUTABLE;
}

AdvTrendTypes Dexcom::getAdvTrendType()
{
  switch (currentTrend)
  {
  case DOUBLE_UP:
  {
    if (currentGlucose > highThreshold)
    {
      return DOUBLE_UP_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return DOUBLE_UP_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return DOUBLE_UP_LOW;
    }
  }
  break;
  case SINGLE_UP:
  {
    if (currentGlucose > highThreshold)
    {
      return SINGLE_UP_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return SINGLE_UP_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return SINGLE_UP_LOW;
    }
  }
  break;
  case FORTYFIVE_UP:
  {
    if (currentGlucose > highThreshold)
    {
      return FORTYFIVE_UP_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return FORTYFIVE_UP_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return FORTYFIVE_UP_LOW;
    }
  }
  break;
  case FLAT:
  {
    if (currentGlucose > highThreshold)
    {
      return FLAT_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return FLAT_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return FLAT_LOW;
    }
  }
  break;
  case FORTYFIVE_DOWN:
  {
    if (currentGlucose > highThreshold)
    {
      return FORTYFIVE_DOWN_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return FORTYFIVE_DOWN_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return FORTYFIVE_DOWN_LOW;
    }
  }
  break;
  case SINGLE_DOWN:
  {
    if (currentGlucose > highThreshold)
    {
      return SINGLE_DOWN_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return SINGLE_DOWN_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return SINGLE_DOWN_LOW;
    }
  }
  break;
  case DOUBLE_DOWN:
  {
    if (currentGlucose > highThreshold)
    {
      return DOUBLE_DOWN_HIGH;
    }
    else if (currentGlucose < highThreshold && currentGlucose > lowThreshold)
    {
      return DOUBLE_DOWN_INRANGE;
    }
    else if (currentGlucose < lowThreshold)
    {
      return DOUBLE_DOWN_LOW;
    }
  }
  break;
  }
  return UNKNOWN;
}

RangeTypes Dexcom::getRange()
{
  if (currentGlucose > (highThreshold + warningThreshold) ||
      currentGlucose < (lowThreshold - warningThreshold))
  {
    return ATTENTION_REQUIRED;
  }

  if (currentGlucose >= highThreshold)
  {
    return TOO_HIGH;
  }
  else if (currentGlucose >= lowThreshold)
  {
    return IN_RANGE;
  }
  else
  {
    return TOO_LOW;
  }
}

void Dexcom::setLowThreshold(int threshold)
{
  lowThreshold = threshold;
}

void Dexcom::setHighThreshold(int threshold)
{
  highThreshold = threshold;
}

void Dexcom::setWarningThreshold(int threshold)
{
  warningThreshold = threshold;
}
