#include <Arduino.h>
#include <WiFiMulti.h>
#include <time.h>

#include <M5Unified.h>

#include <Avatar.h> // https://github.com/meganetaaan/m5stack-avatar
#include <ServoEasing.hpp> // https://github.com/ArminJo/ServoEasing
#include "AtaruFace.h"
#include "RamFace.h"
#include "PandaFace.h"
#include "TVFace.h"

#if defined(ARDUINO_M5STACK_Core2)
#define USE_VOICE_TEXT //for M5STACK_Core2 Only
#endif

#ifdef USE_VOICE_TEXT
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SLipSync.h"
#include "AudioFileSourceVoiceTextStream.h"

#define TSS_VOLUME "200"
#define JST 3600 * 9

WiFiMulti wifiMulti;

AudioGeneratorMP3 *mp3;
AudioFileSourceVoiceTextStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2SLipSync *out;
const int preallocateBufferSize = 40*1024;
uint8_t *preallocateBuffer;
#endif

#define AVATAR_ATARU 0
#define AVATAR_RAM 1
#define AVATAR_STACK 2
#define AVATAR_SUSU 3
#define AVATAR_BRAUN 4
#define NUMBER_OF_AVATARS 5

using namespace m5avatar;
Avatar avatar;
Face *faces[NUMBER_OF_AVATARS];
ColorPalette *cps[NUMBER_OF_AVATARS];
int avatar_indexes[3] = {0, 1, 2};
int current_avatar_index = 0;

#define SETTINGS_FILENAME "/settings.txt"
#define NUMBER_OF_SETTINGS 10
#define MAX_LENGTH_SETTINGS 128
#define MAX_LENGTH_GREETING 128
#define MAX_LENGTH_TIME_ANNOUNCE_SENTENCE 128
#define SETTINGS_INDEX_TTS_API_KEY 0                 // char*
#define SETTINGS_INDEX_DEFAULT_AVATAR 1              // int
#define SETTINGS_INDEX_TIME_ANNOUNCE_SENTENCE 2      // char*
#define SETTINGS_INDEX_TIME_ANNOUNCE_INTERVAL 3      // int
#define SETTINGS_INDEX_TIME_ANNOUNCE_START 4         // int
#define SETTINGS_INDEX_TIME_ANNOUNCE_END 5           // int
#define SETTINGS_INDEX_SERVO_PIN_X 6                 // int
#define SETTINGS_INDEX_SERVO_PIN_Y 7                 // int
#define SETTINGS_INDEX_START_DEGREE_VALUE_X_OFFSET 8 // int
#define SETTINGS_INDEX_START_DEGREE_VALUE_Y_OFFSET 9 // int

// 基本、ソースコードの編集は不要です　※SDカード上の設定ファイルを編集してください
const char *DEFAULT_SETTINGS[NUMBER_OF_SETTINGS] = {
    "YOUR_TSS_API_KEY",           // TTS_API_KEY
    "0",                          // DEFAULT_AVATAR　※あたる(0)/ラム(1)/スタック(2)/スースー(3)/ブラウン(4)
    "現在の時刻は%d時%sです。",      // TIME_ANNOUNCE_SENTENCE　※時刻通知メッセージ
    "10",                         // TIME_ANNOUNCE_INTERVAL　※N分間隔で時刻通知
    "7",                          // TIME_ANNOUNCE_START　※朝のN時0分から時刻通知
    "21",                         // TIME_ANNOUNCE_END　※夜のN-1時59分まで時刻通知
#if defined(ARDUINO_M5STACK_Core2)
    "33", // SERVO_PIN_X　※Core2の場合はPort.Aなら33 / Port.Cなら13、Core2以外の場合は21を記入
    "32", // SERVO_PIN_Y　※Core2の場合はPort.Aなら32 / Port.Cなら14、Core2以外の場合は22を記入
#elif defined(ARDUINO_M5STACK_FIRE)
    "21",
    "22",
#elif defined(ARDUINO_M5Stack_Core_ESP32)
    "21",
    "22",
#endif
    "0", // START_DEGREE_VALUE_X_OFFSET　※Stackchan-tester-core2での調整値を記入
    "0", // START_DEGREE_VALUE_Y_OFFSET　※Stackchan-tester-core2での調整値を記入
};
char settings[NUMBER_OF_SETTINGS][MAX_LENGTH_SETTINGS];

