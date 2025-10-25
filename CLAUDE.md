# How to Upload Examples to CYD with arduino-cli

This guide documents how to compile and upload examples to the ESP32 Cheap Yellow Display (CYD) using arduino-cli.

## Prerequisites

- arduino-cli installed and configured
- ESP32 board support installed (`esp32:esp32`)
- USB connection to the CYD board

## Setup Steps

### 1. Check Connected Board

```bash
arduino-cli board list
```

This should show your CYD connected (typically at `/dev/ttyUSB0` on Linux).

### 2. Verify ESP32 Core is Installed

```bash
arduino-cli core list
```

If `esp32:esp32` is not listed, install it:

```bash
arduino-cli core install esp32:esp32
```

### 3. Install TFT_eSPI Library

The CYD examples use the TFT_eSPI library:

```bash
arduino-cli lib install "TFT_eSPI"
```

### 4. Configure TFT_eSPI for CYD

The TFT_eSPI library needs to be configured specifically for the CYD hardware. Copy the CYD-specific configuration file:

```bash
cp /path/to/ESP32-Cheap-Yellow-Display/DisplayConfig/User_Setup.h ~/Arduino/libraries/TFT_eSPI/User_Setup.h
```

This configuration file sets up:
- The correct display driver (ILI9341_2_DRIVER)
- Pin mappings for the CYD
- SPI settings
- Touch screen controller settings

### 5. Compile the Sketch

Navigate to the example directory and compile:

```bash
cd /path/to/ESP32-Cheap-Yellow-Display/Examples/Basics/1-HelloWorld
arduino-cli compile --fqbn esp32:esp32:esp32 1-HelloWorld.ino
```

### 6. Upload to the Board

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 1-HelloWorld.ino
```

Note: Replace `/dev/ttyUSB0` with your actual port if different.

## Expected Result

After uploading the HelloWorld example, you should see:
- "Hello" displayed in white text at the top left
- "World" displayed in blue text, centered below it
- Black background
- Display in landscape orientation

## Compilation Stats

The HelloWorld example uses:
- Program storage: ~315KB (24% of available space)
- Dynamic memory: ~20KB (6% of available RAM)

## Troubleshooting

### Permission Issues

If you get permission errors accessing `/dev/ttyUSB0`, add your user to the dialout group:

```bash
sudo usermod -a -G dialout $USER
```

Then log out and log back in.

### Upload Fails

If upload fails, try:
1. Press and hold the BOOT button on the CYD
2. Press the RESET button briefly
3. Release the BOOT button
4. Retry the upload command

### Display Shows Nothing

Make sure you copied the correct `User_Setup.h` file to the TFT_eSPI library directory. The stock configuration won't work with the CYD.

## Other Examples

Once you have the basic setup working, you can try other examples:

- Touch screen test: `Examples/Basics/2-TouchTest/`
- SD Card test: `Examples/Basics/3-SDCardTest/`
- Backlight control: `Examples/Basics/4-BacklightControlTest/`
- LDR sensor: `Examples/Basics/5-LDRTest/`
- LED test: `Examples/Basics/6-LEDTest/`
- LVGL GUI: `Examples/LVGL9/LVGL_Arduino/`

All examples follow the same compile and upload process.

## Quick Command Reference

```bash
# List boards
arduino-cli board list

# List installed libraries
arduino-cli lib list

# Compile
arduino-cli compile --fqbn esp32:esp32:esp32 <sketch.ino>

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 <sketch.ino>

# Compile and upload in one command
arduino-cli compile --upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 <sketch.ino>
```
