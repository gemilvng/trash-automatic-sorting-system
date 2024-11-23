#include <Arduino.h>

// library for wireless communication
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// library for the file system with microSD card
#include <FS.h>
#include <SD_MMC.h>

// library for camera configuration
#include "esp_camera.h"
#include "driver/rtc_io.h"

//library to disable brownour problems
#include "soc/soc.h" 
#include "soc/rtc_cntl_reg.h"

// library for EEPROM
#include "EEPROM.h"

/* Network Config */
// Include Wi-Fi credentials file, this is not a library
#include "wifi_credentials.h"

// Flag to control how the ESP32-CAM will behave
bool clientModeFlag = false;
bool serverModeFlag = true;

// Define pin for Wi-Fi connection indicator
#define INDICATOR_PIN 33

// Declaring global variable for storing IP address
IPAddress ipCAM; // Global variable for storing camera IP address
String ipS3; // Global variable for storing ESP32-S3 IP address

// Setting up Web Server
WebServer serverESP32CAM(80); // 80 is for HTTP

// Setting up HTTP client
HTTPClient clientESP32CAM;

// String for storing the HTTP payload
String httpPayload;

// String for storing cloud server address
String cloudServerURL = "ml-tars-571232848590.us-central1.run.app";
String servicePath = "/predict";

// Flag for HTTP POST to cloud server
bool postToCloud = false;
/* End of Network Config */

/* Camera configuration and setup */
// EEPROM Configuration. EEPROM value is used for unique image numbering
#define EEPROM_SIZE 1

// Define pin for camera configuration
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Counter for the number of pictures taken
unsigned int pictureCount = 0;

// Defining path variable for the new photo to be saved in microSD card
String imagePath = "NULL";

// Variable to store prediction result
String predictionResult = "NULL";

bool initCamera = false;
bool captureImage = false;
bool saveImage = false;
/* End of camera config*/

/* SD card configuration */
bool initMicroSD = false;
/* End of SD card config */

// Defining task to configure ESP Camera
void taskInitCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        initCamera = false;
        return;
    }
    
    // Camera quality adjustments
    sensor_t * s = esp_camera_sensor_get();
 
    // BRIGHTNESS (-2 to 2)
    s->set_brightness(s, 0);
    // CONTRAST (-2 to 2)
    s->set_contrast(s, 0);
    // SATURATION (-2 to 2)
    s->set_saturation(s, 0);
    // SPECIAL EFFECTS (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_special_effect(s, 0);
    // WHITE BALANCE (0 = Disable , 1 = Enable)
    s->set_whitebal(s, 1);
    // AWB GAIN (0 = Disable , 1 = Enable)
    s->set_awb_gain(s, 1);
    // WB MODES (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_wb_mode(s, 0);
    // EXPOSURE CONTROLS (0 = Disable , 1 = Enable)
    s->set_exposure_ctrl(s, 1);
    // AEC2 (0 = Disable , 1 = Enable)
    s->set_aec2(s, 0);
    // AE LEVELS (-2 to 2)
    s->set_ae_level(s, 0);
    // AEC VALUES (0 to 1200)
    s->set_aec_value(s, 300);
    // GAIN CONTROLS (0 = Disable , 1 = Enable)
    s->set_gain_ctrl(s, 1);
    // AGC GAIN (0 to 30)
    s->set_agc_gain(s, 0);
    // GAIN CEILING (0 to 6)
    s->set_gainceiling(s, (gainceiling_t)0);
    // BPC (0 = Disable , 1 = Enable)
    s->set_bpc(s, 0);
    // WPC (0 = Disable , 1 = Enable)
    s->set_wpc(s, 1);
    // RAW GMA (0 = Disable , 1 = Enable)
    s->set_raw_gma(s, 1);
    // LENC (0 = Disable , 1 = Enable)
    s->set_lenc(s, 1);
    // HORIZ MIRROR (0 = Disable , 1 = Enable)
    s->set_hmirror(s, 0);
    // VERT FLIP (0 = Disable , 1 = Enable)
    s->set_vflip(s, 0);
    // DCW (0 = Disable , 1 = Enable)
    s->set_dcw(s, 1);
    // COLOR BAR PATTERN (0 = Disable , 1 = Enable)
    s->set_colorbar(s, 0);
    Serial.println("Camera Initialized");
    initCamera = true;
}

// Defining task to configure SD Card
void taskInitMicroSD() {
    Serial.println("Mounting MicroSD Card");
    if (!SD_MMC.begin()) {
        Serial.println("Error mounting MicroSD Card");
        initMicroSD = false;
        return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD Card attached");
        initMicroSD = false;
        return;
    }
    Serial.println("MicroSD Card Initialized");
    initMicroSD = true;
}

// Defining Task to take a new photo
void taskCaptureImage(String path) {
    camera_fb_t * fb = esp_camera_fb_get();

    if (!fb) {
        Serial.println("Camera capture failed");
        captureImage = false;
        return;
    }
    Serial.println("Photo captured successfully");
    captureImage = true;

    // Save picture to MicroSD Card
    fs::FS &fs = SD_MMC;
    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file in write mode");
        saveImage = false;
    } else {
        file.write(fb->buf, fb->len);
        Serial.printf("Saved file to path: %s\n", path.c_str());
    }
    file.close();
    esp_camera_fb_return(fb);
    saveImage = true;
}

