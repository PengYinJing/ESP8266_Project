/*
 * ESP8266MqttClient.ino
 * 
 * Main file for mqtt publisher & subscriber in ESP8266.
 * 
 * Created by:
 *      K.C.Y
 * Date:
 *      2017/07
 */
#include "Global.h"

#include <ESP8266WiFi.h>
#if MQTT_SSL_EN
#include <WiFiClientSecure.h>
#else
#include <WiFiClient.h>
#endif /* MQTT_SSL_EN */
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>

extern "C" {
  #include <spi_flash.h>
}

#include <MQTTClient.h>
#include "SimpleTimer.h"
#include "Key.h"

#if defined(USE_DHT11) || defined(USE_DHT22)
#include "DHT.h"
#endif

#ifdef USE_TSL2561
#include "TSL2561.h"
#endif

#ifdef USE_ULTRASONIC
#include "Ultrasonic.h"
#endif

#if defined(USE_MOISTURE)
#include "Moisture.h"
#elif defined(USE_AIR)
#include "AirQuality.h"
#endif

#ifdef USE_I2C_SOIL
#include "I2CSoilMoistureSensor.h"
#endif

#ifdef USE_DS18B20
#include "DallasTemperature.h"
#endif

struct wifi_config {
  char ssid[32];
  char psk[64];
};

