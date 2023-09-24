# Waky

## Introduction

Waky is an application responsible for detecting pre-configured word ("Marvin"),
record a speech and send recorded audio to a backend for further processing.

## Dependencies

* Platform.io toolkit
* ESP-IDF (ver. 5.1)
* TensorFlow Lite Micro (git submodule)
* ESP-NN (git submodule)
* KissFFT (git submodule)

Note: The TensorFlow Lite component is responsible for recognizing wake word using trained model and histogram of recorded sound.

Note: The KissFFT component is responsible for making histogram of recorded sound.

## Pins

The pin-out for INMP441 sensor is next:

| INMP441 | ESP32  | Info                          |
|---------|--------|-------------------------------|
| L/R     | GND    | Left channel or right channel |
| WS      | GPIO22 | Left right clock              |
| SCK     | GPIO26 | Serial clock                  |
| SD      | GPIO21 | Serial data                   |
| VDD     | 3v3    | Power - DO NOT USE 5V!        |
| GND     | GND    | GND                           |

## Build

* Clone
```shell
$ git clone <url>
$ cd waky
$ git submodule update --init --recursive
```

* Build
```shell
$ pio run -t menuconfig
$ pio run
```

Note: Visit "Waky configuration" submenu

* Upload
```shell
$ pio run -t upload
```

* Connect
```shell
$ pio run -t monitor
```