#ifdef USE_VOICE_TEXT

#define MESSAGES_FILENAME "/messages.txt"
#define NUMBER_OF_MESSAGES 64
#define MAX_LENGTH_MESSAGE 256
#define MAX_LENGTH_MESSAGE_INFO 32

const char *DEFAULT_MESSAGE = "よろしくね！";
const char *DEFAULT_EMOTION = "Happy";
const char *DEFAULT_WHO = "ANY";
const char *I_AM[NUMBER_OF_AVATARS] = {
    "あたる",
    "ラム",
    "スタック",
    "スースー",
    "ブラウン",
};
const char *ANY = "ANY";

int message_count;
int message_index;
char message[NUMBER_OF_MESSAGES][MAX_LENGTH_MESSAGE];
char emotion[NUMBER_OF_MESSAGES][MAX_LENGTH_MESSAGE_INFO];
char who[NUMBER_OF_MESSAGES][MAX_LENGTH_MESSAGE_INFO];

#define TIME_ANNOUNCE_MESSAGES_FILENAME "/time_announce_messages.txt"
#define NUMBER_OF_TIME_ANNOUNCE_MESSAGES 128
#define MAX_LENGTH_TIME_ANNOUNCE_MESSAGE 256
#define MAX_LENGTH_TIME_ANNOUNCE_MESSAGE_INFO 32
const char *WD[7] = { "日", "月", "火", "水", "木", "金", "土" };
const char *DEFAULT_WD = "日月火水木金土";

int pre_min;
int time_announce_message_count;
char time_announce_message[NUMBER_OF_TIME_ANNOUNCE_MESSAGES][MAX_LENGTH_TIME_ANNOUNCE_MESSAGE];
char time_announce_hours[NUMBER_OF_TIME_ANNOUNCE_MESSAGES][MAX_LENGTH_TIME_ANNOUNCE_MESSAGE_INFO];
char time_announce_minutes[NUMBER_OF_TIME_ANNOUNCE_MESSAGES][MAX_LENGTH_TIME_ANNOUNCE_MESSAGE_INFO];
char time_announce_day_of_week[NUMBER_OF_TIME_ANNOUNCE_MESSAGES][MAX_LENGTH_TIME_ANNOUNCE_MESSAGE_INFO];

struct GREETING
{
  int h_start;
  int h_end;
  char greeting[MAX_LENGTH_GREETING];
};
#define NUMBER_OF_GREETINGS 11
GREETING greetings[NUMBER_OF_GREETINGS] = {
    {5, 10, "おはよう！"},
    {10, 17, "こんにちは！"},
    {17, 24, "こんばんは！"},
    {0, 2, "こんばんは！"},
    {5, 12, "グッドモーニング！"},
    {12, 17, "グッドアフタヌーン！"},
    {17, 21, "グッドイブニング！"},
    {21, 24, "グンナイ！"},
    {0, 5, "グンナイ！"},
    {0, 24, "おはこんばんちわ！"},
    {0, 24, "ハロー！"},
};
int greetings_index = 0;
#define NUMBER_OF_EXCLAMATIONS 9
const char *exclamations[NUMBER_OF_EXCLAMATIONS] = {
    "えーーっと！",
    "およよ！",
    "おっとっと！",
    "うほほ～い！",
    "うふふふふ！",
    "ほげほげ！",
    "ふがふが！",
    "ぴよぴよ！",
    "ふむふむ！",
};
int exclamations_index = 0;

#define WIFI_FILENAME "/wifi_info.txt"
#define NUMBER_OF_WIFI_INFO 4
#define MAX_LENGTH_WIFI_INFO 128

bool flag_online;
bool flag_mp3_begin;

#endif

#define START_DEGREE_VALUE_X 90
#define START_DEGREE_VALUE_Y 90

ServoEasing servo_x;
ServoEasing servo_y;