#if MQTT_SSL_EN
const uint8_t root_ca[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
"-----END CERTIFICATE-----\n";
WiFiClientSecure g_WifiClient;
#else
WiFiClient g_WifiClient;
#endif /* MQTT_SSL_EN */

MQTTClient g_MQTTClient;
String https_finger_print = "9D 0A 9B FB CB B3 F2 5F A5 03 2C DB 64 54 7B 01 D3 70 E0 49";
char g_DeviceID[256];

#define ASIZE(a) (sizeof a / sizeof a[0])

//#ifdef USE_SHT10
//#define SHT10DATA 2
//#define SHT10CLOCK 3
//Sensirion sht10(SHT10DATA, SHT10CLOCK);
//#endif

#if defined(USE_DHT11)
DHT dht11_sensor(DHT11_PIN, DHT11);
#elif defined(USE_DHT22)
DHT dht22_sensor(DHT22_PIN, DHT22);
#endif

#ifdef USE_TSL2561
TSL2561 tsl2561_sensor(TSL2561_SCL_PIN, TSL2561_SDA_PIN);
#endif

#ifdef USE_ULTRASONIC
Ultrasonic ultrasonic_sensor(ULTRASONIC_PIN);
#endif

#if defined(USE_MOISTURE)
Moisture moisture_sensor(MOISTURE_PIN);
#elif defined(USE_AIR)
AirQuality air_sensor(AIR_PIN);
#endif

#ifdef USE_HCSR04
static int hcsr04_trig_pin = HCSR04_TRIG_PIN;
static int hcsr04_echo_pin = HCSR04_ECHO_PIN;
#endif

#ifdef USE_I2C_SOIL
I2CSoilMoistureSensor i2c_soil_sensor;
#endif

#ifdef USE_DS18B20
OneWire ds18b20_onewire(DS18B20_PIN);
DallasTemperature ds18b20_sensor(&ds18b20_onewire);
#endif

/* Global variables */
SimpleTimer g_SimpleTimer;
String g_LastBootTime;
int g_WifiReconnectedCount = 0;
int g_MqttReconnectedCount = 0;
bool g_FirmwareUpgradeStarted = false;

void wifi_mqtt_check();
void check_upgrade();
void send_wifi_dbm();
void send_dht_temp_hum();
void send_tsl2561_lux();
void send_ultrasonic_range();
void send_moisture();
void send_air_quality();
void send_hcsr04_distance();
void send_i2c_soil_moisture();
void send_ds18b20_temp();
//void send_soil_temp_hum_dew();

struct {
  int timer; //XXX move to end of struct
  void (*fn)();
  int initial_delay_ms;
  int interval_ms;
} g_Tasks[] =
{
  { 0, wifi_mqtt_check, 500, 3000 },
  { 0, check_upgrade, 0, 1000 },
#ifdef USE_NET_ESP8266
  { 0, send_wifi_dbm, 5000, PUBLISH_TIME_INTERVAL_MS },
#endif
#if defined(USE_DHT11) || defined(USE_DHT22)
  { 0, send_dht_temp_hum, 10000, PUBLISH_TIME_INTERVAL_MS },
#endif
#ifdef USE_TSL2561
  { 0, send_tsl2561_lux, 15000, PUBLISH_TIME_INTERVAL_MS },
#endif
#ifdef USE_ULTRASONIC
  { 0, send_ultrasonic_range, 20000, PUBLISH_TIME_INTERVAL_MS },
#endif
#if defined(USE_MOISTURE)
  { 0, send_moisture, 25000, PUBLISH_TIME_INTERVAL_MS },
#elif defined(USE_AIR)
  { 0, send_air_quality, 25000, PUBLISH_TIME_INTERVAL_MS },
#endif
#ifdef USE_HCSR04
  { 0, send_hcsr04_distance, 30000, PUBLISH_TIME_INTERVAL_MS },
#endif
#ifdef USE_I2C_SOIL
  { 0, send_i2c_soil_moisture, 35000, PUBLISH_TIME_INTERVAL_MS },
#endif
#ifdef USE_DS18B20
  { 0, send_ds18b20_temp, 40000, PUBLISH_TIME_INTERVAL_MS },
#endif
};

int mqtt_connect_timer;

void publish_sensor_data(const char* sensor_name, int value)
{
  char topic[strlen(g_DeviceID) + strlen(sensor_name) + strlen(SENSOR_TOPIC_PREFIX) + 1];
  char payload[8];
  sprintf(topic, SENSOR_TOPIC_FMT_STR, g_DeviceID, sensor_name);
  sprintf(payload, "%d", value);
#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.print(", payload=");
  Serial.println(payload);
#endif
  g_MQTTClient.publish(true, topic, payload);
}

void publish_sensor_data_f(const char* sensor_name, float value)
{
  char topic[strlen(g_DeviceID) + strlen(sensor_name) + strlen(SENSOR_TOPIC_PREFIX) + 1];
  char payload[8];
  sprintf(topic, SENSOR_TOPIC_FMT_STR, g_DeviceID, sensor_name);
  dtostrf(value, 2, 1, payload);
#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.print(", payload=");
  Serial.println(payload);
#endif
  g_MQTTClient.publish(true, topic, payload);
}

void publish_boot_time(String bootTime)
{
  char topic[strlen(g_DeviceID) + strlen(LAST_BOOT_TIME_TOPIC_PREFIX) + 1];
  sprintf(topic, LAST_BOOT_TIME_TOPIC_FMT_STR, g_DeviceID);
#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.println(", payload=" + bootTime);
#endif
  g_MQTTClient.publish(true, topic, bootTime.c_str());
}

void publish_version()
{
  char topic[strlen(g_DeviceID) + strlen(VERSION_TOPIC_PREFIX) + 1];
  char payload[32];

  sprintf(topic, VERSION_TOPIC_FMT_STR, g_DeviceID);
#if defined(USE_MOISTURE)
  strcpy(payload, "version1");
#elif defined(USE_AIR)
  strcpy(payload, "version2");
#else
  strcpy(payload, "version3");
#endif

#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.print(", payload=");
  Serial.println(payload);
#endif
  g_MQTTClient.publish(true, topic, payload);
}

void publish_wifi_dbm(int dbm)
{
  char topic[strlen(g_DeviceID) + strlen(WIFI_DBM_TOPIC_PREFIX) + 1];
  char payload[8];
  sprintf(topic, WIFI_DBM_TOPIC_FMT_STR, g_DeviceID);
  sprintf(payload, "%d", dbm);

#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.print(", payload=");
  Serial.println(payload);
#endif
  g_MQTTClient.publish(true, topic, payload);
}

void publish_wifi_reconnection(int count)
{
  char topic[strlen(g_DeviceID) + strlen(WIFI_RECONN_TOPIC_PREFIX) + 1];
  char payload[8];
  sprintf(topic, WIFI_RECONN_TOPIC_FMT_STR, g_DeviceID);
  sprintf(payload, "%d", count);

#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.print(", payload=");
  Serial.println(payload);
#endif
  g_MQTTClient.publish(true, topic, payload);
}

void publish_mqtt_reconnection(int count)
{
  char topic[strlen(g_DeviceID) + strlen(MQTT_RECONN_TOPIC_PREFIX) + 1];
  char payload[8];
  sprintf(topic, MQTT_RECONN_TOPIC_FMT_STR, g_DeviceID);
  sprintf(payload, "%d", count);

#if PUBLISH_LOG_EN
  Serial.print("Publishing : topic=");
  Serial.print(topic);
  Serial.print(", payload=");
  Serial.println(payload);
#endif
  g_MQTTClient.publish(true, topic, payload);
}

void messageReceived(String topicString, String payloadString, char *payload, unsigned int length)
{
  char* topic = (char*)topicString.c_str();
  Serial.print("Subscribed topic: ");
  Serial.println(topic);

  char fw_upgrade_status_topic[strlen(g_DeviceID) + strlen(FW_UPGRADE_STATUS_TOPIC_PREFIX) + 1];
  char fw_upgrade_loc_topic[strlen(g_DeviceID) + strlen(FW_UPGRADE_LOC_TOPIC_PREFIX) + 1];
  
  sprintf(fw_upgrade_status_topic, FW_UPGRADE_STATUS_TOPIC_FMT_STR, g_DeviceID);
  sprintf(fw_upgrade_loc_topic, FW_UPGRADE_LOC_TOPIC_FMT_STR, g_DeviceID);
  
  if (!strcmp(topic, fw_upgrade_status_topic))
  {
    if (!g_FirmwareUpgradeStarted && !strncmp((const char*)payload, "start", strlen("start")))
    {
      g_FirmwareUpgradeStarted = true;
      g_MQTTClient.publish(false, fw_upgrade_status_topic, "starting");
      g_MQTTClient.subscribe(fw_upgrade_loc_topic);
    }
  }
  else if (!strcmp(topic, fw_upgrade_loc_topic))
  {
    g_MQTTClient.unsubscribe(fw_upgrade_loc_topic);
    
    Serial.print("firmware location: ");
    Serial.println(payloadString);

    if (payloadString.length() == 0)
    {
      g_FirmwareUpgradeStarted = false;
      Serial.println("Upgrading failed (Firmware URL is empty)");
      for (int i = 2; i < ASIZE(g_Tasks); i++)
          g_SimpleTimer.enable(g_Tasks[i].timer);
    }
    else
    {
      g_MQTTClient.publish(false, fw_upgrade_status_topic, "downloading");
      
      t_httpUpdate_return ret;
      if (!strncmp(payloadString.c_str(), "https://", strlen("https://")))
        ret = ESPhttpUpdate.update(payloadString, "", https_finger_print);
      else
        ret = ESPhttpUpdate.update(payloadString);

      if (ret == HTTP_UPDATE_OK)
      {
        Serial.println("Upgrade successed!!!");
        g_MQTTClient.publish(false, fw_upgrade_status_topic, "download_complete");
        delay(1000);
        g_MQTTClient.publish(false, fw_upgrade_status_topic, "rebooting");
        delay(1000);
        ESP.restart();
      }
      else
      {
        g_FirmwareUpgradeStarted = false;
        Serial.println("Upgrade failed!!!");
        g_MQTTClient.publish(false, fw_upgrade_status_topic, "download_fail");
        for (int i = 2; i < ASIZE(g_Tasks); i++)
          g_SimpleTimer.enable(g_Tasks[i].timer);
      }
    }
  }
}

#if defined(USE_DHT11) || defined(USE_DHT22)
void send_dht_temp_hum()
{
  float h = 0, t =  0;
#if defined(USE_DHT11)
  if (dht11_sensor.readValue(&h, &t))
  {
    publish_sensor_data_f(DHT11_HUM_SENSOR_NAME, h);
    publish_sensor_data_f(DHT11_TEMP_SENSOR_NAME, t);
  }
#elif defined(USE_DHT22)
  if (dht22_sensor.readValue(&h, &t))
  {
    publish_sensor_data_f(DHT22_HUM_SENSOR_NAME, h);
    publish_sensor_data_f(DHT22_TEMP_SENSOR_NAME, t);
  }
#endif
}
#endif

#ifdef USE_TSL2561
void send_tsl2561_lux()
{
  uint16_t ir, full;
  uint32_t lum, lux;
  lum = tsl2561_sensor.getFullLuminosity();
  ir = lum >> 16;
  full = lum & 0xFFFF;
  lux = tsl2561_sensor.calculateLux(full, ir);

  publish_sensor_data(TSL2561_IR_SENSOR_NAME, ir);
  publish_sensor_data(TSL2561_LUMENS_SENSOR_NAME, full); // lum
  publish_sensor_data(TSL2561_LUX_SENSOR_NAME, lux);
}
#endif

#ifdef USE_ULTRASONIC
void send_ultrasonic_range()
{
  long range_in_centimeters = ultrasonic_sensor.MeasureInCentimeters();
  publish_sensor_data(ULTRASONIC_SENSOR_NAME, (int)range_in_centimeters);
}
#endif

#if defined(USE_MOISTURE)
void send_moisture()
{
  int moisture = 0;
  if (moisture_sensor.readMoisture(&moisture))
    publish_sensor_data(MOISTURE_SENSOR_NAME, moisture);
}
#elif defined(USE_AIR)
void send_air_quality()
{
  int quality = 0;
  if (air_sensor.readQuality(&quality))
    publish_sensor_data(AIR_QUALITY_SENSOR_NAME, quality);
}
#endif


#ifdef USE_HCSR04
void send_hcsr04_distance()
{
  float sum = 0;
  int samples = 0;
  unsigned long duration;
  while (samples < 8)
  {
    delay(50);
    digitalWrite(hcsr04_trig_pin, LOW);
    delayMicroseconds(5);
    digitalWrite(hcsr04_trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(hcsr04_trig_pin, LOW);

    duration = pulseIn(hcsr04_echo_pin, HIGH);
    if (!duration)
      break;

    sum += (float)duration / 58.0f;
    samples++;
  }
  if (samples == 0)
  {
    Serial.println("HC-SR04 sensor reading fail");
    return;
  }

  publish_sensor_data_f(HCSR04_SENSOR_NAME, sum / samples);
}
#endif

#ifdef USE_I2C_SOIL
void send_i2c_soil_moisture()
{
  while (i2c_soil_sensor.isBusy())
    delay(50);
  int moisture = i2c_soil_sensor.getCapacitance(); //read capacitance register
  float temp = i2c_soil_sensor.getTemperature() / 10.0f; //temperature register
  int light = i2c_soil_sensor.getLight(true); //request light measurement, wait and read light register
  i2c_soil_sensor.sleep();

  publish_sensor_data(I2C_SOIL_MOISTURE_SENSOR_NAME, moisture);
  publish_sensor_data_f(I2C_SOIL_TEMP_SENSOR_NAME, temp);
  publish_sensor_data(I2C_SOIL_LIGHT_SENSOR_NAME, light);
}
#endif

#ifdef USE_DS18B20
void send_ds18b20_temp()
{
  ds18b20_sensor.requestTemperatures();
  float temp = ds18b20_sensor.getTempCByIndex(0);
  publish_sensor_data_f(DS18B20_TEMP_SENSOR_NAME, temp);
}
#endif

void wifi_connect()
{
  wifi_config wc;
  //int retry_count = 10;
  
  EEPROM.begin(512);
  EEPROM.get(0, wc);

  Serial.print("Connecting to ");
  Serial.print(wc.ssid);
  Serial.println("...");

  if (!strlen(wc.ssid))
    wc = wifi_choose_ssid();

  WiFi.mode(WIFI_STA);
  WiFi.begin(wc.ssid, wc.psk);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Wifi connection failed - retrying");
      wc = wifi_choose_ssid();
    WiFi.mode(WIFI_STA);
    WiFi.begin(wc.ssid, wc.psk);
  }

  EEPROM.put(0, wc);
  EEPROM.commit();
  EEPROM.end();

  Serial.println("WiFi connected");

  set_sync_ntp_time();

  if (g_WifiReconnectedCount == 0)
    g_LastBootTime = GetISODateTime();

  g_WifiReconnectedCount++;
}

void mqtt_connect()
{
  bool wifi_reconnect_flag = false;
  if (WiFi.status() != WL_CONNECTED)
  {
    wifi_connect();
    wifi_reconnect_flag = true;
  }

  char user_name[256];
  char password[256];
  /* Read device id from flash */
  spi_flash_read(PARAM_ADDRESS, (uint32_t*)g_DeviceID, 256);
  /* Read user name from flash */
  spi_flash_read(PARAM_ADDRESS + 0x100, (uint32_t*)user_name, 256);
  /* Read password from flash */
  spi_flash_read(PARAM_ADDRESS + 0x200, (uint32_t*)password, 256);

  for (int i = 0; i < 256; i++)
  {
    if (g_DeviceID[i] == 0x0D || g_DeviceID[i] == 0x0A || g_DeviceID[i] == 0xFF)
      g_DeviceID[i] = 0x00;
    if (user_name[i] == 0x0D || user_name[i] == 0x0A || user_name[i] == 0xFF)
      user_name[i] = 0x00;
    if (password[i] == 0x0D || password[i] == 0x0A || password[i] == 0xFF)
      password[i] = 0x00;
  }

  Serial.print("mqtt connecting id: ");
  Serial.println(g_DeviceID);
  Serial.println(user_name);
  Serial.println(password);

  char will_topic[strlen(g_DeviceID) + strlen(WILL_TOPIC_PREFIX) + 1];
  sprintf(will_topic, WILL_TOPIC_FMT_STR, g_DeviceID);
  Serial.print("will_topic: ");
  Serial.println(will_topic);

  g_MQTTClient.setWill(will_topic, "0");
  if (!g_MQTTClient.connect(g_DeviceID, user_name, password))
    return;

  g_MQTTClient.publish(true, will_topic, "1");

  Serial.print("WiFi Reconnection: ");
  Serial.println(g_WifiReconnectedCount);
  Serial.print("MQTT Reconnection: ");
  Serial.println(g_MqttReconnectedCount);
  
  if (!wifi_reconnect_flag && g_WifiReconnectedCount == 1 && g_MqttReconnectedCount == 0)
  {
    /* Publish Last Boot Time & WiFi Reconnection */
    Serial.println("Booting time: " + g_LastBootTime);
    publish_boot_time(g_LastBootTime);
    publish_version();
    publish_wifi_reconnection(g_WifiReconnectedCount);
  }
  else if (wifi_reconnect_flag)
    publish_wifi_reconnection(g_WifiReconnectedCount);
  
  /* Publish MQTT reconnection */
  g_MqttReconnectedCount++;
  publish_mqtt_reconnection(g_MqttReconnectedCount);

#ifdef USE_GPIO
  g_MQTTClient.subscribe("gpio/#");
#endif

  Serial.print(GetISODateTime());
  Serial.println(" : MQTT connected");
  g_SimpleTimer.disable(mqtt_connect_timer);

  for (int i = 0; i < ASIZE(g_Tasks); i++)
  {
    //Serial.print("Interval: ");
    //Serial.println(g_Tasks[i].interval_ms);
    g_Tasks[i].timer = g_SimpleTimer.setInterval(g_Tasks[i].initial_delay_ms, g_Tasks[i].interval_ms, g_Tasks[i].fn);
  }
}

void wifi_mqtt_check()
{
#ifdef USE_NET_ESP8266
  if (WiFi.status() != WL_CONNECTED)
    Serial.println("WiFi disconnected");
  else
#endif
  if (!g_MQTTClient.connected())
  {
    Serial.print(GetISODateTime());
    Serial.println(" : MQTT disconnected");
  }
  else
    return;

  g_SimpleTimer.enable(mqtt_connect_timer);
  for (int i = 0; i < ASIZE(g_Tasks); i++)
    g_SimpleTimer.deleteTimer(g_Tasks[i].timer);
}

void check_upgrade()
{
  char topic[strlen(g_DeviceID) + strlen(FW_UPGRADE_STATUS_TOPIC_PREFIX) + 1];
  sprintf(topic, FW_UPGRADE_STATUS_TOPIC_FMT_STR, g_DeviceID);
  g_MQTTClient.subscribe(topic);
}

#if defined(USE_NET_ESP8266)
const char *wl_enc_to_str(uint8_t e)
{
  switch (e) {
    case ENC_TYPE_NONE: return "NONE";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_AUTO: return "AUTO";
  }
  return "";
}

String decode_string(String str) {
  str.replace("+", " ");
  str.replace("%21", "!");
  str.replace("%22", "");
  str.replace("%23", "#");
  str.replace("%24", "$");
  str.replace("%25", "%");
  str.replace("%26", "&");
  str.replace("%27", "'");
  str.replace("%28", "(");
  str.replace("%29", ")");
  str.replace("%2A", "*");
  str.replace("%2B", "+");
  str.replace("%2C", ",");
  str.replace("%2F", "/");
  str.replace("%3A", ":");
  str.replace("%3B", ";");
  str.replace("%3C", "<");
  str.replace("%3D", "=");
  str.replace("%3E", ">");
  str.replace("%3F", "?");
  str.replace("%40", "@");
  return str;
}

struct wifi_config wifi_choose_ssid()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP("growvisor");
  WiFiServer web(80);
  
  web.begin();

  WiFiClient client;
  while (true)
  {
    client = web.available();
    if (!client || !client.connected())
      continue;

    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.println("/n" + req);
    client.flush();
  
    String s = "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n\r\n"
              "<!DOCTYPE HTML>\r\n<html>\r\n";
    //Check to see if user submitted credentials, otherwise continue to serve up webpage
    if (req.indexOf("msg?ssid") != -1)
    {
      int s1 = req.indexOf("ssid=") + 5;
      int s2 = req.indexOf("&password");
      String ssid = req.substring(s1, s2);
      
      int p1 = s2 + 10;
      int p2 = req.length() - String(" HTTP/1.1").length();
      String password = req.substring(p1, p2);
      
      s += "<h1><center>Configuring ESP8266</center></h1>";
      s += "</html>";
      client.print(s);

      delay(500);
      
      wifi_config wc = { 0 };
      ssid = decode_string(ssid);
      password = decode_string(password);
      strcpy(wc.ssid, ssid.c_str());
      strcpy(wc.psk, password.c_str());
      
      return wc;
    }
    else
    {
       s += 
        "<body>"    
        "<p>"
        "<center>"
        "<h1>ESP8266 WiFi Configuration</h1>"
        "<div>"
        "</div>"
        "<form action='msg'><p>SSID:  <input type='text' name='ssid' size=50 autofocus></p>"
        "<p>Password: <input type='text' name='password' size=50 autofocus></p>"
        "<p><input type='submit' value='Submit'></p>"
        "</form>"
        "</center>";
        s += "</body></html>\n";
        // Send the response to the client
        client.print(s);
    }
  }
}

