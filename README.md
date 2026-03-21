# PawPoint Locator

IoT pet presence and location tracker: an **ESP8266** reads **RFID tags** to know when a registered pet leaves or returns, pushes **GPS coordinates** and status to **Firebase Realtime Database**, and a small **web dashboard** shows live ID, message, coordinates, and date.

---

## What it does

1. **RFID scan** — When a tag is presented to the MFRC522 reader, the firmware compares its UID to a configured “valid pet” UID and optional “known invalid” UIDs.
2. **Status in the cloud** — Messages and flags are written to Firebase (`Ljubimac/*`): whether the pet is home, whether the tag is valid, and the scanned ID.
3. **Outside + GPS** — For the valid pet tag, when the pet is considered “outside,” the firmware reads a **NEO-6M** (or compatible) GPS module via **TinyGPS++** and uploads latitude/longitude (`Koordinate/*`) and date (`Datum/*`).
4. **Return home** — A second scan flow (counter in firmware) can mark the pet as back home and reset state.
5. **Dashboard** — `PawPoint Locator.html` uses the Firebase JS SDK to poll the database and fill the form fields; **Google Maps Lokacija** opens a Google search from the coordinate string.

---

## Repository layout

| Path | Purpose |
|------|--------|
| `PawPoint Locator code/PPL/PPL.ino` | Main Arduino sketch (WiFi, Firebase, RFID, GPS) |
| `PawPoint Locator.html`, `js.js`, `css.css` | Static web UI + Firebase client |
| `libraries/` | Vendored Arduino libraries (Firebase ESP client, MFRC522, TinyGPS / TinyGPS++, etc.) |
| `PowerPoint Locator Izvjestaj.docx` | Project report (if applicable) |

---

## Hardware (as used in the sketch)

- **MCU:** ESP8266 (NodeMCU-style board; sketch uses `D0`–`D7` style pins)
- **RFID:** MFRC522 (SPI)
- **GPS:** UART module (e.g. NEO-6M) at **9600 baud**, read with `SoftwareSerial` in the sketch
- **LED:** On `D0` (optional visual feedback; some LED paths are commented out)
- **LCD (optional):** I2C LCD is referenced in comments and includes; `setup()` LCD calls are **commented out** in the current sketch

Pin notes and wiring hints are written **in the header of `PPL.ino`**. Before flashing, align **physical wiring** with the **actual `SoftwareSerial` and SPI pin definitions** in the sketch—comments and code can drift (e.g. GPS vs LCD pins).

---

## Software requirements

- [Arduino IDE](https://www.arduino.cc/en/software) (or PlatformIO with equivalent board/libraries)
- **Board support:** ESP8266 (e.g. *esp8266* package by ESP8266 Community)
- **Libraries** (you can use the copies under `libraries/` or install via Library Manager where names match):
  - `Firebase Arduino Client Library for ESP8266 and ESP32`
  - `MFRC522`
  - `TinyGPS++` (sketch includes `TinyGPS++.h`)
  - `LiquidCrystal_I2C` (if you enable the LCD)

Open the sketch from:

`PawPoint Locator code/PPL/PPL.ino`

Select your ESP8266 board and correct **COM port**, then compile and upload.

---

## Firebase configuration

1. Create a Firebase project and enable **Realtime Database** (same region as your app if you care about latency).
2. In **Project settings**, copy the **Web API key** and **database URL** into:
   - `PPL.ino`: `API_KEY`, `DATABASE_URL`, and Wi-Fi `WIFI_SSID` / `WIFI_PASSWORD`
   - `PawPoint Locator.html`: `firebaseConfig` object (must match the same project)
3. **Security rules:** The sample uses anonymous-style signup in the firmware (`Firebase.signUp` with empty email/password). For anything beyond a classroom demo, lock down RTDB rules, use proper authentication, and **do not commit real secrets** to a public repo.

### RTDB paths (reference)

| Path | Role |
|------|------|
| `Ljubimac/Poruka` | Human-readable status message |
| `Ljubimac/ID` | Last scanned UID string |
| `Ljubimac/LjubimacJeKuci` | Home vs away (integer flag) |
| `Ljubimac/ValidanLjubimac` | Whether the tag matched the registered pet |
| `Koordinate/GEOSirina`, `Koordinate/GEODuzina` | Latitude / longitude strings |
| `Datum/Dan`, `Datum/Mjesec`, `Datum/Godina` | GPS date fields |

---

## Web dashboard

1. Serve or open `PawPoint Locator.html` from a context where relative links resolve (`css.css`, `js.js`, `PawPoint logo.png` in the same folder), or adjust paths.
2. The page loads Firebase from the CDN and periodically refreshes fields (see inline script in the HTML).
3. **Google Maps Lokacija** builds a Google search URL from the coordinate field (see `js.js`).

---

## Customizing pet tags

In `PPL.ino`, UIDs are compared as space-separated decimal byte strings, for example:

- `KodValidLjubimac` — registered pet
- `KodNotValidLjubimac*` / `KodNotValidKartica*` — tags that should show as not yours

Read your tag’s UID from the Serial Monitor after a scan, then update these constants.

---

## Security warning

This repository may contain **Wi-Fi credentials, Firebase API keys, and database URLs** in source files. If the project is or will be public:

- **Rotate** Firebase keys and database rules after any leak.
- Move secrets to **ignored config** or environment-specific files and keep **rules** strict in production.

---

## License / third-party code

Arduino libraries under `libraries/` retain their respective authors’ licenses. Use and redistribution of those folders should follow each library’s license terms.
