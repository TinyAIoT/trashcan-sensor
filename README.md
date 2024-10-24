# The "sensor" Repository
This repository contains all files related to the smart sensors that were created for the TinyAIoT project seminar. For an explanation of these sensors, refer to the [wiki](https://github.com/tinyaiot-ps/sensor/wiki) of this repository. The following table explains its top-level directory structure.

| File / Directory | Description |
|-|-|
| `XIAO_microphone_20s_recording` | An example for the XIAO Sense and internal microphone to record and share it on a web server in the microcontroller. |
| `esp32S3-i2s-mic-sample` | An example sketch for the XIAO board and ICS-43434 that plots real time audio to the serial plotter. |
| `esp32S3-i2s-record-data` | A sketch to continuesly record data samples on the micro SD card, e.g., for ML model training. |
| `lorawan_antenna` | An experimental Arduino sketch for testing and learning about LoRa and associated libraries. It uses the [senseBox LoRa Bee](https://docs.sensebox.de/docs/hardware/bee/lora-bee/) which is incompatible with the LoRa board used in the smart trash bins. |
| `noise_detection` | The Arduino sketch for our smart noise detector. Refer to the wiki for details about its structure. |
| `trash_bin` | The Arduino sketch for our smart trash bins. Refer to the wiki for details about its structure. |
| `trash_bin_wifi` | The Arduino sketch for our smart trash bins but with MQTT through wifi for demo purposes. |
| `wiki` | A directory for resources used in the wiki. |
| `README.md` | This file. |
| `ei-duonoise-arduino-1.0.2.zip` | The Noise Detection model library that is needed to run General_EI_dB_Wlan. |
| `Payload Formatter` | The Payload Formatter for decoding messages received from sensor devices. |
| `MQTT Message` | Directory containing example MQTT message with JSON payload. |
| `Commands` | Directory containing example MQTT commands. |

## Licensing
There is no single license that applies to the entire repository. Each subdirectory contains the licenses that are relevant for itself because different licenses can apply to modified third-party files.
