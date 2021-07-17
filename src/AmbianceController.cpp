/**
 *  AmbianceController.cpp
 *  @brief Firmware written for Arduino devices, to interface with the corresponding
 *  Ambiance desktop application. The device acts as a bias lighting system for
 *  media displayed on the desktop's screen by setting dynamic color zones on a
 *  LED strip.
 * 
 * @author Logan Surface
 * @date July 17, 2021
 */

#include <Arduino.h>
#include <FastLED.h>
#include <Configuration.h>

bool connected = false;
static CRGB leds[NUM_LEDS];

/*
Handshake protocol
------------------
- Loop while the connected flag is false
- Repeatedly print 'R' to serial, signaling controller is ready
- If a callback of 'R' is recieved on the serial buffer from device:
    -> Toggle the connection flag
    -> Print 'B' to serial, signaling okay to begin
*/

/**
 * Connect to the desktop application with a series of serial flags.
 */
void establish_handshake() {
    while(!connected) {
        Serial.print('R');

        if(Serial.available() > 0) {
            byte byte_in = Serial.read();

            if(byte_in == 'R') {
                connected = true;
                Serial.print('B');
            }
        }
    }
}

void setup() {
    Serial.begin(BAUDRATE);
    establish_handshake();

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
    static unsigned int led_ind = 0;
    static char buffer[MAX_MSG_LENGTH];
    static unsigned int buffer_ind = 0;

    if(connected) {
        while(Serial.available() > 0) {
            // If data present on serial, grab a single byte of it
            byte byte_in = Serial.read();

            // Add it to the filtered byte buffer (exc. EOL and OoR)
            // with \n representing the end of an RGB triple
            if(byte_in != '\n' && buffer_ind < MAX_MSG_LENGTH)
                buffer[buffer_ind++] = byte_in;
            else
                // Clear buffer and set color for the current led
                buffer_ind = 0;
                leds[led_ind++] = CRGB(buffer[0], buffer[1], buffer[2]);

                // If we have received all our led data then update
                // strip and set index to beginning of next frame
                if(led_ind >= NUM_LEDS) {
                    FastLED.show();
                    led_ind = 0;
                }
        }

        // If the data steam halts, then connection was terminated
        connected = false;
    }
    else establish_handshake();
}