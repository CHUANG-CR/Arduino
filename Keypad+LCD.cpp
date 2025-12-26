#include <LiquidCrystal.h>
#include <Arduino.h>
void resetSystem();
void updatePasswordDisplay();
bool scan();
// Keypad列腳及欄腳腳位定義
int rows[] = {13, 12, 14, 27};
int cols[] = {26, 25, 33, 32};
int row = 4;  // Row數量
int col = 4;  // Column數量

int i, x, y;  // 用於儲存掃描時行與列的索引

// Keypad鍵值映射表(4x4矩陣)
char tbl[] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

// LiquidCrystal LCD 腳位接法(rs, en, d4, d5, d6, d7)
const int rs = 16, en = 17, d4 = 5, d5 = 18, d6 = 19, d7 = 21;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// 儲存使用者輸入的密碼字串
String inputPassword = "";

// 預設的正確密碼
const String correctPassword = "1234";

// 紀錄狀態變化時間，用於顯示完結果後自動重置
unsigned long lastStateChangeTime = 0;

// 設定結果顯示維持時間：5秒
const unsigned long displayDelay = 5000;

enum State {
  WAITING_INPUT, // 等待使用者輸入狀態
  SHOW_RESULT    // 顯示密碼驗證結果狀態
};
// 初始狀態為等待輸入
State systemState = WAITING_INPUT;

void setup() {
  lcd.begin(16, 2);   // 初始化LCD為16列2行
  lcd.print("Pass:"); // 第一行顯示"Pass:"
  Serial.begin(9600); // 初始化序列埠便於除錯使用
  
  // 設定Keypad列腳為輸出
  for (i = 0; i < row; i++)
    pinMode(rows[i], OUTPUT);

  // 設定Keypad欄腳為輸入且啟用內建上拉電阻
  for (i = 0; i < col; i++)
    pinMode(cols[i], INPUT_PULLUP);
}

bool now, old = 0; // 用於偵測按鍵狀態變化

void loop() {
  now = scan(); // 掃描Keypad是否有按鍵按下

  // 如果檢測到按鍵從未按到按下（下降沿）
  if (now == 1 && old == 0) {
    char key = tbl[x * 4 + y]; // 取得當前按鍵字符
    Serial.print("Key pressed: ");
    Serial.println(key);       // 將按鍵輸出到序列監控器以便除錯

    if (systemState == WAITING_INPUT) { // 僅等待輸入時才處理輸入
      if (key == 'D') {
        // 按'D'鍵功能：刪除最後一個輸入字元
        if (inputPassword.length() > 0) {
          inputPassword.remove(inputPassword.length() - 1);
          updatePasswordDisplay(); // 更新LCD上的星號顯示
        }
      }
      else if (key >= '0' && key <= '9') { // 僅接受數字鍵
        if (inputPassword.length() < 4) { // 限制輸入長度最多4個字元
          inputPassword += key; // 將輸入加入密碼字串
          updatePasswordDisplay();
        }
        // 若輸入已達4字元，自動切換狀態驗證密碼
        if (inputPassword.length() == 4) {
          systemState = SHOW_RESULT;
          lcd.setCursor(0, 1); // 移至LCD第二行
          if (inputPassword == correctPassword) {
            lcd.print("PASS OK!      ");      // 密碼正確顯示成功訊息
          } else {
            lcd.print("PASS Failure! ");      // 密碼錯誤顯示失敗訊息
          }
          lastStateChangeTime = millis(); // 紀錄目前時間
        }
      }
    }
    // SHOW_RESULT狀態時按鍵無作用（可依需求添加功能）
  }

  old = now; // 更新舊按鍵狀態

  // 若目前為顯示結果狀態，判斷是否需重置系統
  if (systemState == SHOW_RESULT) {
    if (millis() - lastStateChangeTime > displayDelay) {
      resetSystem(); // 顯示5秒後清除並回到等待輸入狀態
    }
  }
}

// 掃描4x4矩陣按鍵，判斷是否有按鍵被按下
bool scan() {
  bool Pressed = false;

  // 掃描第1列，其他列輸出高
  digitalWrite(rows[0], LOW);
  digitalWrite(rows[1], HIGH);
  digitalWrite(rows[2], HIGH);
  digitalWrite(rows[3], HIGH);
  for (int i = 0; i < col; i++) {
    if (digitalRead(cols[i]) == 0) { // 檢測是否有按鍵接地
      Pressed = true;
      x = 0;    // 記錄行號
      y = i;    // 記錄列號
      return Pressed;
    }
  }

  // 掃描第2列
  digitalWrite(rows[0], HIGH);
  digitalWrite(rows[1], LOW);
  digitalWrite(rows[2], HIGH);
  digitalWrite(rows[3], HIGH);
  for (int i = 0; i < col; i++) {
    if (digitalRead(cols[i]) == 0) {
      Pressed = true;
      x = 1;
      y = i;
      return Pressed;
    }
  }

  // 掃描第3列
  digitalWrite(rows[0], HIGH);
  digitalWrite(rows[1], HIGH);
  digitalWrite(rows[2], LOW);
  digitalWrite(rows[3], HIGH);
  for (int i = 0; i < col; i++) {
    if (digitalRead(cols[i]) == 0) {
      Pressed = true;
      x = 2;
      y = i;
      return Pressed;
    }
  }

  // 掃描第4列
  digitalWrite(rows[0], HIGH);
  digitalWrite(rows[1], HIGH);
  digitalWrite(rows[2], HIGH);
  digitalWrite(rows[3], LOW);
  for (int i = 0; i < col; i++) {
    if (digitalRead(cols[i]) == 0) {
      Pressed = true;
      x = 3;
      y = i;
      return Pressed;
    }
  }

  return Pressed; // 無按鍵按下
}

// 根據輸入的字串長度更新LCD密碼欄位，超過位置顯示空白
void updatePasswordDisplay() {
  lcd.setCursor(5, 0); // 定位在"Pass:"後開始顯示位置
  for (int i = 0; i < 4; i++) {
    if (i < inputPassword.length()) {
      lcd.print("*");  // 每輸入一個字元顯示一顆星號
    } else {
      lcd.print(" ");  // 沒有輸入用空白填充
    }
  }
  lcd.setCursor(0, 1);
  lcd.print("                "); // 清除第二行顯示內容
}

// 將系統重置為等待密碼輸入的初始狀態
void resetSystem() {
  inputPassword = "";       // 清空已輸入密碼
  systemState = WAITING_INPUT;
  lcd.clear();              // 清除LCD畫面
  lcd.setCursor(0, 0);
  lcd.print("Pass:");       // 顯示初始提示
  lcd.setCursor(5, 0);
  lcd.print("    ");        // 清空星號顯示欄位
  lcd.setCursor(0, 1);
  lcd.print("                "); // 清除第二行
}
