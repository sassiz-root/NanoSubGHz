/*
 * ============================================================
 *  NanoSubGHz - Sub-GHz RF Tool (Educational)
 *  Author : Sassiz.Root
 *  Board  : Arduino Nano (ATmega328P)
 *  Libs   : RCSwitch, LiquidCrystal_I2C, SD, SPI
 *
 *  ⚠️  DISCLAIMER:
 *  This project is for educational/research use ONLY.
 *  Jamming, replaying, or intercepting RF signals may be
 *  ILLEGAL in your country. Use only on your own devices
 *  in a controlled environment.
 *
 *  💡 RAM OPTIMIZATION NOTES:
 *  - Serial is disabled (saves ~128 B RAM)
 *  - All strings in F()/PROGMEM
 *  - File processed line-by-line (never fully loaded)
 *  - Optional: reduce Wire.h BUFFER_LENGTH to 16 (saves 16 B)
 * ============================================================
 */

#include <RCSwitch.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <avr/pgmspace.h>

// ---------- PIN MAP ----------
#define RX433_PIN     2
#define RX315_PIN     3
#define TX433_PIN     4
#define TX315_PIN     5
#define BTN_LEFT      6
#define BTN_RIGHT     7   // reserved
#define BTN_UP        8
#define BTN_DOWN      9
#define SD_CS_PIN    10
#define BTN_SELECT   A0
#define BTN_BACK     A1
#define AGC_433_PIN  A6   // analog only
#define AGC_315_PIN  A7   // analog only

#define FREQ_433 433
#define FREQ_315 315

// ---------- Objects ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

RCSwitch rx433, rx315, tx433, tx315;

// ---------- PROGMEM MENU ----------
const char m0[] PROGMEM = "1.Sniff";
const char m1[] PROGMEM = "2.View Code";
const char m2[] PROGMEM = "3.Jam";
const char m3[] PROGMEM = "4.Brute-Force";
const char m4[] PROGMEM = "5.Clear Data";
const char m5[] PROGMEM = "6.Shutdown";
const char* const menuItems[] PROGMEM = { m0, m1, m2, m3, m4, m5 };
#define MENU_COUNT 6

// ---------- State machine ----------
enum State {
  STATE_SPLASH, STATE_BOOTING,
  STATE_MENU,
  STATE_SNIFF_FREQ, STATE_SNIFF,
  STATE_VIEW_FREQ, STATE_VIEW_LIST,
  STATE_BRUTE_FREQ, STATE_BRUTE_BITS, STATE_BRUTE_RUN,
  STATE_JAM,
  STATE_CLEAR_CONFIRM,
  STATE_SHUTDOWN
};
State currentState = STATE_SPLASH;

// ---------- Context ----------
uint8_t menuIndex     = 0;
uint16_t currentFreq  = FREQ_433;
int16_t viewIndex     = 0;
int16_t viewTotal     = 0;
uint16_t bruteFreq    = FREQ_433;
uint8_t  bruteBits    = 12;
uint32_t bruteCurrent = 0;

uint8_t sniffFreqSel  = 0;
uint8_t viewFreqSel   = 0;
uint8_t bruteFreqSel  = 0;
uint8_t bruteBitsSel  = 0;

// ---------- Buttons ----------
struct Button { uint8_t pin; bool last; unsigned long db; };
Button btnLeft   = {BTN_LEFT,   HIGH, 0};
Button btnUp     = {BTN_UP,     HIGH, 0};
Button btnDown   = {BTN_DOWN,   HIGH, 0};
Button btnSelect = {BTN_SELECT, HIGH, 0};
Button btnBack   = {BTN_BACK,   HIGH, 0};

// =====================================================================
//  Helpers
// =====================================================================
void printP(const char* s) {
  char c;
  while ((c = pgm_read_byte(s++)) != 0) lcd.print(c);
}

bool readButton(Button &b) {
  bool now = digitalRead(b.pin);
  if (now != b.last) {
    if (millis() - b.db > 30) {
      b.db = millis();
      b.last = now;
      if (now == LOW) return true;
    }
  }
  return false;
}

// ---- AGC / RSSI: 0..5 ----
uint8_t readRSSI(uint16_t freq) {
  uint8_t pin = (freq == FREQ_433) ? AGC_433_PIN : AGC_315_PIN;
  uint16_t a = analogRead(pin);
  uint16_t b = analogRead(pin);
  uint16_t avg = (a + b) >> 1;
  // NOTE: If your module is inverted (strong=low V),
  // change to: return 5 - (avg * 5L) / 1023L;
  return (uint8_t)((avg * 5L) / 1023L);
}