void behavior(void *args)
{
  float gazeX, gazeY;
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
  for (;;)
  {
#ifdef USE_VOICE_TEXT
    int level = out->getLevel();
    level = abs(level);
    if(level > 10000)
    {
      level = 10000;
    }
    float open = (float)level/10000.0;
    avatar->setMouthOpenRatio(open);
#endif

    vTaskDelay(1/portTICK_PERIOD_MS);
//    delay(50);
  }
}

void servoloop(void *args)
{
  float gazeX, gazeY;
  DriveContext *ctx = (DriveContext *)args;
  for (;;)
  {
    Avatar *avatar = ctx->getAvatar();
    avatar->getGaze(&gazeY, &gazeX);
    servo_x.setEaseTo(START_DEGREE_VALUE_X + String(settings[SETTINGS_INDEX_START_DEGREE_VALUE_X_OFFSET]).toInt() + (int)(20.0 * gazeX));
    if(gazeY < 0) {
      servo_y.setEaseTo(START_DEGREE_VALUE_Y + String(settings[SETTINGS_INDEX_START_DEGREE_VALUE_Y_OFFSET]).toInt() + (int)(20.0 * gazeY));
    } else {
      servo_y.setEaseTo(START_DEGREE_VALUE_Y + String(settings[SETTINGS_INDEX_START_DEGREE_VALUE_Y_OFFSET]).toInt() + (int)(10.0 * gazeY));
    }
    synchronizeAllServosStartAndWaitForAllServosToStop();
    vTaskDelay(33/portTICK_PERIOD_MS);
  }
}

