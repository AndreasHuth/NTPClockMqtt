// WiFi & over the air updates
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

// eeprom
#include <EEPROM.h>
#include "myTypes.h"

// defines:
#include "defines.h"

int cfgStart = 0;
configData_t cfg;

// Neopixel lib
#include <Adafruit_NeoPixel.h>
#include <neopixel_fun.h>

#include <PubSubClient.h>
// Update these with values suitable for your network.
const char *mqtt_server = MQTT_SERVER;
const char *mqtt_port   = MQTT_PORT;

WiFiClient espClient;
PubSubClient client(espClient);

const char *sub_action = "/hhome/DG/action";
const char *sub_display_clear = "/hhome/DG/displayOff";
const char *sub_display_Brightness = "/hhome/DG/displayBrightness";
const char *sub_display_control = "/hhome/DG/displayControl";
const char *sub_alarm_minute = "/hhome/DG/alarmminute";
const char *sub_alarm_hour = "/hhome/DG/alarmhour";
const char *sub_alarm_on = "/hhome/DG/alarmOn";
const char *sub_alarm_set = "/hhome/DG/SetAlarm";

// For dealing with NTP & the clock.
//
#include "ntp.h"

//
// The display-interface
//
#include "TM1637.h"
TM1637 tm1637(DIO, CLK);

//
// WiFi setup.
//
#include "WiFiManager.h"

//
// Debug messages over the serial console.
//
#define DEBUG 1

//
// Last 20 debug messages.
//
#define DEBUG_MAX 20
String debug_logs[DEBUG_MAX];

//
// Record a debug-message, only if `DEBUG` is defined.
//
void DEBUG_LOG(const char *format, ...)
{
#ifdef DEBUG
  static int last_debug = 0;
  char buff[1024] = {'\0'};
  va_list arguments;
  va_start(arguments, format);
  vsnprintf(buff, sizeof(buff), format, arguments);
  Serial.print(buff);
  Serial.println();

  debug_logs[last_debug] = String(buff);
  last_debug += 1;

  if (last_debug >= DEBUG_MAX)
  {
    last_debug = 0;
  }

  va_end(arguments);
#endif
}

//
// Pin definitions for buzzer
//
int buzzer = BUZZER_PIN; // Buzzer control port
int freq = 2000;

//
// MQTT reconnect
//
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Hey, here is clock!");
      // ... and resubscribe
      client.subscribe(sub_action);
      client.loop();
      client.subscribe(sub_display_clear);
      client.loop();
      client.subscribe(sub_display_Brightness);
      client.loop();
      client.subscribe(sub_display_control);
      client.loop();

      client.subscribe(sub_alarm_minute);
      client.loop();
      client.subscribe(sub_alarm_hour);
      client.loop();
      client.subscribe(sub_alarm_on);
      client.loop();
      client.subscribe(sub_alarm_set);
      client.loop();
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// PIR functions ...
boolean oldState = HIGH;
void setup_pir(void)
{
  pinMode(PIR_PIN, INPUT);
}

//
// ALARM
//
int alarmHour = 0;
int alarmMinute = 0;
boolean AlarmOn = false;

//
// MQTT callback
//
boolean MQTTEvent = false;
int MQTTEventValue = 0;
boolean MQTTsleepMode = false;
boolean MQTTbrightnessControl = false;
int MQTTclockBrightness = 2;

int MQTTclockAlarmHours = 0;
int MQTTclockAlarmMinutes = 0;

int MQTTclockAlarmOn = false;