// =====================================================================
//  Menu
// =====================================================================
void drawMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print('>');
  printP((const char*)pgm_read_word(&menuItems[menuIndex]));
  uint8_t next = (menuIndex + 1) % MENU_COUNT;
  lcd.setCursor(0, 1);
  lcd.print(' ');
  printP((const char*)pgm_read_word(&menuItems[next]));
}

// =====================================================================
//  File helpers
// =====================================================================
const char* fileFor(uint16_t freq) {
  return (freq == FREQ_433) ? "codes_433.csv" : "codes_315.csv";
}

void saveCode(uint16_t freq, uint32_t code, uint8_t proto,
              uint8_t bits, uint8_t rssi) {
  File f = SD.open(fileFor(freq), FILE_WRITE);
  if (!f) return;
  f.print(code, HEX);   f.print(',');
  f.print(proto);       f.print(',');
  f.print(bits);        f.print(',');
  f.print(rssi);        f.print(',');
  f.println(F("ASK"));
  f.close();
}

int16_t countLines(const char* path) {
  File f = SD.open(path);
  if (!f) return 0;
  int16_t n = 0;
  while (f.available()) if (f.read() == '\n') n++;
  f.close();
  return n;
}

bool readLine(const char* path, int16_t target, char* buf, size_t sz) {
  File f = SD.open(path);
  if (!f) return false;
  int16_t line = 0;
  size_t idx = 0;
  while (f.available()) {
    char c = f.read();
    if (c == '\n') {
      if (line == target) { buf[idx] = 0; f.close(); return true; }
      line++; idx = 0;
    } else if (line == target && idx < sz - 1) {
      buf[idx++] = c;
    }
  }
  bool ok = (line == target);
  if (ok) buf[idx] = 0;
  f.close();
  return ok;
}

void deleteLine(const char* path, int16_t target) {
  SD.remove("temp.csv"); // cleanup stale temp
  File in  = SD.open(path);
  File out = SD.open("temp.csv", FILE_WRITE);
  if (!in || !out) { if (in) in.close(); if (out) out.close(); return; }
  int16_t line = 0;
  while (in.available()) {
    char c = in.read();
    if (line != target) out.write(c);
    if (c == '\n') line++;
  }
  in.close(); out.close();
  SD.remove(path);
  SD.rename("temp.csv", path);
}

// =====================================================================
//  Splash / Boot
// =====================================================================
void showSplash() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("NanoSubGhz"));
  lcd.setCursor(0, 1); lcd.print(F("by Sassiz.Root"));
  delay(2000);
}

void showBooting() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Booting..."));
  delay(1200);
}

// =====================================================================
//  Frequency select UI
// =====================================================================
void drawFreqLine(uint8_t sel) {
  lcd.setCursor(0, 1);
  if (sel == 0) lcd.print(F(">433   315    "));
  else          lcd.print(F(" 433  >315    "));
}

void drawSniffFreqSelect() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Sniff Frequency"));
  drawFreqLine(sniffFreqSel);
}
void drawViewFreqSelect() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("View Frequency"));
  drawFreqLine(viewFreqSel);
}
void drawBruteFreqSelect() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Brute Frequency"));
  drawFreqLine(bruteFreqSel);
}

// =====================================================================
//  Sniff
// =====================================================================
void startSniff(uint16_t freq) {
  currentFreq = freq;
  rx433.disableReceive();
  rx315.disableReceive();
  if (freq == FREQ_433) rx433.enableReceive(digitalPinToInterrupt(RX433_PIN));
  else                  rx315.enableReceive(digitalPinToInterrupt(RX315_PIN));

  lcd.clear();
  lcd.setCursor(0, 0);
  if (freq == FREQ_433) lcd.print(F("Sniff 433 MHz"));
  else                  lcd.print(F("Sniff 315 MHz"));
  lcd.setCursor(0, 1); lcd.print(F("L=Exit"));
  delay(300);
  currentState = STATE_SNIFF;
}

