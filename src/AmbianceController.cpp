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
        delay(500);

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
    static uint8_t buffer[MAX_MSG_LENGTH];
    static unsigned int buffer_ind = 0;
    static unsigned long last_packet_time = millis();

    if(connected) {
        while(Serial.available() > 0) {
            // If data present on serial, grab a single byte of it
            uint8_t byte_in = Serial.parseInt();

            // Debugging output
            Serial.println("Byte: " + String(byte_in));

            // Add it to the filtered byte buffer (exc. EOL and OoR)
            // with \n representing the end of an RGB triple
            if(buffer_ind < MAX_MSG_LENGTH)
            {
                buffer[buffer_ind] = byte_in;
                buffer_ind = buffer_ind + 1;
            }
            else {
                // Clear buffer and set color for the current led
                buffer_ind = 0;
                leds[led_ind] = CRGB(buffer[0], buffer[1], buffer[2]);
                // Debugging output
                Serial.println("LED " + String(led_ind) + ": " + buffer[0] + ", " + buffer[1] + ", " + buffer[2]);
                led_ind = led_ind + 1;

                // If we have received all our led data then update
                // strip and set index to beginning of next frame
                if(led_ind >= NUM_LEDS) {
                    FastLED.show();
                    led_ind = 0;
                }
            }
        }
    }
    else establish_handshake();
}