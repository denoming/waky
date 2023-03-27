# J.A.R.V.I.S

# Introduction

J.A.R.V.I.S Agent is an applciation reponsible to detect pre-configured word ("Marvin"), record a speech and send to backend for further processing.

# Dependencies

* ESP-IDF (ver. 4.4.3)
* TensorFlow Lite Micro ([repository](https://github.com/espressif/tflite-micro-esp-examples.git))
* KissFFT ([repository](https://github.com/karz0n/esp32-kissfft.git))

Note: The TensorFlow Lite component is responsible for recognizing wake word using trained model and gistogram of recorded sound.

Note: The KissFFT component is responsible for making gistogram of recorded sound.

# Prepare

* Install [Visual Studio Code](https://code.visualstudio.com/) IDE
* Install [PlatformIO IDE](https://platformio.org/platformio-ide)
* Open IDE and pre-install ESP-IDF with particular version

# Configure

* Open project in IDE
* On the left panel click on PlatformIDE icon
* Click on "PlatformIO Core CLI"
* Run command:
```shell
$ pio run -t menuconfig
```
* Apply configuration (see J.A.R.V.I.S Agent Configuration)

Note: Usually you might need to change "Backend" configuration (URL address and port). Any other configration is not needed.

# Build

* Clone dependencies using helpfull script:
```shell
$ cd <project-path>
$ bash scripts/setup.sh
```
* Open project in IDE
* Build