void handleSniff() {
  RCSwitch* rx = (currentFreq == FREQ_433) ? &rx433 : &rx315;
  if (rx->available()) {
    uint32_t code = rx->getReceivedValue();
    if (code != 0) {
      uint8_t bits  = rx->getReceivedBitlength();
      uint8_t proto = rx->getReceivedProtocol();
      uint8_t rssi  = readRSSI(currentFreq);
      saveCode(currentFreq, code, proto, bits, rssi);

      // short display
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(currentFreq);
      lcd.print(F("MHz RSSI:"));
      lcd.print(rssi);
      lcd.setCursor(0, 1);
      lcd.print(F("C:"));
      lcd.print(code, HEX);
      delay(600);

      // restore header
      lcd.clear();
      lcd.setCursor(0, 0);
      if (currentFreq == FREQ_433) lcd.print(F("Sniff 433 MHz"));
      else                         lcd.print(F("Sniff 315 MHz"));
      lcd.setCursor(0, 1); lcd.print(F("L=Exit"));
    }
    rx->resetAvailable();
  }

  if (readButton(btnLeft)) {
    rx433.disableReceive();
    rx315.disableReceive();
    currentState = STATE_MENU;
    drawMenu();
  }
}

// =====================================================================
//  View codes
// =====================================================================
void drawViewList() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentFreq);
  lcd.print(F("MHz "));
  lcd.print((int)(viewIndex + 1));
  lcd.print('/');
  lcd.print((int)viewTotal);
  lcd.setCursor(0, 1); lcd.print(F("Sel Info L=Back"));
}

void startViewList(uint16_t freq) {
  currentFreq = freq;
  viewTotal = countLines(fileFor(freq));
  viewIndex = 0;
  if (viewTotal == 0) {
    lcd.clear(); lcd.print(F("No Codes")); delay(800);
    currentState = STATE_MENU; drawMenu(); return;
  }
  drawViewList();
  currentState = STATE_VIEW_LIST;
}

// detail returns true if list changed (delete)
bool showDetail(int16_t idx) {
  char buf[64];
  if (!readLine(fileFor(currentFreq), idx, buf, sizeof(buf))) {
    lcd.clear(); lcd.print(F("Read Err")); delay(600); return false;
  }
  char* t = strtok(buf, ","); if (!t) return false;
  uint32_t code = strtoul(t, NULL, 16);
  t = strtok(NULL, ","); int proto = t ? atoi(t) : 0;
  t = strtok(NULL, ","); int bits  = t ? atoi(t) : 0;
  t = strtok(NULL, ","); int rssi  = t ? atoi(t) : 0;
  t = strtok(NULL, ","); const char* mod = t ? t : "?";

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("C:")); lcd.print(code, HEX);
  lcd.setCursor(0, 1);
  lcd.print(F("P")); lcd.print(proto);
  lcd.print(F(" B")); lcd.print(bits);
  lcd.print(F(" R")); lcd.print(rssi);
  lcd.print(' '); lcd.print(mod);
  delay(500);

  while (true) {
    if (readButton(btnSelect)) {
      // replay
      if (currentFreq == FREQ_433) { tx433.setProtocol(proto); tx433.send(code, bits); }
      else                         { tx315.setProtocol(proto); tx315.send(code, bits); }
      lcd.setCursor(0, 0);
      lcd.print(F("Sent!         "));
      delay(700);
      lcd.setCursor(0, 0);
      lcd.print(F("C:"));
      lcd.print(code, HEX);
      lcd.print(F("       "));
    }
    if (readButton(btnBack)) {
      deleteLine(fileFor(currentFreq), idx);
      lcd.clear(); lcd.print(F("Deleted")); delay(500);
      return true;
    }
    if (readButton(btnLeft)) {
      return false;
    }
  }
}

// =====================================================================
//  Jam
// =====================================================================
void enterJam() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Jamming 433+315"));
  lcd.setCursor(0, 1); lcd.print(F("L=Stop"));
  delay(1500);            // give user time to read
  lcd.noBacklight();      // save power (jam may run long)

  rx433.disableReceive();
  rx315.disableReceive();
  pinMode(TX433_PIN, OUTPUT);
  pinMode(TX315_PIN, OUTPUT);
  currentState = STATE_JAM;
}

