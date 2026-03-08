# Push Button Game

Ein Reaktionsspiel für Arduino mit großen Tastern, LED‑Anzeige, zwei TM1637‑Displays und einer 32×8 NeoPixel‑Matrix.  
Das Projekt wurde ursprünglich 2016 begonnen und 2025/2026 modernisiert.
Die Arbeiten sind noch nicht komplett abgeschlossen.

## 🎮 Features

- 4 große Spieltaster mit LED‑Feedback  
- 4 Spielmodi (verschiedene Geschwindigkeiten)  
- Countdown‑Animation auf der NeoPixel‑Matrix  
- Punkte‑ und Zeit‑Anzeige über zwei TM1637‑Displays  
- Sleep‑Modus  
- Entprellte Tastererkennung  
- Zufällige Trefferziele  
- Saubere Zustandsmaschine (State Machine)

---

## 🧩 Verwendete Libraries

### 1. **TM1637 Display Library**
- Typ: 7‑Segment Display Treiber  
- Lizenz: **MIT** oder **LGPL**, abhängig vom verwendeten Repo  
- Offizielle Version (empfohlen):  
  https://github.com/avishorp/TM1637  
- Lizenz:  
  https://github.com/avishorp/TM1637/blob/master/LICENSE

---

### 2. **Adafruit NeoPixel**
- Für LED‑Stripes und NeoPixel‑Matrix  
- Lizenz: **LGPL 3.0**  
- Repo:  
  https://github.com/adafruit/Adafruit_NeoPixel  
- Lizenz:  
  https://github.com/adafruit/Adafruit_NeoPixel/blob/master/COPYING

---

### 3. **Adafruit NeoMatrix**
- Matrix‑Layout‑Handling  
- Lizenz: **BSD**  
- Repo:  
  https://github.com/adafruit/Adafruit_NeoMatrix  
- Lizenz:  
  https://github.com/adafruit/Adafruit_NeoMatrix/blob/master/license.txt

---

### 4. **Adafruit GFX Library**
- Grafische Grundfunktionen (Text, Pixel, Linien)  
- Lizenz: **BSD**  
- Repo:  
  https://github.com/adafruit/Adafruit-GFX-Library  
- Lizenz:  
  https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt

---

## 🔧 Hardware

- Arduino Mega 2560  
- 4 große Arcade‑Buttons  
- 4 Setup‑Buttons  
- 8 LEDs  
- 32×8 NeoPixel‑Matrix (WS2812)  
- 2× TM1637 4‑Digit‑Displays  

---

## 📂 Projektstruktur


--------------------------------------------------------------------------------

# Push Button Game

A reaction-based arcade game for Arduino featuring large push buttons, LED indicators, two TM1637 7‑segment displays, and a 32×8 NeoPixel matrix.  
The project was originally created in 2016 and updated in 2024/2025.
The work has not yet been completely finished.

## 🎮 Features

- 4 large arcade-style game buttons with LED feedback  
- 4 game modes with different speeds  
- Countdown animation on the NeoPixel matrix  
- Score and remaining time displayed on two TM1637 displays  
- Sleep mode after inactivity  
- Debounced button handling  
- Randomized target button selection  
- Clear and structured state machine

---

## 🧩 Used Libraries

### 1. **TM1637 Display Library**
- Purpose: Control of 4‑digit 7‑segment displays  
- License: **MIT** or **LGPL**, depending on the version  
- Repository:  
  https://github.com/avishorp/TM1637  
- License file:  
  https://github.com/avishorp/TM1637/blob/master/LICENSE

---

### 2. **Adafruit NeoPixel**
- Purpose: Control of WS2812/NeoPixel LEDs  
- License: **LGPL 3.0**  
- Repository:  
  https://github.com/adafruit/Adafruit_NeoPixel  
- License file:  
  https://github.com/adafruit/Adafruit_NeoPixel/blob/master/COPYING

---

### 3. **Adafruit NeoMatrix**
- Purpose: Matrix layout and pixel mapping  
- License: **BSD**  
- Repository:  
  https://github.com/adafruit/Adafruit_NeoMatrix  
- License file:  
  https://github.com/adafruit/Adafruit_NeoMatrix/blob/master/license.txt

---

### 4. **Adafruit GFX Library**
- Purpose: Graphics primitives (text, lines, shapes)  
- License: **BSD**  
- Repository:  
  https://github.com/adafruit/Adafruit-GFX-Library  
- License file:  
  https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt

---

## 🔧 Hardware Requirements

- Arduino Mega 2560  
- 4 large arcade push buttons  
- 4 setup buttons  
- 8 LEDs  
- 32×8 NeoPixel matrix (WS2812)  
- 2× TM1637 4‑digit displays  

---

## 📂 Project Structure

