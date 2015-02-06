/*
 * lightwand.ino - Runs light wand
 * Copyright (c) 2015 Matthew Petroff
 * http://mpetroff.net
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <OctoWS2811.h>
#include <SPI.h>
#include <SD.h>

#define STRIP_LENGTH 144

#define FILENAME1 "img1.bin"
#define FILENAME2 "img2.bin"
#define FILENAME3 "img3.bin"
#define FILENAME4 "img4.bin"

#define CS_PIN 10
#define LED_CONFIG WS2811_GRB | WS2811_800kHz

#define BTN1 28
#define BTN2 27
#define BTN3 29
#define BTN4 30

DMAMEM int displayMemory[STRIP_LENGTH * 6];
int drawingMemory[STRIP_LENGTH * 6];
OctoWS2811 leds(STRIP_LENGTH, displayMemory, drawingMemory, LED_CONFIG);

void setup() {
    // Initialize LEDs
    leds.begin();
    leds.show();
    
    // Initialize microSD card
    if(!SD.begin(CS_PIN)) {
        display_error(-1);
    }
    
    // Configure buttons
    pinMode(BTN1, INPUT_PULLUP);
    pinMode(BTN2, INPUT_PULLUP);
    pinMode(BTN3, INPUT_PULLUP);
    pinMode(BTN4, INPUT_PULLUP);
}

void loop() {
    // Display image when button is pressed
    if (digitalRead(BTN1) == LOW) {
        display_image(FILENAME1);
    } else if (digitalRead(BTN2) == LOW) {
        display_image(FILENAME2);
    } else if (digitalRead(BTN3) == LOW) {
        display_image(FILENAME3);
    } else if (digitalRead(BTN4) == LOW) {
        display_image(FILENAME4);
    }
    delay(100);
}

/*
 * Display an image.
 */
void display_image(char *filename) {
    // Wait two seconds
    delay(2000);
    
    // Open image
    File image = SD.open(filename, FILE_READ);
    if (!image) {
        display_error(3);
        return;
    }
    
    // Read start delay configuration and wait
    delay(image.read() + (image.read() << 8));
    
    // Read line delay configuration
    unsigned int lineDelay = image.read() + (image.read() << 8);
    
    // Display image
    boolean btnReleased = false;
    for (;;) {
        if (image.read(drawingMemory, sizeof(drawingMemory)) > 0) {
            leds.show();
            delay(lineDelay);
        } else {
            break;
        }
        
        // Stop if a button is pressed
        if (digitalRead(BTN1) == HIGH && digitalRead(BTN2) == HIGH
            || digitalRead(BTN3) == HIGH || digitalRead(BTN4) == HIGH) {
            btnReleased = true;
        }
        if (btnReleased && (digitalRead(BTN1) == LOW
            || digitalRead(BTN2) == LOW || digitalRead(BTN3) == LOW
            || digitalRead(BTN4) == LOW)) {
            delay(1000);
            break;
        }
    }
    clear_strip();
    
    // Close image
    image.close();
}

/*
 * Blink LEDs red a certain number of times to signify an error. For an
 * unrecoverable error, pass -1 to make the strip blink forever (effectively).
 */
void display_error(int count) {
    for (int i = count; i != 0; i--) {
        for (int i = 0; i < STRIP_LENGTH; i++) {
            leds.setPixel(i, 0xff0000);
        }
        leds.show();
        delay(500);
        clear_strip();
        delay(500);
    }
}

/*
 * Clear LED strip.
 */
void clear_strip() {
    for (int i = 0; i < STRIP_LENGTH; i++) {
        leds.setPixel(i, 0);
    }
    leds.show();
}