void setup() {

#ifdef USE_VOICE_TEXT
  preallocateBuffer = (uint8_t*)ps_malloc(preallocateBufferSize);
#endif
  auto cfg = M5.config();
  M5.begin(cfg);

  auto spk_config = M5.Speaker.config();
  spk_config.sample_rate = 88200;
  spk_config.stereo = false;
  M5.Speaker.config(spk_config);
  //M5.Speaker.begin();

  M5.Lcd.setBrightness(100);
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  delay(1000);

  Serial.println("# SET DEFAULT SETTINGS");
  for (int index = 0; index < NUMBER_OF_SETTINGS; index++)
  {
    strcpy((char *)settings[index], (char *)DEFAULT_SETTINGS[index]);
  }

  // SDカードマウント待ち
  Serial.print("Mounting SD");
  M5.Lcd.print("Mounting SD");
  int wait_count = 0;
  bool sd_mount = true;
  while (false == SD.begin(GPIO_NUM_4, SPI, 25000000))
  {
    Serial.printf(".");
    M5.Lcd.printf(".");
    delay(100);
    wait_count += 1;
    if (wait_count >= 20)
    {
      sd_mount = false;
      break;
    }
  }
  if (sd_mount)
  {
    Serial.println("\nMounted");
    M5.Lcd.println("\nMounted");
  }
  else
  {
    Serial.println("\nNOT Mount");
    M5.Lcd.println("\nNOT Mount");
  }

  int settings_count = 0;
  auto fs = SD.open(SETTINGS_FILENAME);
  if (fs)
  {
    Serial.println("# SETTINGS FILE OPEN");

    int line_count = 1;
    int pos = 0;
    while (fs.available())
    {
      char c = fs.read();
      if (pos == 0 && c == '#')
      {
        line_count = -1;
        pos += 1;
      }
      else if (c == '\r')
      {
        // 何もしない
      }
      else if (c == '\n')
      {
        if (line_count == 0)
        {
          settings_count += 1;
          if (settings_count >= NUMBER_OF_SETTINGS) break;
        }
        line_count += 1;
        pos = 0;
      }
      else if (line_count == 0)
      {
        settings[settings_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_SETTINGS)
        {
          pos -= 1;
        }
        settings[settings_count][pos] = 0;
      }
    }
    if (line_count == 0 && pos > 0 && settings_count < NUMBER_OF_SETTINGS)
    {
      settings_count += 1;
    }
    fs.close();
  }
  else
  {
    Serial.println("# SETTINGS FILE OPEN ERROR");
  }

  Serial.printf("# settings_count = %d\n", settings_count);
  for (int index = 0; index < NUMBER_OF_SETTINGS; index++)
  {
    Serial.printf("%d: %s\n", index, (char *)settings[index]);
  }
  Serial.println("");

#ifdef USE_VOICE_TEXT
  auto f = SD.open(MESSAGES_FILENAME);
  if (f)
  {
    Serial.println("# MESSAGES FILE OPEN");

    message_count = 0;
    int line_count = 3;
    int pos = 0;
    while (f.available())
    {
      char c = f.read();
      if (pos == 0 && c == '#')
      {
        line_count = -1;
        pos += 1;
      }
      else if (c == '\r')
      {
        // 何もしない
      }
      else if (c == '\n')
      {
        if (line_count == 2)
        {
          message_count += 1;
          if (message_count >= NUMBER_OF_MESSAGES) break;
        }
        line_count += 1;
        pos = 0;
      }
      else if (line_count == 0)
      {
        message[message_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_MESSAGE)
        {
          pos -= 1;
        }
        message[message_count][pos] = 0;
      }
      else if (line_count == 1)
      {
        emotion[message_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_MESSAGE_INFO)
        {
          pos -= 1;
        }
        emotion[message_count][pos] = 0;
      }
      else if (line_count == 2)
      {
        who[message_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_MESSAGE_INFO)
        {
          pos -= 1;
        }
        who[message_count][pos] = 0;
      }
    }
    if (line_count == 2 && pos > 0 && message_count < NUMBER_OF_MESSAGES)
    {
      message_count += 1;
    }
    f.close();
  }
  else
  {
    Serial.println("# MESSAGES FILE OPEN ERROR");
  }

  Serial.printf("# message_count = %d\n", message_count);
  if (message_count == 0)
  {
    Serial.println("# SET DEFAULT MESSAGE");
    strcpy((char *)message[0], DEFAULT_MESSAGE);
    strcpy((char *)emotion[0], DEFAULT_EMOTION);
    strcpy((char *)who[0], DEFAULT_WHO);
    message_count = 1;
  }
  for (int index = 0; index < message_count; index++)
  {
    Serial.printf("%s\n", (char *)message[index]);
    Serial.printf("%s\n", (char *)emotion[index]);
    Serial.printf("%s\n", (char *)who[index]);
  }
  Serial.println("");

  auto ft = SD.open(TIME_ANNOUNCE_MESSAGES_FILENAME);
  if (ft)
  {
    Serial.println("# TIME ANNOUNCE MESSAGES FILE OPEN");

    time_announce_message_count = 0;
    int line_count = 3;
    int pos = 0;
    while (ft.available())
    {
      char c = ft.read();
      if (pos == 0 && c == '#')
      {
        line_count = -1;
        pos += 1;
      }
      else if (c == '\r')
      {
        // 何もしない
      }
      else if (c == '\n')
      {
        if (line_count == 2)
        {
          time_announce_message_count += 1;
          if (time_announce_message_count >= NUMBER_OF_TIME_ANNOUNCE_MESSAGES) break;
        }
        line_count += 1;
        pos = 0;
      }
      else if (line_count == 0)
      {
        time_announce_message[time_announce_message_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_TIME_ANNOUNCE_MESSAGE)
        {
          pos -= 1;
        }
        time_announce_message[time_announce_message_count][pos] = 0;
      }
      else if (line_count == 1)
      {
        time_announce_hours[time_announce_message_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_TIME_ANNOUNCE_MESSAGE_INFO)
        {
          pos -= 1;
        }
        time_announce_hours[time_announce_message_count][pos] = 0;
      }
      else if (line_count == 2)
      {
        time_announce_minutes[time_announce_message_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_TIME_ANNOUNCE_MESSAGE_INFO)
        {
          pos -= 1;
        }
        time_announce_minutes[time_announce_message_count][pos] = 0;
      }
    }
    if (line_count == 2 && pos > 0 && time_announce_message_count < NUMBER_OF_TIME_ANNOUNCE_MESSAGES)
    {
      time_announce_message_count += 1;
    }
    ft.close();
  }
  else
  {
    Serial.println("# TIME ANNOUNCE MESSAGES FILE OPEN ERROR");
  }

  Serial.printf("# time_announce_message_count = %d\n", time_announce_message_count);
  for (int index = 0; index < time_announce_message_count; index++)
  {
    int pos = String(time_announce_hours[index]).indexOf(":");
    if (pos != -1)
    {
      strcpy((char *)time_announce_day_of_week[index], (char *)time_announce_minutes[index]);
      char tmp[3];
      String(time_announce_hours[index]).substring(pos + 1).toCharArray(tmp, 3);
      strcpy((char *)time_announce_minutes[index], (char *)tmp);
      time_announce_hours[index][pos] = 0;
    }
    else
    {
      strcpy((char *)time_announce_day_of_week[index], (char *)DEFAULT_WD);
    }
    Serial.printf("%s\n", (char *)time_announce_message[index]);
    Serial.printf("%s\n", (char *)time_announce_hours[index]);
    Serial.printf("%s\n", (char *)time_announce_minutes[index]);
    Serial.printf("%s\n", (char *)time_announce_day_of_week[index]);
  }
  Serial.println("");

  int wifi_info_count = 0;
  char ssid[NUMBER_OF_WIFI_INFO][MAX_LENGTH_WIFI_INFO];
  char password[NUMBER_OF_WIFI_INFO][MAX_LENGTH_WIFI_INFO];
  auto fw = SD.open(WIFI_FILENAME);
  if (fw)
  {
    Serial.println("# WIFI INFO FILE OPEN");

    int line_count = 2;
    int pos = 0;
    while (fw.available())
    {
      char c = fw.read();
      if (pos == 0 && c == '#')
      {
        line_count = -1;
        pos += 1;
      }
      else if (c == '\r')
      {
        // 何もしない
      }
      else if (c == '\n')
      {
        if (line_count == 1)
        {
          wifi_info_count += 1;
          if (wifi_info_count >= NUMBER_OF_WIFI_INFO) break;
        }
        line_count += 1;
        pos = 0;
      }
      else if (line_count == 0)
      {
        ssid[wifi_info_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_WIFI_INFO)
        {
          pos -= 1;
        }
        ssid[wifi_info_count][pos] = 0;
      }
      else if (line_count == 1)
      {
        password[wifi_info_count][pos] = c;
        pos += 1;
        if (pos >= MAX_LENGTH_WIFI_INFO)
        {
          pos -= 1;
        }
        password[wifi_info_count][pos] = 0;
      }
    }
    if (line_count == 1 && pos > 0 && wifi_info_count < NUMBER_OF_WIFI_INFO)
    {
      wifi_info_count += 1;
    }
    fw.close();
  }
  else
  {
    Serial.println("# WIFI INFO FILE OPEN ERROR");
  }

  Serial.printf("# wifi_info_count = %d\n", wifi_info_count);
  for (int index = 0; index < wifi_info_count; index++)
  {
    Serial.printf("%s\n", (char *)ssid[index]);
    Serial.printf("%s\n", (char *)password[index]);
  }
  Serial.println("");

  Serial.println("Connecting to WiFi");
  M5.Lcd.print("Connecting to WiFi");
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  if (wifi_info_count > 0)
  {
    for (int index = 0; index < wifi_info_count; index++)
    {
      wifiMulti.addAP(ssid[index], password[index]);
    }
    while (wifiMulti.run() != WL_CONNECTED) {
      delay(250);
      Serial.print(".");
      M5.Lcd.print(".");
    }
  }

  flag_online = false;
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nConnected");
    M5.Lcd.println("\nConnected");
    M5.Speaker.tone(2000, 500);
    delay(500);
    M5.Speaker.tone(1000, 500);
    flag_online = true;
  }
  else
  {
    Serial.println("\nNOT Connect");
    M5.Lcd.println("\nNOT Connect");
    M5.Speaker.tone(1000, 1500);
  }
  delay(1000);

  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  audioLogger = &Serial;
  out = new AudioOutputI2SLipSync(0, 0);
  out->SetPinout(12, 0, 2);           // ピン配列を指定（BCK, LRCK, DATA)BashCopy
  out->SetOutputModeMono(false);
  mp3 = new AudioGeneratorMP3();

  flag_mp3_begin = false;
  message_index = 0;

  time_t nowSecs = time(nullptr);
  struct tm *tm = localtime(&nowSecs);
  pre_min = tm->tm_min;

