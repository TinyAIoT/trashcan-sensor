#include <I2S.h>
#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <WebServer.h>

#define RECORD_TIME 20  // seconds, The maximum value is 240
#define WAV_FILE_NAME "/arduino_rec.wav"
#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16
#define WAV_HEADER_SIZE 44
#define VOLUME_GAIN 2

const char* ssid = "TinyAIoT";
const char* password = "FaceRecog";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) ;

  I2S.setAllPins(-1, 42, 41, -1, -1);
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    while (1) ;
  }

  // Format SPIFFS before mounting
  SPIFFS.format();

  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS!");
    while (1) ;
  } else {
    Serial.println("SPIFFS mounted successfully!");
  }

  // Check SPIFFS info
  Serial.printf("Total SPIFFS space: %u bytes\n", SPIFFS.totalBytes());
  Serial.printf("Used SPIFFS space: %u bytes\n", SPIFFS.usedBytes());

  record_wav();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.print("ESP32 IP Address for download: http//");
  Serial.print(WiFi.localIP());
  Serial.println("/download");
  server.on("/download", HTTP_GET, []() {
    File file = SPIFFS.open(WAV_FILE_NAME, FILE_READ);
    if (!file) {
      server.send(500, "text/plain", "Failed to open file for reading");
      return;
    }
    server.streamFile(file, "audio/wav");
    file.close();
  });

  server.begin();
}

void loop() {
  server.handleClient();
  delay(1000);
  Serial.printf(".");
}

void record_wav() {
  uint32_t sample_size = 0;
  uint32_t record_size = (SAMPLE_RATE * SAMPLE_BITS / 8) * RECORD_TIME;
  uint8_t *rec_buffer = NULL;
  Serial.printf("Ready to start recording ...\n");

  // Delete the file if it already exists
  if (SPIFFS.exists(WAV_FILE_NAME)) {
    SPIFFS.remove(WAV_FILE_NAME);
  }

  File file = SPIFFS.open(WAV_FILE_NAME, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    Serial.printf("SPIFFS available space: %u bytes\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());
    return;
  }

  uint8_t wav_header[WAV_HEADER_SIZE];
  generate_wav_header(wav_header, record_size, SAMPLE_RATE);
  if (file.write(wav_header, WAV_HEADER_SIZE) != WAV_HEADER_SIZE) {
    Serial.println("Failed to write WAV header");
    file.close();
    return;
  }

  rec_buffer = (uint8_t *)ps_malloc(record_size);
  if (rec_buffer == NULL) {
    Serial.printf("malloc failed!\n");
    file.close();
    return;
  }
  Serial.printf("Buffer: %d bytes\n", ESP.getPsramSize() - ESP.getFreePsram());

  esp_i2s::i2s_read(esp_i2s::I2S_NUM_0, rec_buffer, record_size, &sample_size, portMAX_DELAY);
  if (sample_size == 0) {
    Serial.printf("Record Failed!\n");
    free(rec_buffer);
    file.close();
    return;
  } else {
    Serial.printf("Recorded %d bytes\n", sample_size);
  }

  for (uint32_t i = 0; i < sample_size; i += SAMPLE_BITS / 8) {
    (*(uint16_t *)(rec_buffer + i)) <<= VOLUME_GAIN;
  }

  Serial.printf("Writing to the file ...\n");
  if (file.write(rec_buffer, sample_size) != sample_size) {
    Serial.printf("Write file Failed!\n");
  } else {
    Serial.printf("File written successfully\n");
  }

  free(rec_buffer);
  file.close();
  Serial.printf("The recording is over.\n");
}

void generate_wav_header(uint8_t *wav_header, uint32_t wav_size, uint32_t sample_rate) {
  uint32_t file_size = wav_size + WAV_HEADER_SIZE - 8;
  uint32_t byte_rate = sample_rate * SAMPLE_BITS / 8;

  wav_header[0] = 'R';
  wav_header[1] = 'I';
  wav_header[2] = 'F';
  wav_header[3] = 'F';

  wav_header[4] = file_size & 0xFF;
  wav_header[5] = (file_size >> 8) & 0xFF;
  wav_header[6] = (file_size >> 16) & 0xFF;
  wav_header[7] = (file_size >> 24) & 0xFF;

  wav_header[8] = 'W';
  wav_header[9] = 'A';
  wav_header[10] = 'V';
  wav_header[11] = 'E';

  wav_header[12] = 'f';
  wav_header[13] = 'm';
  wav_header[14] = 't';
  wav_header[15] = ' ';

  wav_header[16] = 0x10;
  wav_header[17] = 0x00;
  wav_header[18] = 0x00;
  wav_header[19] = 0x00;

  wav_header[20] = 0x01;
  wav_header[21] = 0x00;

  wav_header[22] = 0x01;
  wav_header[23] = 0x00;

  wav_header[24] = sample_rate & 0xFF;
  wav_header[25] = (sample_rate >> 8) & 0xFF;
  wav_header[26] = (sample_rate >> 16) & 0xFF;
  wav_header[27] = (sample_rate >> 24) & 0xFF;

  wav_header[28] = byte_rate & 0xFF;
  wav_header[29] = (byte_rate >> 8) & 0xFF;
  wav_header[30] = (byte_rate >> 16) & 0xFF;
  wav_header[31] = (byte_rate >> 24) & 0xFF;

  wav_header[32] = 0x02;
  wav_header[33] = 0x00;

  wav_header[34] = 0x10;
  wav_header[35] = 0x00;

  wav_header[36] = 'd';
  wav_header[37] = 'a';
  wav_header[38] = 't';
  wav_header[39] = 'a';

  wav_header[40] = wav_size & 0xFF;
  wav_header[41] = (wav_size >> 8) & 0xFF;
  wav_header[42] = (wav_size >> 16) & 0xFF;
  wav_header[43] = (wav_size >> 24) & 0xFF;
}
