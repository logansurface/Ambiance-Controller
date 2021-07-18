/**
 *  AmbianceController.cpp
 *  @brief Firmware written for Arduino devices, to interface with the corresponding
 *  Ambiance desktop application. The device acts as a bias lighting system for
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
CRGB leds[NUM_LEDS];

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
        Serial.print('R');
        delay(500);

        if(Serial.available() > 0) {
            byte byte_in = Serial.read();

            if(byte_in == 'R') {
                Serial.print('B');
                return true;
            }
        }
    }
}

void setup() {
    Serial.begin(BAUDRATE);
    Serial.setTimeout(10);

    connected = serial_handshake('R', 'B');

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
    static unsigned int led_ind = 0;

    if(connected) {
        while(Serial.available() > 0) {
            uint8_t r_in = Serial.parseInt();
            uint8_t g_in = Serial.parseInt();
            uint8_t b_in = Serial.parseInt();

            leds[led_ind++] = CRGB(r_in, g_in, b_in);

            // When a complete frame has been recieved, refresh the
            // strip and move the index to beginning of next frame.
            if(led_ind >= NUM_LEDS) {
                FastLED.show();
                led_ind = 0;
            }
        }

        // If the data stream halts, the device has been disconnected
        connected = false;
    }
    else connected = serial_handshake('R', 'B');
}