void handleJam() {
  static unsigned long last = 0;
  static bool tog = false;
  if (micros() - last > 100) {   // ~10 kHz OOK
    last = micros();
    tog = !tog;
    digitalWrite(TX433_PIN, tog);
    digitalWrite(TX315_PIN, tog);
  }
  if (readButton(btnLeft)) {
    digitalWrite(TX433_PIN, LOW);
    digitalWrite(TX315_PIN, LOW);
    pinMode(TX433_PIN, INPUT);
    pinMode(TX315_PIN, INPUT);
    lcd.backlight();
    currentState = STATE_MENU;
    drawMenu();
  }
}

// =====================================================================
//  Brute-force
// =====================================================================
void drawBruteBitsSelect() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Brute Bit Length"));
  lcd.setCursor(0, 1);
  if (bruteBitsSel == 0) lcd.print(F(">12b  16b(~30m)"));
  else                   lcd.print(F(" 12b >16b(~30m)"));
}

void startBruteRun() {
  bruteCurrent = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Brute "));
  lcd.print(bruteFreq);
  lcd.print(F("MHz "));
  lcd.print(bruteBits);
  lcd.print(F("b"));
  lcd.setCursor(0, 1); lcd.print(F("L=Stop"));
  currentState = STATE_BRUTE_RUN;
}

void handleBruteRun() {
  uint32_t maxCode = (1UL << bruteBits) - 1;
  if (bruteCurrent > maxCode) {
    lcd.clear(); lcd.print(F("Brute Done")); delay(800);
    currentState = STATE_MENU; drawMenu(); return;
  }
  if (readButton(btnLeft)) {
    lcd.clear(); lcd.print(F("Stopped")); delay(500);
    currentState = STATE_MENU; drawMenu(); return;
  }
  RCSwitch* tx = (bruteFreq == FREQ_433) ? &tx433 : &tx315;
  tx->setProtocol(1);
  tx->send(bruteCurrent, bruteBits);

  if ((bruteCurrent & 0x1F) == 0) {
    lcd.setCursor(0, 1);
    lcd.print(F("C:"));
    lcd.print(bruteCurrent, HEX);
    lcd.print(F("       "));
  }
  bruteCurrent++;
}

// =====================================================================
//  Clear / Shutdown
// =====================================================================
void clearAllData() {
  SD.remove("codes_433.csv");
  SD.remove("codes_315.csv");
  // recreate empty files
  File a = SD.open("codes_433.csv", FILE_WRITE); if (a) a.close();
  File b = SD.open("codes_315.csv", FILE_WRITE); if (b) b.close();
  lcd.clear(); lcd.print(F("Data Cleared"));
  delay(800);
  currentState = STATE_MENU;
  drawMenu();
}

void doShutdown() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Shutting down"));
  delay(600);

  rx433.disableReceive();
  rx315.disableReceive();
  digitalWrite(TX433_PIN, LOW);
  digitalWrite(TX315_PIN, LOW);
  pinMode(TX433_PIN, INPUT);
  pinMode(TX315_PIN, INPUT);

  delay(300);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Safe Unplug"));
  lcd.setCursor(0, 1); lcd.print(F("Power off now"));
  lcd.noBacklight();

  // Stay here. User must unplug or press RESET.
  while (true) delay(1000);
}

// =====================================================================
//  Setup
// =====================================================================
void setup() {
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  pinMode(TX433_PIN, OUTPUT); digitalWrite(TX433_PIN, LOW);
  pinMode(TX315_PIN, OUTPUT); digitalWrite(TX315_PIN, LOW);

  lcd.init();
  lcd.backlight();

  if (!SD.begin(SD_CS_PIN)) {
    lcd.clear(); lcd.print(F("SD Init Fail")); delay(1500);
  }
  // Ensure files exist
  if (!SD.exists("codes_433.csv")) { File f = SD.open("codes_433.csv", FILE_WRITE); if (f) f.close(); }
  if (!SD.exists("codes_315.csv")) { File f = SD.open("codes_315.csv", FILE_WRITE); if (f) f.close(); }

  tx433.enableTransmit(TX433_PIN);
  tx315.enableTransmit(TX315_PIN);

  showSplash();
  showBooting();

  menuIndex = 0;
  drawMenu();
  currentState = STATE_MENU;
}

