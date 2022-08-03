# J.A.R.V.I.S

## Build

1. Clone TensorFlowLite component

```sh
$ git clone --recurse-submodules https://github.com/espressif/tflite-micro-esp-examples.git $HOME/tflite-micro
$ cd <project-path>
$ mkdir -p components && cd components
$ ln -s $HOME/tflite-micro/components/tflite-lib tflite-lib
$ ln -s $HOME/tflite-micro/components/esp-nn esp-nn
```
Note: The TensorFlow Lite component is responsible for recognizing wake word using trained model and gistogram of recorded sound.

2. Clone KissFFT component

```sh
$ git clone https://github.com/karz0n/esp32-kissfft.git $HOME/kissfft
$ cd <project-path>
$ mkdir -p components && cd components
$ ln -s $HOME/kissfft kissfft
```
Note: The KissFFT component is responsible for making gistogram of recorded sound.

3. Build

TBD