#include <Keypad.h>              // 4x4 鍵盤控制函式庫
#include "RTClib.h"              // RTC (DS1307) 實時時鐘函式庫
#include "SPI.h"
#include "Adafruit_GFX.h"        // Adafruit 圖形函式庫
#include "Adafruit_ILI9341.h"    // ILI9341 TFT 驅動函式庫

// ===== TFT 液晶顯示初始化 =====
#define TFT_DC 5
#define TFT_CS 17
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// ===== RTC 初始化 =====
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// ===== 4x4 鍵盤設定 =====
const byte ROWS = 4;
const byte COLS = 4;
char keysArr[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// 對應引腳設定 ROW(列) & COL(行)
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keysArr), rowPins, colPins, ROWS, COLS);

// ===== 遊戲設定參數 =====
const uint8_t CELL = 10;         // 每格大小：10 像素
const uint8_t COLS_GRID = 32;    // 橫向格數 (320 像素 / 10)
const uint8_t ROWS_GRID = 24;    // 縱向格數 (240 像素 / 10)

// 貪吃蛇結構
struct Point { int8_t x; int8_t y; };

Point snake[COLS_GRID * ROWS_GRID];  // 貪吃蛇的每一節座標
int snakeLen = 0;                    // 蛇長度
Point food;                          // 食物座標

int dirX = 1, dirY = 0;              // 初始方向 (向右)
bool gameOver = false;

unsigned long lastMove = 0;
unsigned long moveInterval = 200;    // 每 200ms 移動一次

// ===== 使用 RTC 記錄遊戲時間 =====
unsigned long startMs = 0, endMs = 0;        // 若無 RTC，用 millis() 作回退
unsigned long startEpoch = 0, endEpoch = 0;  // 若有 RTC，用 epoch（秒）
bool recordedTime = false;

// ===== 畫出一個方格 =====
void drawCell(int x, int y, uint16_t color) {
  tft.fillRect(x * CELL, y * CELL, CELL, CELL, color);
}

// 比較兩個點是否相等
bool pointEqual(const Point &a, const Point &b) { return a.x==b.x && a.y==b.y; }

// 隨機生成食物位置，避免與蛇身重疊
void placeFood() {
  while (true) {
    int fx = random(0, COLS_GRID);
    int fy = random(0, ROWS_GRID);
    Point p{(int8_t)fx, (int8_t)fy};
    bool onSnake = false;
    for (int i=0;i<snakeLen;i++)
      if (pointEqual(snake[i], p)) { onSnake=true; break; }
    if (!onSnake) { food = p; break; }
  }
  drawCell(food.x, food.y, ILI9341_RED); // 畫出紅色食物
}

// ===== 記錄遊戲開始時間 =====
void recordGameStart() {
  recordedTime = false;
  if (rtc.isrunning()) {              // 如果 RTC 運行正常
    startEpoch = rtc.now().unixtime();
  } else {
    startMs = millis();
  }
}

// ===== 記錄遊戲結束時間並顯示時間 =====
void recordGameEnd() {
  if (recordedTime) return;           // 只記錄一次
  if (rtc.isrunning()) {
    endEpoch = rtc.now().unixtime();
  } else {
    endMs = millis();
  }

  unsigned long durationSec = 0;
  if (rtc.isrunning()) {
    durationSec = (endEpoch >= startEpoch) ? endEpoch - startEpoch : 0;
  } else {
    durationSec = (endMs >= startMs) ? (endMs - startMs) / 1000 : 0;
  }

  // 顯示遊戲時間
  int m = durationSec / 60;
  int s = durationSec % 60;
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setCursor(10, 60);
  tft.print("Game Time: ");
  tft.print(m);
  tft.print(":");
  if (s < 10) tft.print('0');
  tft.print(s);

  // 若有 RTC 顯示開始與結束時間戳
  if (rtc.isrunning()) {
    DateTime st(startEpoch);
    DateTime ed(endEpoch);
    char buf[64];
    snprintf(buf, sizeof(buf), "Start: %04u/%02u/%02u %02u:%02u:%02u",
             st.year(), st.month(), st.day(), st.hour(), st.minute(), st.second());
    tft.setCursor(10, 74);
    tft.print(buf);

    snprintf(buf, sizeof(buf), "End:   %04u/%02u/%02u %02u:%02u:%02u",
             ed.year(), ed.month(), ed.day(), ed.hour(), ed.minute(), ed.second());
    tft.setCursor(10, 86);
    tft.print(buf);
  }

  Serial.print("Game duration (s): ");
  Serial.println(durationSec);
  recordedTime = true;
}

