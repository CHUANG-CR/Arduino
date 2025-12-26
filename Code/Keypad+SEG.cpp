#include <Keypad.h>
#include <Arduino.h>
// -------- Keypad 設定 --------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27};     // 依你的實際接線修改
byte colPins[COLS] = {26, 25, 33, 32};     // 依你的實際接線修改

Keypad keypad = Keypad (makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------- 7 段顯示器腳位 (共陰極) --------
const int segPins[7] = {21,19,18,5,17,16,4}; // a b c d e f g

// 0~9 的段碼 (a~g)，1 = 亮，0 = 滅
int digits[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

void setup() {
  Serial.begin(115200);
  Serial.println("Keypad diagnostic starting...");

  for (int i = 0; i < 7; i++) {
    pinMode(segPins[i], OUTPUT);
    digitalWrite(segPins[i], LOW);
  }

  // Ensure keypad pins are in a known state for diagnostics
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT);
  }
}

// 顯示一個 0~9 數字
void showDigit(int n) {
  if (n < 0 || n > 9) return;
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], digits[n][i]);
  }
}

// 全部關閉
void clearDisplay() {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], LOW);
  }
}

unsigned long lastDiagMillis = 0;

void diagScan() {
  Serial.println("--- Keypad diag scan ---");
  for (byte c = 0; c < COLS; c++) {
    // Drive this column active
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);
    delay(5);
    for (byte r = 0; r < ROWS; r++) {
      pinMode(rowPins[r], INPUT_PULLUP);
      int v = digitalRead(rowPins[r]);
      Serial.print('C'); Serial.print(c); Serial.print("->R"); Serial.print(r); Serial.print('='); Serial.print(v); Serial.print(' ');
    }
    Serial.println();
    // Release column
    digitalWrite(colPins[c], HIGH);
    pinMode(colPins[c], INPUT);
  }
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key detected: "); Serial.println(key);
    if (key >= '0' && key <= '9') {
      int num = key - '0';  // 轉成 0~9
      showDigit(num);
    } else {
      // 非數字鍵就關掉顯示（或依需要自訂）
      clearDisplay();
    }
  }

  if (millis() - lastDiagMillis >= 1000) {
    diagScan();
    lastDiagMillis = millis();
  }
}
