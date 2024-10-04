// MQTT Defines
#define MQTT_SERVER         "192.168.0.192"
#define MQTT_SERVER_TEST    "192.168.1.0"
#define MQTT_PORT           "1883"

// timing
#define INTERVAL1  60000 // 60 sec = 1Min
#define INTERVAL2  500
#define INTERVAL3  600000 // 600 sec  = 10min

// ADC-Filter
#define RATIO 0.90

//
// The name of this project.
//
// Used for:
//   Access-Point name, in config-mode
//   OTA name.
//
#define PROJECT_NAME "NTP-CLOCK"

//
// Pin definitions for TM1637 and can be changed to other ports
//
#define CLK          D1
#define DIO          D2

// Pin Definition 
#define BUZZER_PIN   D3
#define PIR_PIN      D7
#define BUTTON_PIN_0 D4
#define BUTTON_PIN_1 D5
#define BUTTON_PIN_2 D6