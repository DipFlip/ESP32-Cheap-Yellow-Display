# Weather and BART Display

Displays current Berkeley weather and the next 4 BART trains departing from Rockridge station on the Cheap Yellow Display.

## Features

- Current weather for Berkeley, CA (temperature and conditions)
- Next 4 BART trains from Rockridge sorted by departure time
- Color-coded BART line indicators
- Updates every 2 minutes (API calls)
- Display refreshes every minute (countdown between API calls)
- Smart polling to reduce API usage

## Setup Instructions

### 1. Install Required Libraries

Using arduino-cli:
```bash
arduino-cli lib install "TFT_eSPI"
arduino-cli lib install "ArduinoJson"
```

### 2. Configure TFT_eSPI

Copy the CYD-specific configuration file:
```bash
cp ../../../DisplayConfig/User_Setup.h ~/Arduino/libraries/TFT_eSPI/User_Setup.h
```

### 3. Create Credentials File

Copy the example credentials file and fill in your values:
```bash
cp credentials.h.example credentials.h
```

Then edit `credentials.h` with your actual credentials:
- WiFi SSID and password
- BART API key (get one free at https://api.bart.gov/api/register.aspx)

**Important:** `credentials.h` is gitignored and will not be committed to version control.

### 4. Compile and Upload

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 WeatherAndBART.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 WeatherAndBART.ino
```

## APIs Used

### Open-Meteo (Weather)
- **URL:** https://open-meteo.com
- **API Key:** Not required
- **Rate Limit:** 10,000 requests/day (free tier)
- **Our Usage:** ~720 requests/day
- **Data Source:** NOAA/National Weather Service

### BART Real-Time API
- **URL:** https://api.bart.gov
- **API Key:** Required (free)
- **Register:** https://api.bart.gov/api/register.aspx
- **Rate Limit:** Not explicitly documented
- **Our Usage:** ~720 requests/day (polling every 2 minutes)

## How It Works

### Smart Polling Strategy

1. **Every 2 minutes:** Fetch fresh data from APIs
2. **Every 1 minute (in between):** Update display by subtracting 1 minute from train times
3. This reduces API calls by 50% while keeping the display fresh

### Display Layout

```
Berkeley: 52F Clear
Rockridge BART:
  3m  San Francisco    ●
  7m  Antioch          ●
  12m Millbrae/SFO     ●
  15m San Francisco    ●
API: 45s ago
```

- Top line: Current weather
- 4 trains: departure time, destination, and line color dot
- Bottom: Time since last API call

## Customization

You can modify these values in the code:

- **Location:** Change `latitude` and `longitude` (lines 27-28)
- **BART Station:** Change `ROCK` to another station code in `fetchBART()` (line 154)
- **Update Intervals:**
  - `apiInterval` (line 47): How often to poll APIs
  - `displayInterval` (line 48): How often to refresh display

## Troubleshooting

### WiFi Connection Failed
- Check your credentials in `credentials.h`
- Ensure you're in range of your WiFi network

### No BART Data
- Verify your BART API key is valid
- Check if BART service is running (no trains late at night)
- Ensure the station code is correct

### Weather Shows "N/A"
- Check internet connectivity
- Open-Meteo might be temporarily down (rare)

## Files

- `WeatherAndBART.ino` - Main sketch
- `credentials.h` - Your secrets (gitignored, you create this)
- `credentials.h.example` - Template for credentials
- `README.md` - This file

## License

Same as the parent ESP32-Cheap-Yellow-Display project (MIT)