#endif

  if (servo_x.attach(String(settings[SETTINGS_INDEX_SERVO_PIN_X]).toInt(), START_DEGREE_VALUE_X + String(settings[SETTINGS_INDEX_START_DEGREE_VALUE_X_OFFSET]).toInt(), DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE))
  {
    Serial.println("Error attaching servo x");
  }
  if (servo_y.attach(String(settings[SETTINGS_INDEX_SERVO_PIN_Y]).toInt(), START_DEGREE_VALUE_Y + String(settings[SETTINGS_INDEX_START_DEGREE_VALUE_Y_OFFSET]).toInt(), DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE))
  {
    Serial.println("Error attaching servo y");
  }
  servo_x.setEasingType(EASE_QUADRATIC_IN_OUT);
  servo_y.setEasingType(EASE_QUADRATIC_IN_OUT);
  setSpeedForAllServos(60);

  faces[AVATAR_ATARU] = new AtaruFace();
  faces[AVATAR_RAM] = new RamFace();
  faces[AVATAR_STACK] = avatar.getFace();
  faces[AVATAR_SUSU] = new PandaFace();
  faces[AVATAR_BRAUN] = new TVFace();

  for (int i = 0; i < NUMBER_OF_AVATARS; i++)
  {
    cps[i] = new ColorPalette();
  }
  cps[AVATAR_ATARU]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[AVATAR_ATARU]->set(COLOR_BACKGROUND, TFT_WHITE);
  cps[AVATAR_ATARU]->set(COLOR_SECONDARY, TFT_WHITE);

  cps[AVATAR_RAM]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[AVATAR_RAM]->set(COLOR_BACKGROUND, TFT_WHITE);
  cps[AVATAR_RAM]->set(COLOR_SECONDARY, TFT_WHITE);

  cps[AVATAR_SUSU]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[AVATAR_SUSU]->set(COLOR_BACKGROUND, TFT_WHITE);
  cps[AVATAR_SUSU]->set(COLOR_SECONDARY, TFT_WHITE);

  cps[AVATAR_BRAUN]->set(COLOR_PRIMARY, TFT_WHITE);
  cps[AVATAR_BRAUN]->set(COLOR_BACKGROUND, TFT_BLUE);
  cps[AVATAR_BRAUN]->set(COLOR_SECONDARY, TFT_YELLOW);

  avatar.init(8);
  current_avatar_index = String(settings[SETTINGS_INDEX_DEFAULT_AVATAR]).toInt();
  avatar.setFace(faces[current_avatar_index]);
  avatar.setColorPalette(*cps[current_avatar_index]);
  avatar.addTask(behavior, "behavior");
  avatar.addTask(servoloop, "servoloop");

  randomSeed(millis());
}

