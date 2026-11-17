
#define PIN_SCK 13
#define PIN_SDA 11
#define PIN_CS  10
#define PIN_RST 9


void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_RST, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_SDA, OUTPUT);

  hardReset();
  sendCommand(0x11); // Выводим из сна (некоторые регистры заблокированы в Sleep In)
  delay(150);

  Serial.println("=== 1. Опрос идентификаторов (ID) ===");
  uint32_t id04 = readReg(0x04, 24, true);
  uint8_t  da   = readReg(0xDA, 8,  false);
  uint8_t  db   = readReg(0xDB, 8,  false);
  uint8_t  dc   = readReg(0xDC, 8,  false);
  uint32_t d3   = readReg(0xD3, 24, true);
  uint32_t d3_32= readReg(0xD3, 32, true); // На случай 4-байтного ответа (0x00 + 3 байта)

  printHex("CMD 0x04 (RDDID 24-bit): ", id04, 6);
  printHex("CMD 0xDA (ID1 - Module): ", da,   2);
  printHex("CMD 0xDB (ID2 - Ver)   : ", db,   2);
  printHex("CMD 0xDC (ID3 - Driver): ", dc,   2);
  printHex("CMD 0xD3 (ID4 24-bit)  : ", d3,   6);
  printHex("CMD 0xD3 (ID4 32-bit)  : ", d3_32,8);

  Serial.println("\n=== 2. Опрос статуса и внутренних регистров ===");
  uint32_t st09 = readReg(0x09, 32, true);
  uint8_t  pm0a = readReg(0x0A, 8,  false);
  uint8_t  sd0f = readReg(0x0F, 8,  false);
  uint32_t fd   = readReg(0xFD, 24, true); // Внутренний регистр NewVision NV3029
  uint32_t ef   = readReg(0xEF, 24, true); // Внутренний регистр Ilitek / GC

  printHex("CMD 0x09 (Status 32b)  : ", st09, 8);
  printHex("CMD 0x0A (Power Mode)  : ", pm0a, 2);
  printHex("CMD 0x0F (Self-Diag)   : ", sd0f, 2);
  printHex("CMD 0xFD (NV3029 check): ", fd,   6);
  printHex("CMD 0xEF (Vendor check): ", ef,   6);

  Serial.println("\n=== 3. Автоматический вердикт ===");
  identifyChip(id04, da, d3, d3_32, fd);

  Serial.println("\n=== 4. Полный дамп всех живых регистров (0x00..0xFF) ===");
  fullScan();
}

void loop() {}

void identifyChip(uint32_t id04, uint8_t da, uint32_t d3, uint32_t d3_32, uint32_t fd) {
  // 1. Проверка чипов GalaxyCore (GC9106 / GC9107) и Ilitek через 0xD3
  if ((d3 & 0xFFFF) == 0x9106 || (d3_32 & 0xFFFF) == 0x9106) {
    Serial.println("-> Контроллер: GalaxyCore GC9106 (определен по 0xD3)");
    return;
  }
  if ((d3 & 0xFFFF) == 0x9107 || (d3_32 & 0xFFFF) == 0x9107) {
    Serial.println("-> Контроллер: GalaxyCore GC9107 (определен по 0xD3)");
    return;
  }
  if ((d3 & 0xFFFF) == 0x9163 || (d3_32 & 0xFFFF) == 0x9163) {
    Serial.println("-> Контроллер: Ilitek ILI9163 (определен по 0xD3)");
    return;
  }

  // 2. Проверка NewVision NV3029
  if ((fd & 0xFFFF) == 0x3029 || (id04 & 0xFFFF) == 0x3029) {
    Serial.println("-> Контроллер: NewVision NV3029");
    return;
  }

  // 3. Проверка по первому байту ID1 (0xDA)
  if (da == 0x54) {
    Serial.println("-> Контроллер: Ilitek ILI9163 (заводской код Ilitek 0x54)");
  } else if (da == 0x5C || da == 0x7C) {
    Serial.println("-> Контроллер: Sitronix ST7735 / ST7735R / ST7735S (заводской код Sitronix)");
  } else if (da == 0xE3) {
    Serial.println("-> Контроллер: Sitronix ST7735S с OEM-прошивкой модуля Nokia (0xE3)");
  } else if (id04 == 0x000000 || id04 == 0xFFFFFF) {
    Serial.println("-> ID не прошит в OTP или не поддерживается. Смотрите дамп ниже.");
  } else {
    Serial.println("-> Нестандартный OEM-код. Совместим с базовым профилем ST7735 / ILI9163.");
  }
}

void fullScan() {
  for (uint16_t cmd = 0x00; cmd <= 0xFF; cmd++) {
    uint32_t rNoDummy = readReg((uint8_t)cmd, 32, false);
    uint32_t rDummy   = readReg((uint8_t)cmd, 32, true);

    bool ok1 = (rNoDummy != 0x00000000 && rNoDummy != 0xFFFFFFFF);
    bool ok2 = (rDummy   != 0x00000000 && rDummy   != 0xFFFFFFFF);

    if (ok1 || ok2) {
      Serial.print("CMD 0x");
      if (cmd < 0x10) Serial.print("0");
      Serial.print(cmd, HEX);
      Serial.print(" -> NoDummy: 0x");
      printHexRaw(rNoDummy, 8);
      Serial.print(" | Dummy: 0x");
      printHexRaw(rDummy, 8);
      Serial.println();
    }
  }
}

void hardReset() {
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_SCK, LOW);
  digitalWrite(PIN_RST, HIGH);
  delay(20);
  digitalWrite(PIN_RST, LOW);
  delay(50);
  digitalWrite(PIN_RST, HIGH);
  delay(150);
}

uint32_t readReg(uint8_t cmd, uint8_t bits, bool dummyClock) {
  uint32_t result = 0;
  digitalWrite(PIN_CS, LOW);
  delayMicroseconds(5);

  pinMode(PIN_SDA, OUTPUT);
  sendBit(0); // D/CX = 0 (команда)
  for (int i = 7; i >= 0; i--) {
    sendBit((cmd >> i) & 1);
  }

  pinMode(PIN_SDA, INPUT_PULLUP);
  delayMicroseconds(5);

  if (dummyClock) {
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(5);
  }

  for (uint8_t i = 0; i < bits; i++) {
    digitalWrite(PIN_SCK, LOW);
    delayMicroseconds(5);
    result <<= 1;
    if (digitalRead(PIN_SDA)) result |= 1;
    digitalWrite(PIN_SCK, HIGH);
    delayMicroseconds(5);
  }

  digitalWrite(PIN_CS, HIGH);
  delayMicroseconds(10);
  return result;
}

void sendCommand(uint8_t cmd) {
  digitalWrite(PIN_CS, LOW);
  pinMode(PIN_SDA, OUTPUT);
  sendBit(0);
  for (int i = 7; i >= 0; i--) {
    sendBit((cmd >> i) & 1);
  }
  digitalWrite(PIN_CS, HIGH);
}

void sendBit(bool bitVal) {
  digitalWrite(PIN_SCK, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_SDA, bitVal);
  delayMicroseconds(5);
  digitalWrite(PIN_SCK, HIGH);
  delayMicroseconds(5);
}

void printHex(const char* label, uint32_t val, uint8_t digits) {
  Serial.print(label);
  Serial.print("0x");
  printHexRaw(val, digits);
  Serial.println();
}

void printHexRaw(uint32_t val, uint8_t digits) {
  for (int i = (digits - 1) * 4; i >= 0; i -= 4) {
    Serial.print((val >> i) & 0xF, HEX);
  }
}
