# Automated Heating System for the Original Prusa Enclosure

This repo contains the software for the Automated Heating System for the Original Prusa Enclosure, which is designed and published by me (user @lars at printables.com).

Link to the model on Printables: https://www.printables.com/model/561491

It is designed to run on an Arduino Nano (the orinal one with an ATMEGA328p), so it tries to uses little RAM while still being able to drive the 128x64px OLED display, which already eats up half of it.

Enabling serial without reducing the buffer size may result in crashes, as there is not that much RAM free then.

Thanks to @Prusa3D for designing a great enclosure and publishing STEP files of it - this project was only possible because of it.