#ifdef USE_VOICE_TEXT

const char *tts_params[NUMBER_OF_AVATARS] = {
    "&emotion_level=2&emotion=happiness&format=mp3&speaker=takeru&volume=" TSS_VOLUME "&speed=100&pitch=130",
    "&emotion_level=2&emotion=happiness&format=mp3&speaker=hikari&volume=" TSS_VOLUME "&speed=120&pitch=130",
    "&emotion_level=4&emotion=anger&format=mp3&speaker=bear&volume=" TSS_VOLUME "&speed=120&pitch=100",
    "&emotion_level=2&emotion=sadness&format=mp3&speaker=hikari&volume=" TSS_VOLUME "&speed=90&pitch=60",
    "&format=mp3&speaker=show&volume=" TSS_VOLUME "&speed=150&pitch=200"};

void set_expression(char *emotion)
{
  if (String(emotion) == String("Neutral") || String(emotion) == String("Normal"))
  {
    avatar.setExpression(Expression::Neutral);
  }
  else if (String(emotion) == String("Angry"))
  {
    avatar.setExpression(Expression::Angry);
  }
  else if (String(emotion) == String("Happy"))
  {
    avatar.setExpression(Expression::Happy);
  }
  else if (String(emotion) == String("Sad"))
  {
    avatar.setExpression(Expression::Sad);
  }
  else if (String(emotion) == String("Doubt") || String(emotion) == String("Worried"))
  {
    avatar.setExpression(Expression::Doubt);
  }
  else if (String(emotion) == String("Sleepy"))
  {
    avatar.setExpression(Expression::Sleepy);
  }
  else
  {
    avatar.setExpression(Expression::Neutral);
  }
}