// =====================================================================
//  Main loop
// =====================================================================
void loop() {
  switch (currentState) {

    case STATE_MENU:
      if (readButton(btnUp)) {
        menuIndex = (menuIndex == 0) ? (MENU_COUNT - 1) : (menuIndex - 1);
        drawMenu();
      }
      if (readButton(btnDown)) {
        menuIndex = (menuIndex + 1) % MENU_COUNT;
        drawMenu();
      }
      if (readButton(btnSelect)) {
        switch (menuIndex) {
          case 0:
            sniffFreqSel = 0;
            drawSniffFreqSelect();
            currentState = STATE_SNIFF_FREQ;
            break;
          case 1:
            viewFreqSel = 0;
            drawViewFreqSelect();
            currentState = STATE_VIEW_FREQ;
            break;
          case 2:
            enterJam();
            break;
          case 3:
            bruteFreqSel = 0;
            drawBruteFreqSelect();
            currentState = STATE_BRUTE_FREQ;
            break;
          case 4:
            lcd.clear();
            lcd.setCursor(0, 0); lcd.print(F("Clear all?"));
            lcd.setCursor(0, 1); lcd.print(F("Sel=Y  Back=N"));
            currentState = STATE_CLEAR_CONFIRM;
            break;
          case 5:
            currentState = STATE_SHUTDOWN;
            doShutdown();
            break;
        }
      }
      break;

    case STATE_SNIFF_FREQ:
      if (readButton(btnUp) || readButton(btnDown)) {
        sniffFreqSel ^= 1;
        drawFreqLine(sniffFreqSel);
      }
      if (readButton(btnSelect)) {
        startSniff(sniffFreqSel == 0 ? FREQ_433 : FREQ_315);
      }
      if (readButton(btnLeft)) {
        currentState = STATE_MENU; drawMenu();
      }
      break;

    case STATE_SNIFF:
      handleSniff();
      break;

    case STATE_VIEW_FREQ:
      if (readButton(btnUp) || readButton(btnDown)) {
        viewFreqSel ^= 1;
        drawFreqLine(viewFreqSel);
      }
      if (readButton(btnSelect)) {
        startViewList(viewFreqSel == 0 ? FREQ_433 : FREQ_315);
      }
      if (readButton(btnLeft)) {
        currentState = STATE_MENU; drawMenu();
      }
      break;

    case STATE_VIEW_LIST:
      if (readButton(btnUp) && viewIndex > 0) {
        viewIndex--; drawViewList();
      }
      if (readButton(btnDown) && viewIndex < viewTotal - 1) {
        viewIndex++; drawViewList();
      }
      if (readButton(btnSelect)) {
        bool changed = showDetail(viewIndex);
        if (changed) {
          viewTotal = countLines(fileFor(currentFreq));
          if (viewTotal == 0) {
            lcd.clear(); lcd.print(F("No Codes")); delay(600);
            currentState = STATE_MENU; drawMenu(); break;
          }
          if (viewIndex >= viewTotal) viewIndex = viewTotal - 1;
        }
        drawViewList();
      }
      if (readButton(btnLeft) || readButton(btnBack)) {
        currentState = STATE_MENU; drawMenu();
      }
      break;

    case STATE_BRUTE_FREQ:
      if (readButton(btnUp) || readButton(btnDown)) {
        bruteFreqSel ^= 1;
        drawFreqLine(bruteFreqSel);
      }
      if (readButton(btnSelect)) {
        bruteFreq = (bruteFreqSel == 0) ? FREQ_433 : FREQ_315;
        bruteBitsSel = 0;
        drawBruteBitsSelect();
        currentState = STATE_BRUTE_BITS;
      }
      if (readButton(btnLeft)) {
        currentState = STATE_MENU; drawMenu();
      }
      break;

    case STATE_BRUTE_BITS:
      if (readButton(btnUp) || readButton(btnDown)) {
        bruteBitsSel ^= 1;
        drawBruteBitsSelect();
      }
      if (readButton(btnSelect)) {
        bruteBits = (bruteBitsSel == 0) ? 12 : 16;
        startBruteRun();
      }
      if (readButton(btnLeft)) {
        currentState = STATE_MENU; drawMenu();
      }
      break;

    case STATE_BRUTE_RUN:
      handleBruteRun();
      break;

    case STATE_JAM:
      handleJam();
      break;

    case STATE_CLEAR_CONFIRM:
      if (readButton(btnSelect)) clearAllData();
      if (readButton(btnBack) || readButton(btnLeft)) {
        currentState = STATE_MENU; drawMenu();
      }
      break;

    case STATE_SHUTDOWN:
      // handled inside doShutdown() — infinite loop
      break;

    default:
      currentState = STATE_MENU;
      drawMenu();
      break;
  }
}
