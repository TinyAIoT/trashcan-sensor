/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <DuoNoise_inferencing.h>
#include "driver/i2s.h"
#include <HTTPClient.h>
#include <WiFi.h>

#define I2S_SCK SCL
#define I2S_WS D6
#define I2S_SD SDA

/** Set these variables depending on the deployment */
const char* ssid = "WiFi-Network-Name";         // WiFi that the device should connect to
const char* password = "WiFi-Network-Password"; // WiFi Password

const char* apiUrl = "https://backend.com/update";        // POST address of the TinyAIoT Backend
const char* authBearer = "Bearer Long-And-Secure-Secret-Key"; // Secure API Token

const char* projectId = "Project-Id"; // Project of the device
const char* sensorId = "Sensor-Id";   // Identification of one specific device

/** Audio buffers, pointers and selectors */
typedef struct {
    int16_t *buffer;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048; // Size of the buffer for audio samples
static signed short sampleBuffer[sample_buffer_size]; // Buffer for raw audio data
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static bool record_status = true; // Flag to control recording status

/**
 * @brief Arduino setup function
 */
void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Noise prediction and dBFS level over Wlan");

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf(" ms.\n");
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    ei_printf("\nStarting continuous inference in 2 seconds...\n");
    ei_sleep(2000);

    // Start microphone inference
    if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;
    }

    ei_printf("Recording...\n");
}

/**
 * @brief Arduino main function
 */
void loop() {
    // Record audio data
    bool m = microphone_inference_record();
    if (!m) {
        ei_printf("ERR: Failed to record audio...\n");
        return;
    }

    // Initialize signal object for classifier input
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    int pred_index = 0;
    float pred_value = 0;

    // Print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: ", result.classification[ix].label);
        ei_printf_float(result.classification[ix].value);
        ei_printf("\n");

        if (result.classification[ix].value > pred_value) {
           pred_index = ix;
           pred_value = result.classification[ix].value;
        }
    }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: ");
    ei_printf_float(result.anomaly);
    ei_printf("\n");
#endif

    // Calculate and print the unweighted sound level (dBFS)
    float rms = 0; // Root Mean Square: the average power of the sound signal
    for (uint32_t i = 0; i < inference.n_samples; i++) {
        rms += (inference.buffer[i] * inference.buffer[i]);
    }
    rms = sqrt(rms / inference.n_samples);
    float dbfs = 20 * log10(rms / 32768.0);
    Serial.print("Unweighted Sound Level (dBFS): ");
    Serial.println(dbfs);

    // Send dBFS and ML prediction over WiFi 
    sendData(dbfs, result.classification[1].value * 100);
}

/**
 * @brief Send data over Wi-Fi
 */
void sendData(float dbfs, float class1_percentage) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiUrl);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", authBearer);

        // Format as {"projectId": "6681292999d92893669ea287", 
        //            "sensorId": "668945bfbc487cfc392c0068", 
        //            "prediction": 0.4, // Example
        //            "value": -10 // Example} 
        String json = "{\"projectId\": \"" + String(projectId) + "\", \"sensorId\": \"" + String(sensorId) + "\", \"prediction\": " + String(class1_percentage) + ", \"value\": " + String(dbfs) + "}";

        int httpResponseCode = http.POST(json);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("Error in WiFi connection");
    }
}

/**
 * @brief Audio inference callback function
 */
static void audio_inference_callback(uint32_t n_bytes) {
    for (int i = 0; i < n_bytes >> 1; i++) {
        inference.buffer[inference.buf_count++] = sampleBuffer[i];

        if (inference.buf_count >= inference.n_samples) {
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}

/**
 * @brief Capture samples from the I2S interface
 */
static void capture_samples(void* arg) {
    const int32_t i2s_bytes_to_read = (uint32_t)arg;
    size_t bytes_read = i2s_bytes_to_read;

    while (record_status) {
        i2s_read((i2s_port_t)1, (void*)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);

        if (bytes_read <= 0) {
            ei_printf("Error in I2S read : %d", bytes_read);
        } else {
            if (bytes_read < i2s_bytes_to_read) {
                ei_printf("Partial I2S read");
            }

            for (int x = 0; x < i2s_bytes_to_read / 2; x++) {
                sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
            }

            if (record_status) {
                audio_inference_callback(i2s_bytes_to_read);
            } else {
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief Init inferencing struct and setup/start PDM
 */
static bool microphone_inference_start(uint32_t n_samples) {
    inference.buffer = (int16_t *)ps_malloc(n_samples * sizeof(int16_t));

    if (inference.buffer == NULL) {
        return false;
    }

    inference.buf_count  = 0;
    inference.n_samples  = n_samples;
    inference.buf_ready  = 0;

    if (i2s_init(EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start I2S!");
    }

    ei_sleep(100);

    record_status = true;

    xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void*)sample_buffer_size, 10, NULL);

    return true;
}

/**
 * @brief Wait on new data
 */
static bool microphone_inference_record(void) {
    bool ret = true;

    while (inference.buf_ready == 0) {
        delay(10);
    }

    inference.buf_ready = 0;
    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);
    return 0;
}

/**
 * @brief Stop PDM and release buffers
 */
static void microphone_inference_end(void) {
    free(sampleBuffer);
    ei_free(inference.buffer);
}

/**
 * @brief Initialize I2S interface
 */
static int i2s_init(uint32_t sampling_rate) {
    i2s_config_t i2s_config = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate          = sampling_rate,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 8,
        .dma_buf_len          = 512,
        .use_apll             = false,
        .tx_desc_auto_clear   = false,
        .fixed_mclk           = -1
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD
    };

    esp_err_t ret = 0;

    ret = i2s_driver_install((i2s_port_t)1, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ei_printf("Error in i2s_driver_install");
    }

    ret = i2s_set_pin((i2s_port_t)1, &pin_config);
    if (ret != ESP_OK) {
        ei_printf("Error in i2s_set_pin");
    }

    ret = i2s_zero_dma_buffer((i2s_port_t)1);
    if (ret != ESP_OK) {
        ei_printf("Error in initializing dma buffer with 0");
    }

    return int(ret);
}

/**
 * @brief Deinitialize I2S interface
 */
static int i2s_deinit(void) {
    i2s_driver_uninstall((i2s_port_t)1);
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
