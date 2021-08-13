/**
 *  AmbianceController.cpp
 *  @brief Firmware written for Arduino devices to interface with the corresponding
 *  Ambiance desktop application. The arduino device acts as a bias lighting system for
 *  media displayed on the desktop's screen by updating dynamic color zones on a
 *  LED strip behind the display.
 * 
 * @author Logan Surface
 * @date July 17, 2021
 */

#include <Arduino.h>
#include <FastLED.h>
#include <Configuration.h>

bool connected = false;
CRGB frame[NUM_LEDS];

/**
 * Connect to the Ambiance desktop application using a series of serial flags.
 * 
 * @param ready_flag The byte printed to serial indicating ready for connection.
 * @param proceed_flag The byte printed to serial signaling a connection was established.
 * 
 * @returns A boolean state of true upon a successful connection.
 */
bool serial_handshake(byte ready_flag, byte proceed_flag) {
    while(!connected) {
        Serial.write(ready_flag);
        delay(500);

        if(Serial.available() > 0) {
            byte byte_in = Serial.read();

            if(byte_in == ready_flag) {
                Serial.write(proceed_flag);
                return true;
            }
        }
    }
}

void setup() {
    Serial.begin(BAUDRATE);
    Serial.setTimeout(10);

    // Establish a connection with the serving device + init. leds
    connected = serial_handshake('R', 'F');
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(frame, NUM_LEDS);
}

void loop() {
    static unsigned int led_ind = 0;

    if(Serial.available()) {
        byte temp_char = Serial.read();

        if(temp_char == '\n') {
            FastLED.show();
            led_ind = 0;
        }
        else {
            // If temp_char isn't EOL, then it has to be
            // the first color value in an RGB set.
            uint8_t r_in = temp_char - '0';
            uint8_t g_in = Serial.parseInt();
            uint8_t b_in = Serial.parseInt();

            frame[led_ind++] = CRGB(r_in, b_in, g_in);
        }
    }

    // Allow the serving device to catch up if it's data stream halts.
    // WARNING: I don't know if this will work as intended
    delay(50);
}