void send_wifi_dbm()
{
  publish_wifi_dbm(WiFi.RSSI());
}

void config_key_long_pressed()
{
  Serial.println("config_key_long_pressed");
  EEPROM.begin(512);
  memset(EEPROM.getDataPtr(), 0x0, 512);
  EEPROM.commit();
  EEPROM.end();

  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED)
    WiFi.disconnect();
}
#endif

void setup() 
{
  Serial.begin (115200);
  //Serial.begin (74880);
  
  Serial.println("Starting");

#if MQTT_SSL_EN
  g_WifiClient.setCACert(root_ca, sizeof(root_ca));
#endif
  g_MQTTClient.begin(MQTT_SERVER_NAME, MQTT_SERVER_PORT, g_WifiClient);

  key_init(KEY_PIN, 5000, config_key_long_pressed);
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);
  
  wifi_connect();
  // setup i2c for tsl
  Wire.begin();

  mqtt_connect_timer = g_SimpleTimer.setInterval(0, 3000, mqtt_connect);

#if defined(USE_DHT11)
  dht11_sensor.begin();
#elif defined(USE_DHT22)
  dht22_sensor.begin();
#endif

#ifdef USE_TSL2561
  if (tsl2561_sensor.begin())
  {
    tsl2561_sensor.setGain(TSL2561_GAIN_16X);
    tsl2561_sensor.setTiming(TSL2561_INTEGRATIONTIME_13MS);
  }
  else
    Serial.println("No light sensor");
#endif

#if defined(USE_MOISTURE)
  moisture_sensor.begin();
#elif defined(USE_AIR)
  air_sensor.begin();
#endif

#ifdef USE_I2C_SOIL
  i2c_soil_sensor.begin();
#endif

#ifdef USE_DS18B20
  ds18b20_sensor.begin();
#endif

#ifdef USE_HCSR04
  pinMode(hcsr04_trig_pin, OUTPUT);
  pinMode(hcsr04_echo_pin, INPUT);
#endif

  Serial.println("setup complete");
}

void loop()
{
  g_SimpleTimer.run();
  g_MQTTClient.loop();
}

