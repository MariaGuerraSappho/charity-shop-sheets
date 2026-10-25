#include <Audio.h>
#include <SD.h>
#include <SPI.h>

// ---- Audio graph ----
AudioPlaySdWav      playWav1;
AudioOutputI2S      i2s1;
AudioConnection     pc1(playWav1, 0, i2s1, 0);
AudioConnection     pc2(playWav1, 1, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;

// ---- Piezo on A0 ----
const int PIEZO_PIN = A0;

// simple baseline & threshold detection
int    base      = 600;         // will auto-tune on start
const  int warmN = 200;         // samples to learn baseline
int    warmCnt   = 0;

const int TRIGGER_DELTA = 90;   // adjust: lower = more sensitive
const unsigned long REFRACT_MS = 120;
unsigned long lastTrig = 0;

void setup() {
  Serial.begin(115200);
  AudioMemory(16);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD init failed");
    while (1) { delay(500); }
  }
  Serial.println("Starting…");

  // quick baseline learn
  long acc = 0;
  for (int i = 0; i < warmN; ++i) { acc += analogRead(PIEZO_PIN); delay(2); }
  base = acc / warmN;
  Serial.print("Base learned = "); Serial.println(base);
}

void loop() {
  int raw = analogRead(PIEZO_PIN);
  int delta = abs(raw - base);

  // slow baseline drift correction (ignores big hits)
  if (delta < 20) base = (base * 99 + raw) / 100;

  // print a few times per second
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 150) {
    Serial.print("raw="); Serial.print(raw);
    Serial.print("  base="); Serial.print(base);
    Serial.print("  delta="); Serial.println(delta);
    lastPrint = millis();
  }

  // trigger
  if (delta > TRIGGER_DELTA && (millis() - lastTrig) > REFRACT_MS) {
    Serial.println("TRIGGER → TEST.WAV");
    playWav("TEST.WAV");          // file in SD root
    lastTrig = millis();
  }

  delay(2);
}

void playWav(const char *name) {
  if (playWav1.isPlaying()) playWav1.stop();
  playWav1.play(name);
  delay(5); // allow to start
}
