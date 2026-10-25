#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ctype.h>  // for tolower

// -------------------- Pins ------------------------------
const int POT_PIN = A0;    // B10k pot on A0 for master volume

// -------------------- Audio objects ----------------------
AudioInputI2S        audioInput;     // piezo into LINE IN (sensor only)
AudioAnalyzePeak     peakLeft;       // for gesture detection

AudioPlaySdWav       playWav1;
AudioPlaySdWav       playWav2;
AudioPlaySdWav       playWav3;

AudioMixer4          mixL;
AudioMixer4          mixR;

AudioOutputI2S       audioOutput;
AudioControlSGTL5000 sgtl5000_1;

// Sample voices in an array for round-robin
AudioPlaySdWav* voices[] = { &playWav1, &playWav2, &playWav3 };
const int NUM_VOICES = sizeof(voices) / sizeof(voices[0]);
int nextVoice = 0;

// -------------------- Audio connections ------------------

// Piezo left channel → peak analyzer (sensing only)
AudioConnection patchMicToPeak(audioInput, 0, peakLeft, 0);

// Sample players to mixers (inputs 0–2)
AudioConnection patchWav1L(playWav1, 0, mixL, 0);
AudioConnection patchWav1R(playWav1, 1, mixR, 0);

AudioConnection patchWav2L(playWav2, 0, mixL, 1);
AudioConnection patchWav2R(playWav2, 1, mixR, 1);

AudioConnection patchWav3L(playWav3, 0, mixL, 2);
AudioConnection patchWav3R(playWav3, 1, mixR, 2);

// Mixers to outputs
AudioConnection patchMixLtoOut(mixL, 0, audioOutput, 0);
AudioConnection patchMixRtoOut(mixR, 0, audioOutput, 1);

// -------------------- Dynamic file lists -----------------
const int MAX_FILES = 32;
String longFiles[MAX_FILES];
int numLongFiles = 0;

String shortFiles[MAX_FILES];
int numShortFiles = 0;

// -------------------- Motion / trigger settings ----------
// MAIN SENSITIVITY KNOBS:

// Hit threshold: how big a peak to actually trigger a sound.
// Lower = more sensitive, Higher = less sensitive.
float HIT_THRESHOLD          = 0.008f;   // try 0.004–0.008 range

// Release factor: how far level must fall (relative to HIT_THRESHOLD)
// before we consider the hit "finished" and allow a new trigger.
const float HIT_RELEASE_FACTOR = 0.4f;   // 40% of HIT_THRESHOLD

// Energy threshold: how much motion counts towards "busy" energy.
float ENERGY_INPUT_THRESHOLD = 0.0015f;  // try 0.001–0.002 range

// Debounce: minimum ms between hits (as backup to edge-detection)
const uint32_t DEBOUNCE_MS     = 180;
uint32_t lastTriggerTime       = 0;

// Track whether we're currently "inside" a hit
bool hitActive                 = false;

// Motion energy model (memory of recent motion)
float motionEnergy             = 0.0f;   // 0.0 (calm) to ~1.0 (very busy)
const float ENERGY_DECAY_PER_SEC = 0.5f; // higher = forgets motion faster
uint32_t lastEnergyUpdate      = 0;

// End-of-motion tail
bool   tailPending             = false;
uint32_t lastHitTime           = 0;
const uint32_t QUIET_FOR_TAIL_MS = 1200; // ms of quiet before tail

// Threshold between "calm" and "busy" for long vs short
const float HIGH_ENERGY = 0.35f;  // lower = more shorts overall

// -------------------- SD initialisation ------------------
bool initSD() {
  Serial.println("Trying BUILTIN_SDCARD...");
  if (SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Using BUILTIN_SDCARD.");
    return true;
  }
  Serial.println("Trying Audio Shield SD (CS=10)...");
  if (SD.begin(10)) {
    Serial.println("Using Audio Shield SD on CS=10.");
    return true;
  }
  Serial.println("SD init failed on both slots!");
  return false;
}

// -------------------- Helpers: check .wav extension ------
bool isWavFile(const char* name) {
  const char* dot = strrchr(name, '.');
  if (!dot) return false;
  dot++;
  if (!dot[0] || !dot[1] || !dot[2]) return false;

  char c1 = tolower(dot[0]);
  char c2 = tolower(dot[1]);
  char c3 = tolower(dot[2]);

  return (c1 == 'w' && c2 == 'a' && c3 == 'v');
}

// -------------------- Scan a folder into a list ----------
void scanFolder(const char* folder, String* list, int &count, int maxCount) {
  count = 0;

  Serial.print("Scanning folder: ");
  Serial.println(folder);

  File dir = SD.open(folder);
  if (!dir) {
    Serial.print("  Could not open folder ");
    Serial.println(folder);
    return;
  }

  File entry = dir.openNextFile();
  while (entry && count < maxCount) {
    if (!entry.isDirectory()) {
      const char* name = entry.name();

      if (isWavFile(name)) {
        String fullPath = String(folder) + "/" + String(name);
        list[count] = fullPath;
        Serial.print("  Found: ");
        Serial.println(fullPath);
        count++;
      }
    }

    entry.close();
    entry = dir.openNextFile();
  }

  dir.close();

  Serial.print("  Total .wav files in ");
  Serial.print(folder);
  Serial.print(": ");
  Serial.println(count);
}

// -------------------- Random pickers ---------------------
int pickRandomLongIndex() {
  if (numLongFiles <= 0) return -1;
  return random(numLongFiles);
}

int pickRandomShortIndex() {
  if (numShortFiles <= 0) return -1;
  return random(numShortFiles);
}

