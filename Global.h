/*
 * Global.h
 * 
 * Defines macros for project.
 * 
 * Created by:
 *      K.C.Y
 * Date:
 *      2017/07
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_
 
//#define USE_NET_ETH
#define USE_NET_ESP8266

#define KEY_PIN        0
#define D0_PIN         14
#define D1_PIN         12
#define D2_PIN         13
#define A0_PIN         17
#define I2C0_SDA_PIN   4
#define I2C0_SCL_PIN   5
#define UART0_TX_PIN   1
#define UART0_RX_PIN   3
#define EXT_PIN        2

/* Macros for Sensors */
#define USE_DHT11
#define USE_TSL2561
#define USE_ULTRASONIC
#define USE_MOISTURE
#define USE_HCSR04
#define USE_I2C_SOIL
#define USE_DS18B20

/* Pins of Sensors Description */
#if defined(USE_DHT11)
#define DHT11_PIN         D0_PIN
#elif defined(USE_DHT22)
#define DHT22_PIN         D0_PIN
#endif
#ifdef USE_TSL2561
#define TSL2561_SCL_PIN   I2C0_SCL_PIN
#define TSL2561_SDA_PIN   I2C0_SDA_PIN
#endif
#ifdef USE_ULTRASONIC
#define ULTRASONIC_PIN    D1_PIN
#endif
#if defined(USE_MOISTURE)
#define MOISTURE_PIN      A0_PIN
#elif defined(USE_AIR)
#define AIR_PIN           A0_PIN
#endif
/* Added new sensor */
#ifdef USE_HCSR04
#define HCSR04_TRIG_PIN   D2_PIN
#define HCSR04_ECHO_PIN   EXT_PIN
#endif
#ifdef USE_I2C_SOIL
#define I2C_SOIL_SCL_PIN  I2C0_SCL_PIN
#define I2C_SOIL_SDA_PIN  I2C0_SDA_PIN
#endif
#ifdef USE_DS18B20
#define DS18B20_PIN       D0_PIN
#endif

/* MQTT Client Information */
#define MQTT_SSL_EN         1

#define MQTT_SERVER_NAME    "your mqtt server name"
#if MQTT_SSL_EN
#define MQTT_SERVER_PORT    8883
#else
#define MQTT_SERVER_PORT    1883
#endif

#if defined(USE_DHT11)
#define DHT11_TEMP_SENSOR_NAME        "dht_11_temperature"
#define DHT11_HUM_SENSOR_NAME         "dht_11_humidity"
#elif defined(USE_DHT22)
#define DHT22_TEMP_SENSOR_NAME        "dht_22_temperature"
#define DHT22_HUM_SENSOR_NAME         "dht_22_humidity"
#endif
#ifdef USE_TSL2561
#define TSL2561_LUX_SENSOR_NAME       "tsl2561_lux"
#define TSL2561_LUMENS_SENSOR_NAME    "tsl2561_lumens"
#define TSL2561_IR_SENSOR_NAME        "tls2561_ir"
#endif
#ifdef USE_ULTRASONIC
#define ULTRASONIC_SENSOR_NAME        "ultrasonic_distance"
#endif
#if defined(USE_MOISTURE)
#define MOISTURE_SENSOR_NAME          "soil_humidity"
#elif defined(USE_AIR)
#define AIR_QUALITY_SENSOR_NAME       "air_quality"
#endif
#ifdef USE_HCSR04
#define HCSR04_SENSOR_NAME            "hcsr04_distance"
#endif
#ifdef USE_I2C_SOIL
#define I2C_SOIL_MOISTURE_SENSOR_NAME          "i2c_soil_moisture"
#define I2C_SOIL_LIGHT_SENSOR_NAME             "i2c_soil_light"
#define I2C_SOIL_TEMP_SENSOR_NAME              "i2c_soil_temperature"
#endif
#ifdef USE_DS18B20
#define DS18B20_TEMP_SENSOR_NAME       "ds18b20_temperature"
#endif

#define PUBLISH_LOG_EN    1
#define PUBLISH_TIME_INTERVAL_MS      30000

/* Flash address */
#define PARAM_ADDRESS      0x7D000

#endif