//Task to make HTTP POST request to cloud server
void taskHTTPPOSTtoCloud(String path) {
    fs::FS &fs = SD_MMC;
    File file = fs.open(path.c_str(), FILE_READ); // Open file system in read mode
    if (!file) { // Read: If file is not opened ...
        Serial.println("Failed to open file in read mode");
        postToCloud = false;
        return;
    }
    // Convert image file to buffer
    size_t fileSize = file.size();
    uint8_t *imageBuffer = new uint8_t[fileSize];
    file.read(imageBuffer, fileSize);
    file.close();

    clientESP32CAM.begin(cloudServerURL + servicePath);
    clientESP32CAM.addHeader("Content-Type", "image/jpeg");
    int httpResponseCode = clientESP32CAM.POST(imageBuffer, fileSize);
    if (httpResponseCode == 200) {
        predictionResult = clientESP32CAM.getString();
    } else {
        Serial.println("HTTP POST request failed");
        postToCloud = false;
    }
    clientESP32CAM.end();
    postToCloud = true;
    delete[] imageBuffer;
}

// Task to make HTTP request to ESP32-S3
void taskHTTPPOSTToS3() {
    if (ipS3.length() == 0) {
        return;
    } else if (ipS3.length() > 0) {
        clientESP32CAM.begin("http://" + ipS3 + "/predictionresult");
        clientESP32CAM.addHeader("Content-Type", "text/plain");
        if (postToCloud == false) {
            httpPayload = "Request Failed.";
            int httpResponseCode = clientESP32CAM.POST(httpPayload);
        } else if (postToCloud == true) {
            int httpResponseCode = clientESP32CAM.POST(predictionResult);
        }
        clientESP32CAM.end();
    }
}

// Handler function for HTTP request to take a new photo
void handleImageCapture() {
    // Check if SD card and camera initialized properly
    if (initCamera == false || initMicroSD == false) {
        httpPayload = "Hardware Failure.";
        serverESP32CAM.send(500, "text/plain", httpPayload);
        Serial.println("Hardware Failure.");
        return;
    }

    /* Take value in EEPROM and increment it, then embed it
    to path string to make the picture name unique */ 
    pictureCount = EEPROM.read(0) + 1;
    imagePath = "/picture" + String(pictureCount) + ".jpg";

    // Calling task to take a new photo
    taskCaptureImage(imagePath);
    if (captureImage == false || saveImage == false) {
        httpPayload = "Capture/Save Failed.";
        serverESP32CAM.send(500, "text/plain", httpPayload);
        Serial.println("Failed to take a new photo");
        return;
    }

    /* Update the value in EEPROM, if image captured*/
    EEPROM.write(0, pictureCount);
    EEPROM.commit();

    httpPayload = imagePath;
    // Send HTTP response to the client
    serverESP32CAM.send(200, "text/plain", httpPayload);
    Serial.println("New photo is taken" + imagePath);
}

// Handler function for HTTP request to handle 404 error
void handleNotFound() {
    httpPayload = "404 Not Found";
    serverESP32CAM.send(404, "text/plain", httpPayload);
    Serial.println("404 Not Found");
}

void setup() {
    delay(100); // Short delay

    Serial.begin(115200); // Initialize serial communication

    /* Camera configuration in setup() */
    EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
    
    // Initialize Camera
    Serial.println("Initializing Camera...");
    taskInitCamera();

    // Initialize MicroSD Card
    Serial.println("Initializing MicroSD Card...");
    taskInitMicroSD();
    /* End of camera configuration setup */

    pinMode(INDICATOR_PIN, OUTPUT); // Set Indicator LED pin as output

    /* Turn off the Indicator LED by set HIGH as parameter.
    This pin behavior is different than the usual. */
    digitalWrite(INDICATOR_PIN, HIGH);

    //WiFi.config(staticIP, gateway, subnet);
    WiFi.begin(ssid, password); // Attempting to connect to Wi-Fi network
    while (WiFi.status() != WL_CONNECTED) {
        delay(3000);
        Serial.println("Connecting to Wi-Fi...");
    }

    /* Blinking Indicator LED three times to indicate
    successfull connection attempt after booting*/
    for (int i = 0; i < 3; i++) {
        digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED
        delay(1000);
        digitalWrite(INDICATOR_PIN, HIGH); // Turn off Indicator LED
        delay(1000);
    }

    ipCAM = WiFi.localIP(); // Store local IP address to global variable

    // Print the IP address of the camera when connection is established
    Serial.println("Connected. camera IP address: " + ipCAM.toString());
    digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED, Wi-Fi is connected

    // Setup the Web Server
    serverESP32CAM.on("/takephoto", HTTP_GET, handleImageCapture);
    serverESP32CAM.onNotFound(handleNotFound);
    serverESP32CAM.begin(); // Start the server
    Serial.println("HTTP server started");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        serverESP32CAM.handleClient(); // Check for incoming connection
    } else {
        do {
            digitalWrite(INDICATOR_PIN, HIGH); // Turn off LED, Wi-Fi is disconnected
            Serial.println("Reconnecting to Wi-Fi...");
            delay(3000);
        } while (WiFi.status() != WL_CONNECTED);
        digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED after reconnection
        Serial.println("Connected. camera IP address: " + ipCAM.toString());
    }
}