void callback(char *topic, byte *payload, unsigned int length)
{
  String messageTemp;
  char *charhelp;
  // String help_str_1 = "";
  // String help_str_2 = "";
  int help_int = 0;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
    charhelp += (char)payload[i];
  }
  Serial.println();

  if (strcmp(topic, sub_action) == 0)
  {
    if ((char)payload[0] == '3')
    {
      MQTTEventValue = 3;
      MQTTEvent = true;
      MQTTsleepMode = false;
    }
    else if ((char)payload[0] == '2')
    {
      MQTTEventValue = 2;
      MQTTEvent = true;
      MQTTsleepMode = false;
    }
    else if ((char)payload[0] == '1')
    {
      MQTTEventValue = 1;
      MQTTEvent = true;
      MQTTsleepMode = false;
    }
    else if ((char)payload[0] == '0')
    {
      MQTTEvent = false;
    }
  }

  if (strcmp(topic, sub_display_Brightness) == 0)
  {
    Serial.print("Helligkeit: ");

    help_int = messageTemp.toInt();
    MQTTclockBrightness = (byte)help_int;
    if (MQTTclockBrightness > 7)
      MQTTclockBrightness = 7;

    Serial.println(MQTTclockBrightness);
  }

  if (strcmp(topic, sub_display_clear) == 0)
  {
    if ((char)payload[0] == '1')
    {
      MQTTsleepMode = true;
    }
    else if ((char)payload[0] == '0')
    {
      MQTTsleepMode = false;
    }
  }

  if (strcmp(topic, sub_display_control) == 0)
  {
    if ((char)payload[0] == '1')
    {
      MQTTbrightnessControl = true;
      MQTTclockBrightness = 7;
    }
    else if ((char)payload[0] == '0')
    {
      MQTTbrightnessControl = false;
    }
  }

  if (strcmp(topic, sub_alarm_hour) == 0)
  {
    Serial.print("Alarm hour: ");

    help_int = messageTemp.toInt();
    MQTTclockAlarmHours = (byte)help_int;
    if (MQTTclockAlarmHours > 23)
      MQTTclockAlarmHours = 0;

    Serial.println(MQTTclockAlarmHours);
  }

  if (strcmp(topic, sub_alarm_minute) == 0)
  {
    Serial.print("Alarm minute: ");

    help_int = messageTemp.toInt();
    MQTTclockAlarmMinutes = (byte)help_int;
    if (MQTTclockAlarmMinutes > 59)
      MQTTclockAlarmMinutes = 0;

    Serial.println(MQTTclockAlarmMinutes);
  }

  if (strcmp(topic, sub_alarm_on) == 0)
  {
    if ((char)payload[0] == '1')
    {
      Serial.print("MQTTclockAlarmOn!");
      // MQTTclockAlarmOn = true;
      AlarmOn = true;
    }
    else
    {
      Serial.print("MQTTclockAlarmOff");
      // MQTTclockAlarmOn = false;
      AlarmOn = false;
    }
  }

  if (strcmp(topic, sub_alarm_set) == 0)
  {
    if ((char)payload[0] == '1')
    {
      Serial.print("MQTTclockAlarmSET!");
      MQTTclockAlarmOn = true;
      // AlarmOn = true;
    }
    else
    {
      // Serial.print("MQTTclockAlarmSET");
      // MQTTclockAlarmOn = false;
      // AlarmOn = false;
    }
  }
}

//
// PIR LOOP
//
boolean loop_pir(void)
{

  boolean pirEvent = false;
  boolean newState = digitalRead(PIR_PIN);
  // Check if state changed from high to low (button press).
  if ((newState == HIGH) && (oldState == LOW))
  {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(PIR_PIN);
    if (newState == HIGH)
    { //
      // counter ++;
      pirEvent = true;
      Serial.println("pir1");
      MQTTsleepMode = false;
      client.publish("/hhome/DG/pirevent", String("1").c_str());
    }
  }
  else if ((newState == LOW) && (oldState == HIGH))
  {
    pirEvent = false;
    Serial.println("pir0");
    client.publish("/hhome/DG/pirevent", String("0").c_str());
  }
  oldState = newState;
  return pirEvent;
}

void eraseConfig(void)
{
  // Reset EEPROM bytes to '0' for the length of the data structure
  EEPROM.begin(512);
  for (int i = cfgStart; (unsigned long long) i < sizeof(cfg); i++)
  {
    EEPROM.write(i, 0);
  }
  delay(200);
  EEPROM.commit();
  EEPROM.end();
}

void saveConfig(void)
{
  // Save configuration from RAM into EEPROM
  EEPROM.begin(512);
  EEPROM.put(cfgStart, cfg);
  delay(200);
  EEPROM.commit(); // Only needed for ESP8266 to get data written
  EEPROM.end();    // Free RAM copy of structure
}

void loadConfig(void)
{
  // Loads configuration from EEPROM into RAM
  EEPROM.begin(512);
  EEPROM.get(cfgStart, cfg);
  EEPROM.end();
}