// -------------------- Setup ------------------------------
void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("\nPiezo → motion-based sample system (LONG/SHORT folders, edge-detected hits) starting...");

  pinMode(POT_PIN, INPUT);

  AudioMemory(60);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5f);          // overridden by pot in loop
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(12);       // adjust for sensitivity (0–15)

  if (!initSD()) {
    while (1) delay(1000);
  }

  // Scan LONG and SHORT folders
  scanFolder("/LONG",  longFiles,  numLongFiles,  MAX_FILES);
  scanFolder("/SHORT", shortFiles, numShortFiles, MAX_FILES);

  if (numLongFiles == 0) {
    Serial.println("WARNING: No files found in /LONG");
  }
  if (numShortFiles == 0) {
    Serial.println("WARNING: No files found in /SHORT");
  }

  // Mixer gains: all sample voices to same level
  mixL.gain(0, 0.9f);
  mixR.gain(0, 0.9f);
  mixL.gain(1, 0.9f);
  mixR.gain(1, 0.9f);
  mixL.gain(2, 0.9f);
  mixR.gain(2, 0.9f);
  mixL.gain(3, 0.0f);
  mixR.gain(3, 0.0f);

  lastEnergyUpdate = millis();

  Serial.println("-----------------------------------------------");
  Serial.println("• Put long sounds in /LONG, short in /SHORT");
  Serial.println("• Pot on A0 = overall volume");
  Serial.println("• Light taps/shakes → respond, idle → stay quiet");
  Serial.println("• Calm → /LONG, busy → /SHORT, then tail from /LONG");
  Serial.println("• To tune behaviour, adjust:");
  Serial.println("    HIT_THRESHOLD, ENERGY_INPUT_THRESHOLD, HIGH_ENERGY");
  Serial.println("-----------------------------------------------");
}

// -------------------- Trigger helper ---------------------
void triggerVoiceWithPath(const String &path) {
  if (path.length() == 0) return;

  int v = nextVoice;
  nextVoice = (nextVoice + 1) % NUM_VOICES;

  AudioPlaySdWav* player = voices[v];

  Serial.print("    Voice ");
  Serial.print(v);
  Serial.print(" -> ");
  Serial.println(path);

  if (player->isPlaying()) {
    player->stop();
  }

  player->play(path.c_str());
}

// -------------------- Main loop --------------------------
void loop() {
  uint32_t now = millis();

  // ---- POT: master volume ----
  {
    int raw = analogRead(POT_PIN);
    float vol = raw / 1023.0f;
    vol = vol * vol;          // curve for better low-end control
    sgtl5000_1.volume(vol);
  }

  // ---- Update motion energy (decay over time) ----
  {
    uint32_t dtMs = now - lastEnergyUpdate;
    if (dtMs > 0) {
      float dtSec = dtMs / 1000.0f;
      motionEnergy -= ENERGY_DECAY_PER_SEC * dtSec;
      if (motionEnergy < 0.0f) motionEnergy = 0.0f;
      lastEnergyUpdate = now;
    }
  }

  // ---- Process input level ----
  if (peakLeft.available()) {
    float level = peakLeft.read();

    // Accumulate motionEnergy from any noticeable movement
    if (level > ENERGY_INPUT_THRESHOLD) {
      motionEnergy += level * 3.0f;   // higher factor = energy builds faster
      if (motionEnergy > 1.0f) motionEnergy = 1.0f;
    }

    // Edge-based hit detection:
    // - When not in a hit and level crosses above HIT_THRESHOLD (and debounce ok) → trigger ONCE.
    // - While hitActive, we do NOT trigger again, even if level wiggles.
    // - When level drops below a lower threshold, we end the hit (hitActive = false).
    bool cooldown = (now - lastTriggerTime) < DEBOUNCE_MS;

    if (!hitActive) {
      // We're currently "not in a hit": look for rising edge
      if (!cooldown && level > HIT_THRESHOLD) {
        hitActive = true;         // we're now inside a hit
        lastHitTime = now;
        tailPending = true;

        Serial.print(">>> HIT (edge), level = ");
        Serial.print(level, 5);
        Serial.print(", motionEnergy = ");
        Serial.println(motionEnergy, 3);

        // Decide LONG vs SHORT based on motionEnergy
        if (motionEnergy < HIGH_ENERGY) {
          // calmer / single strikes → LONG
          if (numLongFiles > 0) {
            int idx = pickRandomLongIndex();
            Serial.print("  -> LONG: ");
            Serial.println(longFiles[idx]);
            triggerVoiceWithPath(longFiles[idx]);
          }
        } else {
          // busy / shaken → SHORT
          if (numShortFiles > 0) {
            int idx = pickRandomShortIndex();
            Serial.print("  -> SHORT: ");
            Serial.println(shortFiles[idx]);
            triggerVoiceWithPath(shortFiles[idx]);
          }
        }

        lastTriggerTime = now;
      }
    } else {
      // We're currently "inside" a hit: wait for level to drop
      float releaseThreshold = HIT_THRESHOLD * HIT_RELEASE_FACTOR;
      if (level < releaseThreshold) {
        // signal has fallen low enough; next peak can be a new hit
        hitActive = false;
      }
    }
  }

  // ---- End-of-motion tail logic ----
  {
    if (tailPending && numLongFiles > 0) {
      bool quietLongEnough = (now - lastHitTime) > QUIET_FOR_TAIL_MS;
      if (quietLongEnough && motionEnergy < 0.15f) {
        int idx = pickRandomLongIndex();
        Serial.print("Tail LONG: ");
        Serial.println(longFiles[idx]);
        triggerVoiceWithPath(longFiles[idx]);
        tailPending = false;  // one tail per burst
      }
    }
  }
}