char *get_exclamation()
{
  char *exclamation = (char *)exclamations[exclamations_index];
  exclamations_index = (exclamations_index + random(NUMBER_OF_EXCLAMATIONS)) % NUMBER_OF_EXCLAMATIONS;
  return exclamation;
}

char *get_greeting()
{
  char *greeting;

  time_t nowSecs = time(nullptr);
  struct tm *tm = localtime(&nowSecs);
  int count = 0;
  while (true)
  {
    greeting = (char *)greetings[greetings_index].greeting;
    int h_start = greetings[greetings_index].h_start;
    int h_end = greetings[greetings_index].h_end;
    greetings_index = (greetings_index + 1) % NUMBER_OF_GREETINGS;
    if (tm->tm_hour >= h_start && tm->tm_hour < h_end)
    {
      break;
    }

    count++;
    if (count >= NUMBER_OF_GREETINGS)
    {
      greeting = (char *)greetings[NUMBER_OF_GREETINGS - 1].greeting;
      break;
    }
  }
  return greeting;
}

void VoiceText_tts(char *text, char *emotion)
{
  char msg[MAX_LENGTH_GREETING * 3 + MAX_LENGTH_TIME_ANNOUNCE_SENTENCE + MAX_LENGTH_TIME_ANNOUNCE_MESSAGE];
  sprintf(msg, "%s%s%s%s", get_exclamation(), get_exclamation(), get_greeting(), text);
  Serial.println(msg);
  Serial.println(emotion);

  file = new AudioFileSourceVoiceTextStream(msg, (char *)tts_params[current_avatar_index], (char *)settings[SETTINGS_INDEX_TTS_API_KEY]);
  buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
  set_expression(emotion);

  Serial.println("# mp3 begin");
  delay(100);
  mp3->begin(buff, out);
}

void create_time_announce_sentence(char *sentence, char *format, int hour, int min)
{
  if (String(format).indexOf("%s") != -1)
  {
    // 0.2.1以降の設定ファイル(%d時%s)向け
    char min_str[32];
    if (min == 0)
    {
      sprintf(min_str, "ちょうど");
    }
    else if (min == 30)
    {
      sprintf(min_str, "半");
    }
    else
    {
      sprintf(min_str, "%d分", min);
    }
    sprintf(sentence, format, hour, (char *)min_str);
  }
  else
  {
    // 0.2.0の設定ファイル(%d時%d分)向け
    sprintf(sentence, format, hour, min);
  }
}

void announce_time_if_needed()
{
  time_t nowSecs = time(nullptr);
  struct tm *tm = localtime(&nowSecs);
  if (pre_min != tm->tm_min)
  {
    pre_min = tm->tm_min;

    bool done = false;
    if (time_announce_message_count > 0)
    {
      for (int index = 0; index < time_announce_message_count; index++)
      {
        if (tm->tm_hour == String(time_announce_hours[index]).toInt() && tm->tm_min == String(time_announce_minutes[index]).toInt() && String(time_announce_day_of_week[index]).indexOf(WD[tm->tm_wday]) != -1)
        {
          char tmp[MAX_LENGTH_TIME_ANNOUNCE_SENTENCE];
          char msg[MAX_LENGTH_TIME_ANNOUNCE_MESSAGE];
          create_time_announce_sentence((char *)tmp, (char *)settings[SETTINGS_INDEX_TIME_ANNOUNCE_SENTENCE], tm->tm_hour, tm->tm_min);
          sprintf(msg, "%s%s", tmp, (char *)time_announce_message[index]);
          VoiceText_tts((char *)msg, (char *)"Neutral");
          done = true;
          break;
        }
      }
    }

    if (!done && tm->tm_hour >= String(settings[SETTINGS_INDEX_TIME_ANNOUNCE_START]).toInt() && tm->tm_hour < String(settings[SETTINGS_INDEX_TIME_ANNOUNCE_END]).toInt())
    {
      if (String(settings[SETTINGS_INDEX_TIME_ANNOUNCE_INTERVAL]).toInt() != 0 && tm->tm_min % String(settings[SETTINGS_INDEX_TIME_ANNOUNCE_INTERVAL]).toInt() == 0)
      {
        char msg[MAX_LENGTH_TIME_ANNOUNCE_SENTENCE];
        create_time_announce_sentence((char *)msg, (char *)settings[SETTINGS_INDEX_TIME_ANNOUNCE_SENTENCE], tm->tm_hour, tm->tm_min);
        VoiceText_tts((char *)msg, (char *)"Neutral");
      }
    }
  }
}