//
// This function is called when the device is powered-on.
//
void setup()
{
  // Enable our serial port.
  Serial.begin(115200);

  loadConfig();
  alarmHour = cfg.eehours;
  alarmMinute = cfg.eeminutes;

  //
  // pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  // digitalWrite(BUILTIN_LED, LOW);  // turn on LED with voltage LOW
  // initialize the display
  tm1637.init();

  // We want to see ":" between the digits.
  tm1637.point(true);

  //
  // Set the intensity - valid choices include:
  //
  //   BRIGHT_DARKEST   = 0
  //   BRIGHT_TYPICAL   = 2
  //   BRIGHTEST        = 7
  //
  tm1637.set(BRIGHTEST);

  Serial.println();
  Serial.println();
  Serial.println("Display ready");
  //
  // Set Buzzer output
  //
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  analogWriteRange(freq);
  // beeb
  analogWrite(buzzer, 512); // 512
  delay(200);
  analogWrite(buzzer, 0);

  // Buttons
  pinMode(BUTTON_PIN_0, INPUT_PULLUP);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);

  // Neopixel-Ring
  setup_neopixel();
  strip.setBrightness(32);
  // neopixel_fun(7);
  SetLedRed(4);
  Serial.println("NeoPixel ready");

  // setup PIR
  setup_pir();
  Serial.println("PIR ready");
  //
  // Handle WiFi setup
  //
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180); // 3 Minuten!
  // wifiManager.setConnectTimeout(10);
  // wifiManager.autoConnect(PROJECT_NAME);

  if (!wifiManager.autoConnect(PROJECT_NAME))
  {
    Serial.println("failed to connect & NO AP-access ==> RESET ");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println("WIFI ready");
  setuptime();
  ArduinoOTA.setHostname(PROJECT_NAME);
  ArduinoOTA.onStart([]()
                     { DEBUG_LOG("OTA Start\n"); });
  ArduinoOTA.onEnd([]()
                   { DEBUG_LOG("OTA End\n"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
        char buf[32];
        memset(buf, '\0', sizeof(buf));
        snprintf(buf, sizeof(buf) - 1, "Upgrade - %02u%%\n", (progress / (total / 100)));
        DEBUG_LOG(buf); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
        DEBUG_LOG("Error - ");

        if (error == OTA_AUTH_ERROR)
            DEBUG_LOG("Auth Failed\n");
        else if (error == OTA_BEGIN_ERROR)
            DEBUG_LOG("Begin Failed\n");
        else if (error == OTA_CONNECT_ERROR)
            DEBUG_LOG("Connect Failed\n");
        else if (error == OTA_RECEIVE_ERROR)
            DEBUG_LOG("Receive Failed\n");
        else if (error == OTA_END_ERROR)
            DEBUG_LOG("End Failed\n"); });

  //
  // Ensure the OTA process is running & listening.
  ArduinoOTA.begin();
  // digitalWrite(BUILTIN_LED, HIGH);  // turn on LED with voltage HIGH
  ClearLed(4);
  //
  // Connect MQTT server
  //
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
  Serial.println("MQTT ready");
  Serial.println("Start MAIN...");
}

//
// This function is called continously, and is responsible
// for flashing the ":", and otherwise updating the display.
//
// We rely on the background NTP-updates to actually make sure
// that that works.
//
void loop()
{
  int button0Ivent = 0;
  int button1Ivent = 0;
  int button2Ivent = 0;

  float sensorValueLDRmean = 0.0;
  static float sensorValueLDRmean_old = 0.0;

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  static char buf[10] = {'\0'};
  static char prev[10] = {'\0'};

  static long last_read = 0;
  static long last_Change = 0;
  static long last_counter = 0;
  static long last_transmit = 0;
  static long lastWakeup = 0;

  static bool flash = true;
  static boolean setAlarm = false;
  static boolean lastsetAlarm = false;
  static boolean oldButtonState0 = true;
  static boolean oldButtonState1 = true;
  static boolean oldButtonState2 = true;
  static int button0Counter = 0;
  static int button1Counter = 0;
  static int button2Counter = 0;
  static boolean toggle = false;

  // static int newButtonState = 0;

  long counter = millis();
  int cur_hour = 0;
  int cur_min = 0;

  int cur_hour_value = 0;
  int cur_min_value = 0;
  boolean alarmMinuteSpeed = false;
  boolean alarmHourSpeed = false;
  static boolean AlarmActive = false;

  char helper[5] = "";
  char helper1[5];
  char helper2[5];

  long now = millis();

  //
  // Handle any pending over the air updates.
  ArduinoOTA.handle();

  // PIR func
  loop_pir();

  if (MQTTEvent)
  {
    if (MQTTEventValue == 3)
      colorWipe(strip.Color(255, 0, 0), 1);
    else if (MQTTEventValue == 2)
      colorWipe(strip.Color(0, 0, 255), 1);
    else if (MQTTEventValue == 1)
      colorWipe(strip.Color(0, 255, 0), 1);
  }
  else if (MQTTsleepMode)
  {
    colorWipe(strip.Color(0, 0, 0), 1);
  }
  else if (AlarmActive && toggle)
  {
    ClearLed(1);
    SetLedAlarmActive(2);
    SetLedAlarmActive(3);
    SetLedAlarmActive(4);
    SetLedAlarmActive(5);
    SetLedAlarmActive(6);
    ClearLed(7);
  }
  else if (setAlarm)
  {
    SetLedSetAlarm(1);
    ClearLed(2);
    ClearLed(3);
    ClearLed(4);
    ClearLed(5);
    ClearLed(6);
    SetLedSetAlarm(7);
  }
  else if (AlarmOn)
  {
    SetLedAlarmOn(0);
    ClearLed(1);
    ClearLed(2);
    ClearLed(3);
    ClearLed(4);
    ClearLed(5);
    ClearLed(6);
    ClearLed(7);
  }
  else
  {
    colorWipe(strip.Color(0, 0, 0), 1);
  }

  if ((last_counter == 0) || (abs(counter - last_counter) > 50))
  {

    boolean newButtonState0 = digitalRead(BUTTON_PIN_0);
    // Check if state changed from high to low (button press).
    if ((newButtonState0 == HIGH) && (oldButtonState0 == HIGH))
    {
      button0Counter = 0;
    }
    else if ((newButtonState0 == LOW) && (oldButtonState0 == HIGH))
    {
      button0Ivent = 1;
      button0Counter = 0;
    }
    else if ((newButtonState0 == LOW) && (oldButtonState0 == LOW))
    {
      button0Counter++;
    }
    else if ((newButtonState0 == HIGH) && (oldButtonState0 == LOW))
    {
      button0Counter = 0;
    }

    boolean newButtonState1 = digitalRead(BUTTON_PIN_1);
    // Check if state changed from high to low (button press).
    if ((newButtonState1 == HIGH) && (oldButtonState1 == HIGH))
    {
      button1Counter = 0;
    }
    else if ((newButtonState1 == LOW) && (oldButtonState1 == HIGH))
    {
      button1Ivent = 1;
      button1Counter = 0;
    }
    else if ((newButtonState1 == LOW) && (oldButtonState1 == LOW))
    {
      button1Counter++;
      if (button1Counter >= 10)
        button1Counter = 10;
    }
    else if ((newButtonState1 == HIGH) && (oldButtonState1 == LOW))
    {
      button1Counter = 0;
      alarmMinuteSpeed = false;
      alarmHourSpeed = false;
    }

    boolean newButtonState2 = digitalRead(BUTTON_PIN_2);
    // Check if state changed from high to low (button press).
    if ((newButtonState2 == HIGH) && (oldButtonState2 == HIGH))
    {
      button2Counter = 0;
    }
    else if ((newButtonState2 == LOW) && (oldButtonState2 == HIGH))
    {
      button2Counter = 0;
      button2Ivent = 1;
    }
    else if ((newButtonState2 == LOW) && (oldButtonState2 == LOW))
    {
      button2Counter++;
      if (button2Counter >= 10)
        button2Counter = 10;
    }
    else if ((newButtonState2 == HIGH) && (oldButtonState2 == LOW))
    {
      button2Counter = 0;
      alarmMinuteSpeed = false;
      alarmHourSpeed = false;
    }

    if (button0Counter >= 10)
    {
      button0Counter = 10;
      if (setAlarm)
      {
        setAlarm = false;
      }
      else
      {
        setAlarm = true;
      }
      button0Counter = 0;
      button1Counter = 0;
      button2Counter = 0;
      button1Ivent = 0;
      button2Ivent = 0;

      analogWrite(buzzer, 512); // 512
      delay(100);
      analogWrite(buzzer, 0);
    }

    oldButtonState0 = newButtonState0;
    oldButtonState1 = newButtonState1;
    oldButtonState2 = newButtonState2;

    last_counter = counter;
  }

  if (setAlarm)
  {
    // SET the alarm hour/min
    if (button1Counter >= 10)
    {
      alarmMinuteSpeed = true;
    }
    else if (button1Ivent)
    {
      alarmMinute++;
      if (alarmMinute > 59)
        alarmMinute = 0;
      button1Ivent = 0;
    }

    if (button2Counter >= 10)
    {
      alarmHourSpeed = true;
    }
    if (button2Ivent)
    {
      alarmHour++;
      if (alarmHour > 23)
        alarmHour = 0;
      button2Ivent = 0;
    }

    if ((last_Change == 0) || (abs(now - last_Change) > 150))
    {
      if (alarmMinuteSpeed)
        alarmMinute++;
      if (alarmMinute > 59)
        alarmMinute = 0;
      if (alarmHourSpeed)
        alarmHour++;
      if (alarmHour > 23)
        alarmHour = 0;
      last_Change = now;
    }

    cur_hour = alarmHour;
    cur_min = alarmMinute;
    //
    // Format them in a useful way.
    //
    sprintf(buf, "%02d%02d", cur_hour, cur_min);
  }
  else
  {
    //
    // Get the current hour/min
    String strTime = localTime();
    cur_hour_value = tm.tm_hour; // hours since midnight  0-23
    cur_min_value = tm.tm_min;   // minutes after the hour  0-59

    // Format them in a useful way.
    sprintf(buf, "%02d%02d", cur_hour_value, cur_min_value);
  }

  if (!MQTTsleepMode)
  {
    //
    // If the current "hourmin" is different to
    // that we displayed last loop ..
    //
    if (strcmp(buf, prev) != 0)
    {
      // Update the display
      tm1637.display(0, buf[0] - '0');
      tm1637.display(1, buf[1] - '0');
      tm1637.display(2, buf[2] - '0');
      tm1637.display(3, buf[3] - '0');

      // And cache it
      strcpy(prev, buf);
    }
  }
  else
  {
    tm1637.clearDisplay();
    tm1637.point(false);
  }

  if ((!setAlarm && lastsetAlarm))
  {
    if ((cfg.eehours != alarmHour) || (cfg.eeminutes != alarmMinute))
    {
      cfg.eehours = alarmHour;
      cfg.eeminutes = alarmMinute;
      saveConfig();
      Serial.println("WriteToEE");
    }
    itoa(alarmHour, helper1, 10);
    itoa(alarmMinute, helper2, 10);

    if (alarmHour > 9)
      strcpy(helper, helper1);
    else if (alarmHour == 0)
      strcpy(helper, "00");
    else
    {
      strcpy(helper, "0");
      strcat(helper, helper1);
    }

    if (alarmMinute > 9)
    {
      strcat(helper, ":");
      strcat(helper, helper2);
    }
    else if (alarmMinute == 0)
      strcat(helper, ":00");
    else
    {
      strcat(helper, ":0");
      strcat(helper, helper2);
    }

    client.publish("/hhome/DG/alarmtime", String(helper).c_str());
  }
  lastsetAlarm = setAlarm;

  // turn alarm on / off 
  if ((button1Counter >= 10) && (button2Counter >= 10) && !setAlarm)
  {
    if (AlarmOn)
      AlarmOn = false;
    else
      AlarmOn = true;
    button1Counter = 0;
    button2Counter = 0;
  }

  if (MQTTclockAlarmOn)
  {
    if ((MQTTclockAlarmHours != alarmHour) || (MQTTclockAlarmMinutes != alarmMinute))
    {
      alarmHour = MQTTclockAlarmHours;
      alarmMinute = MQTTclockAlarmMinutes;

      cfg.eehours = alarmHour;
      cfg.eeminutes = alarmMinute;
      saveConfig();
      Serial.println("WriteToEE_2");
    }

    itoa(alarmHour, helper1, 10);
    itoa(alarmMinute, helper2, 10);

    if (alarmHour > 9)
      strcpy(helper, helper1);
    else if (alarmHour == 0)
      strcpy(helper, "00");
    else
    {
      strcpy(helper, "0");
      strcat(helper, helper1);
    }

    if (alarmMinute > 9)
    {
      strcat(helper, ":");
      strcat(helper, helper2);
    }
    else if (alarmMinute == 0)
      strcat(helper, ":00");
    else
    {
      strcat(helper, ":0");
      strcat(helper, helper2);
    }

    client.publish("/hhome/DG/alarmtime", String(helper).c_str());

    MQTTclockAlarmOn = false;
  }

  //
  // The preceeding piece of code would
  // have ensured the display only updated
  // when the hour/min changed.
  //
  // However note that we nuke the cached
  // value every half-second - solely so we can
  // blink the ":".
  //
  //  Sigh

  if ((last_read == 0) || (abs(now - last_read) > INTERVAL2))
  {
    // Invert the "show :" flag
    flash = !flash;

    // Apply it.
    if (!MQTTsleepMode)
    {
      if (!setAlarm)
        tm1637.point(flash);
      else
        tm1637.point(true);
    }
    else
    {
      tm1637.point(false);
    }

    //
    // Note that the ":" won't redraw unless/until you update.
    // So we'll force that to happen by removing the cached
    // value here.
    //
    memset(prev, '\0', sizeof(prev));

    // read the input on analog pin 0:
    int sensorValueLDR = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V):
    // float voltage = sensorValueLDR * (3.2 / 1023.0);
    // print out the value you read:

    sensorValueLDRmean = RATIO * sensorValueLDRmean + (1 - RATIO) * sensorValueLDR;

    int brightness_setpoint = sensorValueLDR / 128; // 1024/128= 8 (0...7 )

    brightness_setpoint++;

    if (brightness_setpoint > 7)
      brightness_setpoint = 7;

    if (!MQTTbrightnessControl)
      tm1637.set(brightness_setpoint);
    else
      tm1637.set(MQTTclockBrightness);

    uint8_t brightness_setpoint_neopixel = sensorValueLDR / 12; // 1024/10 =  100  max ist eigentlich 255!

    if (brightness_setpoint_neopixel == 0)
      brightness_setpoint_neopixel = 1;

    strip.setBrightness(brightness_setpoint_neopixel);

    // Check alarm time
    if (AlarmOn)
    {
      if ((cur_hour_value == alarmHour) && (cur_min_value == alarmMinute))
        AlarmActive = true;
      else
        AlarmActive = false;
    }
    else
      AlarmActive = false;

    // and turn on buzzer - if necessary
    if (AlarmActive && toggle)
    {
      analogWrite(buzzer, 512); // 512
      toggle = false;
    }
    else
    {
      analogWrite(buzzer, 0);
      toggle = true;
    }

    last_read = now;
  }

  if (abs(sensorValueLDRmean - sensorValueLDRmean_old) > 50)
  {
    client.publish("/hhome/DG/ldr", String(sensorValueLDRmean).c_str());
    sensorValueLDRmean_old = sensorValueLDRmean;
  }

  if ((last_transmit == 0) || (abs(now - last_transmit) > INTERVAL1))
  {
    sensorValueLDRmean_old = sensorValueLDRmean;
    client.publish("/hhome/DG/ldr", String(sensorValueLDRmean).c_str());

    itoa(alarmHour, helper1, 10);
    itoa(alarmMinute, helper2, 10);

    if (alarmHour > 9)
      strcpy(helper, helper1);
    else if (alarmHour == 0)
      strcpy(helper, "00");
    else
    {
      strcpy(helper, "0");
      strcat(helper, helper1);
    }

    if (alarmMinute > 9)
    {
      strcat(helper, ":");
      strcat(helper, helper2);
    }
    else if (alarmMinute == 0)
      strcat(helper, ":00");
    else
    {
      strcat(helper, ":0");
      strcat(helper, helper2);
    }

    client.publish("/hhome/DG/alarmtime", String(helper).c_str());
    last_transmit = now;
  }

  if ((lastWakeup == 0) || (abs(now - lastWakeup) > INTERVAL3))
  {
    MQTTsleepMode = true;
    MQTTEvent = false;
    lastWakeup = now;
  }
}