// ===== 初始化遊戲 =====
void initGame() {
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRect(0, 0, COLS_GRID*CELL-1, ROWS_GRID*CELL-1, ILI9341_WHITE);

  // 初始化蛇的位置（畫面中央）
  snakeLen = 3;
  int cx = COLS_GRID/2;
  int cy = ROWS_GRID/2;
  for (int i=0;i<snakeLen;i++) {
    snake[i].x = cx - i;
    snake[i].y = cy;
  }
  dirX = 1; dirY = 0; gameOver=false;

  // 畫出蛇
  for (int i=0;i<snakeLen;i++)
    drawCell(snake[i].x, snake[i].y, ILI9341_GREEN);

  // 放食物
  placeFood();

  // 記錄開始時間
  recordGameStart();
}

// 根據鍵盤輸入重新啟動遊戲
void restartIfRequested(char k) {
  if (k == '#') { initGame(); }
}

// ===== Arduino 初始化 =====
void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);

  // 啟動畫面
  tft.setCursor(10,10);
  tft.print("Press A to start Snake (use 2/4/6/8)");
  tft.setCursor(10,20);
  tft.print("# = restart, * = pause/clear");

  // 初始化 RTC（容錯處理）
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  randomSeed(analogRead(0)); // 初始化亂數種子
}

// ===== 顯示當前時間在畫面底部 =====
void showTimeOnTop() {
  if (rtc.isrunning()) {
    DateTime now = rtc.now();
    char buf[40];
    snprintf(buf, sizeof(buf), "%04u/%02u/%02u %02u:%02u:%02u",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    tft.fillRect(0, 240 - 10, 320, 10, ILI9341_BLACK);
    tft.setCursor(2, 240 - 10);
    tft.print(buf);
  }
}

// ===== 主要遊戲迴圈 =====
void loop() {
  char k = keypad.getKey();
  if (k) {
    // 2/4/6/8 控制上下左右
    if (k=='2' && !(dirY==1 && dirX==0)) { dirX=0; dirY=-1; }      // 上
    else if (k=='5' && !(dirY==-1 && dirX==0)) { dirX=0; dirY=1; } // 下
    else if (k=='4' && !(dirX==1 && dirY==0)) { dirX=-1; dirY=0; } // 左
    else if (k=='6' && !(dirX==-1 && dirY==0)) { dirX=1; dirY=0; } // 右
    else if (k=='B') { initGame(); }                                // 重新開始
    else if (k=='C') { gameOver = true; recordGameEnd(); }          // 暫停
    else if (k=='A') { if (gameOver) initGame(); else if (snakeLen==0) initGame(); }
  }

  // 每隔 moveInterval 移動蛇
  if (!gameOver && millis() - lastMove >= moveInterval) {
    lastMove = millis();
    Point newHead = { (int8_t)(snake[0].x + dirX), (int8_t)(snake[0].y + dirY) };

    // 撞牆檢測
    if (newHead.x < 0 || newHead.x >= COLS_GRID || newHead.y < 0 || newHead.y >= ROWS_GRID) {
      gameOver = true;
      tft.setCursor(10, 30);
      tft.setTextSize(2);
      tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
      tft.print("GAME OVER");
      tft.setTextSize(1);
      tft.setCursor(10, 50);
      tft.print("Press # to restart");
      recordGameEnd();
      return;
    }

    // 撞自己檢測
    for (int i=0;i<snakeLen;i++)
      if (pointEqual(snake[i], newHead)) { gameOver=true; break; }
    if (gameOver) { recordGameEnd(); return; }

    // 移動：尾巴消除，頭前進
    Point tail = snake[snakeLen-1];
    drawCell(tail.x, tail.y, ILI9341_BLACK);
    for (int i=snakeLen-1;i>0;i--) snake[i]=snake[i-1];
    snake[0]=newHead;

    // 畫頭
    drawCell(newHead.x, newHead.y, ILI9341_GREEN);

    // 吃到食物
    if (pointEqual(newHead, food)) {
      snakeLen++;
      snake[snakeLen-1] = tail; // 尾巴延伸
      placeFood();
      // 隨著長度加快遊戲速度
      if (moveInterval > 60) moveInterval -= 5;
    }
  }

  // 每秒刷新一次時間顯示
  static unsigned long lastTimeUpdate=0;
  if (millis() - lastTimeUpdate >= 1000) {
    lastTimeUpdate = millis();
    showTimeOnTop();
  }
}