#endif

void loop() {
  M5.update();
#ifdef USE_VOICE_TEXT
  auto t = M5.Touch.getDetail();
  auto pos = M5.Touch.getTouchPointRaw();
  static int lastms = 0;
  if (mp3->isRunning())
  {
    if (!mp3->loop())
    {
      // メッセージ再生終了
      mp3->stop();
      out->setLevel(0);
      delete file;
      delete buff;
      Serial.println("# mp3 stop");
      avatar.setExpression(Expression::Neutral);
    }
    else if (millis() - lastms > 1000)
    {
      lastms = millis();
      Serial.printf("# Running for %d ms...\n", lastms);
      Serial.flush();
    }
  }
  else if (flag_mp3_begin)
  {
    flag_mp3_begin = false;

    // メッセージ決定
    int tmp_count = 0;
    while (tmp_count < message_count)
    {
      int index = message_index % message_count;
      if (String(who[index]) == String(ANY) || String(who[index]) == String(I_AM[current_avatar_index]))
      {
        break;
      }
      tmp_count += 1;
      message_index += 1;
    }
    int index = message_index % message_count;
    Serial.printf("message_index = %d\n", index);
    Serial.println((char *)who[index]);

    // メッセージ再生開始
    VoiceText_tts((char *)message[index], (char *)emotion[index]);
    message_index += 1;
  }
  else
  {
    bool flag_change = false;
    if (M5.BtnA.wasPressed())
    {
      flag_change = true;
      int index = avatar_indexes[0];
      avatar_indexes[0] = 3 - index; // 0 <-> 3
      current_avatar_index = index;
    }
    else if (M5.BtnB.wasPressed())
    {
      flag_change = true;
      int index = avatar_indexes[1];
      current_avatar_index = index;
    }
    else if (M5.BtnC.wasPressed())
    {
      flag_change = true;
      int index = avatar_indexes[2];
      avatar_indexes[2] = 6 - index; // 2 <-> 4
      current_avatar_index = index;
    }
    else if (t.isPressed() && pos.y < 240)
    {
      M5.Speaker.tone(2000, 200);
      if (flag_online)
      {
        avatar.setExpression(Expression::Doubt);
        flag_mp3_begin = true;
        delay(1000);
      }
      else{
        Serial.println("# NOT mp3 begin (NOT ONLINE)");
      }
    }

    if (flag_change)
    {
      M5.Speaker.tone(2000, 200);
      // アバター変更
      avatar.setFace(faces[current_avatar_index]);
      avatar.setColorPalette(*cps[current_avatar_index]);
      delay(200);
    }

    if (flag_online && !flag_mp3_begin)
    {
      announce_time_if_needed();
    }
  }
#else
  if (M5.BtnA.wasPressed())
  {
    int index = avatar_indexes[0];
    avatar_indexes[0] = 3 - index; // 0 <-> 3
    avatar.setFace(faces[index]);
    avatar.setColorPalette(*cps[index]);
  }
  if (M5.BtnB.wasPressed())
  {
    int index = avatar_indexes[1];
    avatar.setFace(faces[index]);
    avatar.setColorPalette(*cps[index]);
  }
  if (M5.BtnC.wasPressed())
  {
    int index = avatar_indexes[2];
    avatar_indexes[2] = 6 - index; // 2 <-> 4
    avatar.setFace(faces[index]);
    avatar.setColorPalette(*cps[index]);
  }
#endif
}
