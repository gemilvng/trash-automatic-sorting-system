// Library for using the arduino framework to program ESP32-CAM
#include <Arduino.h>

// library for wireless communication
#include <WiFi.h>
#include <HTTPClient.h>

// library for the file system with microSD card
#include <FS.h>
#include <SD_MMC.h>

// library for camera configuration
#include "esp_camera.h"
#include "driver/rtc_io.h"

// library to disable brownour problems
#include "soc/soc.h" 
#include "soc/rtc_cntl_reg.h"

// library for emulating EEPROM functionality in ESP32-CAM
#include "EEPROM.h"

/* Network and Wi-Fi related Config
- Include wifi_credentials.h file for Wi-Fi credentials
- Include serverCredentials.h file for server credentials
- Define pin for Wi-Fi connection indicator (INDICATOR_PIN)
- Creating object instance of HTTPClient: clientESP32CAM
- Setting up flag as boolean variable to control HTTP request
    - doHTTPPOSTimage: flag to control HTTP POST request to send image to server
    - Is set to false so the image is not taken before an event occurs
*/
#include "wifi_credentials.h"
#include "serverCredentials.h"

#define INDICATOR_PIN 33

HTTPClient clientESP32CAM;

bool doHTTPPOSTimage = false;

/* Camera config
- Define EEPROM_SIZE to record the number of images taken
- Define GPIO pins for camera configuration
- Initialize pictureCount variable as a unique ID for each image
- Initialize imagePath variable (string) to store the path of each image taken
- Initialize flags for camera configuration
    - initCamera: flag to check camera initialization status
    - captureImage: flag to check image capture status
    - saveImage: flag to check whether image is saved to SD card or not
    - initMicroSD: flag to check SD card initialization status
*/
#define EEPROM_SIZE 1

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

unsigned int pictureCount = 0;

String imagePath = "NULL";

bool initCamera = false;
bool captureImage = false;
bool saveImage = false;
bool initMicroSD = false;

/* taskInitCamera() function
- Initialize camera using esp_camera_init() function
- Implementing error handling with if-else statement
    - Ensure certain settings selected only if device has PSRAM
    - Ensure camera is properly initialized before executing other tasks
- Camera settings for brightness, contrast, etc.
- Set initCamera flag to true if all executed properly
- Further reading: https://dronebotworkshop.com/esp32-cam-microsd/
*/
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
    s->set_vflip(s, 1);
    // DCW (0 = Disable , 1 = Enable)
    s->set_dcw(s, 1);
    // COLOR BAR PATTERN (0 = Disable , 1 = Enable)
    s->set_colorbar(s, 0);
    initCamera = true;
}

/* taskInitMicroSD() function
- Initialize MicroSD card configuration
- Implementing error handling with if-else statement
    - Check if MicroSD card interfacing done properly with SD_MMC.begin() function
    - Check if there is valid no card inserted with cardType() function
- Set initMicroSD flag to true if MicroSD card initialized properly
*/
void taskInitMicroSD() {
    if (!SD_MMC.begin()) {
        initMicroSD = false;
        return;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        initMicroSD = false;
        return;
    }
    initMicroSD = true;
}

/* taskCaptureImage() function
- Capture image from camera using esp_camera_fb_get() function
- Set captureImage flag to true if image captured properly
- Implementing error handling with if-else statement
    - Check if camera failed to capture image by examining fb variable
    - Check if file is not opened by examining file variable
- Creating object instance of File: file, an empty file, with path as reference
- Write image buffer to file with file.write() function
- Release the memory allocated for image buffer with esp_camera_fb_return() function
- Release the memory allocated for file with .close() method
- Set saveImage flag to true if image saved properly
*/
void taskCaptureImage(String path) {
    camera_fb_t * fb = esp_camera_fb_get();

    if (!fb) {
        captureImage = false;
        return;
    }
    captureImage = true;

    fs::FS &fs = SD_MMC;
    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file) {
        saveImage = false;
    } else {
        file.write(fb->buf, fb->len);
    }
    file.close();
    esp_camera_fb_return(fb);
    saveImage = true;
}

/* taskHTTPGETtrigger() function
- Check for trigger to capture image with HTTP GET request
- Trigger is set by button attached to ESP32-S3
- Implementing error handling with if-else statement
    - Check if camera or MicroSD card is not initialized properly
- Start HTTP connection with .begin() method
- Parse HTTP response code with .GET() method
- Get payload from HTTP response with .getString() method
- Handling HTTP response code and payload with if-else statement
    - Check if HTTP response code is 200
    - Check if payload contains "true" string
        - Construct path with string concatenation
        - Call taskCaptureImage() function with path as parameter
        - Increment pictureCount by 1, only if image is captured
    - Check if HTTP response code is 500
- Terminate HTTP connection with .end() method
*/
void taskHTTPGETtrigger() {
    if (initCamera == false || initMicroSD == false) {
        return;
    }

    clientESP32CAM.begin(getStatusURL);
    int httpResponseCode = clientESP32CAM.GET();
    String HTTPpayloadJSON = clientESP32CAM.getString();
    int isPayloadTrue = HTTPpayloadJSON.indexOf("true");

    if (httpResponseCode == 200) {
        for (int i = 0; i < 2; i++) {
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
            digitalWrite(INDICATOR_PIN, LOW); delay(1000);
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
        }

        if (isPayloadTrue != -1) {
            pictureCount = EEPROM.read(0) + 1;
            imagePath = "/picture" + String(pictureCount) + ".jpg";
            taskCaptureImage(imagePath);
            if (captureImage == false || saveImage == false) {
                clientESP32CAM.end();
                return;
            } else {
                EEPROM.write(0, pictureCount);
                EEPROM.commit();
                doHTTPPOSTimage = true;
            }
        } else {
            clientESP32CAM.end();
            return;
        }
    } else if (httpResponseCode == 500) {
        for (int i = 0; i < 5; i++) {
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
            digitalWrite(INDICATOR_PIN, LOW); delay(1000);
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
        }
        clientESP32CAM.end();
        return;
    }
    clientESP32CAM.end();
}

/* taskHTTPPOSTimage() function
- Send image to server with HTTP POST request
- Open the saved image file with .open() method and path as reference
- Read file size with .size() method
- Create image buffer imageBuffer in size of fileSize
- Copy the file content to imageBuffer with .read() method
- Construct HTTP POST request in multipart/form-data format
    - Create components of multipart/form-data header and footer
    - Using .reserve() method to reserve memory for HTTPpayloadJSON
- Start HTTP connection with .begin() method
- Send HTTP POST request with .POST() method
- Parse HTTP response code with .POST() method and blink LED accordingly
- Terminate HTTP connection with .end() method
- Set doHTTPPOSTimage flag to false to ensure task is only executed once
*/
void taskHTTPPOSTimage(String path) {
    fs::FS &fs = SD_MMC;
    File file = fs.open(path.c_str(), FILE_READ);
    if (!file) {
        return;
    }

    size_t fileSize = file.size();
    std::vector<uint8_t> imageBuffer(fileSize);
    file.read(imageBuffer.data(), fileSize);
    file.close();

    /* Create HTTP POST structure with data concatenation.
    The final form of the data being sent is as follows:
    POST /your_backend_services HTTP/1.1
    Host: your_server_ip_or_domain
    Content-Type: multipart/form-data; boundary=ThisIsTheRequestBoundary
    Content-Length: <contentLength>

    --RequestBoundary
    Content-Disposition: form-data; name="file"; filename="picture1.jpeg"
    Content-Type: image/jpeg

    <imageBuffer>
    --ThisIsTheRequestBoundary--
    */   
    
    String boundary = "RequestBoundary";
    String headerRequest = "--" + boundary + "\r\n";
    String footerRequest = "\r\n--" + boundary + "--\r\n";
    headerRequest += "Content-Disposition: form-data; name=\"file\"; filename=\"payload.jpg\"\"\r\n";
    headerRequest += "Content-Type: image/jpeg\r\n\r\n";
    size_t contentLength = headerRequest.length() + fileSize + footerRequest.length();
    std::vector<uint8_t> HTTPpayloadJSON;
    HTTPpayloadJSON.reserve(contentLength);
    HTTPpayloadJSON.insert(HTTPpayloadJSON.end(), headerRequest.begin(), headerRequest.end());
    HTTPpayloadJSON.insert(HTTPpayloadJSON.end(), imageBuffer.begin(), imageBuffer.end());
    HTTPpayloadJSON.insert(HTTPpayloadJSON.end(), footerRequest.begin(), footerRequest.end());
    clientESP32CAM.begin(predictURL);
    clientESP32CAM.addHeader("Content-Type", "multipart/form-data; boundary=RequestBoundary");
    clientESP32CAM.addHeader("Content-Length", String(contentLength));
    int httpResponseCode = clientESP32CAM.POST(HTTPpayloadJSON.data(), HTTPpayloadJSON.size());

    if (httpResponseCode == 201) {
        for (int i = 0; i < 2; i++) {
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
            digitalWrite(INDICATOR_PIN, LOW); delay(1000);
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
        }
    } else if (httpResponseCode == 400) {
        for (int i = 0; i < 4; i++) {
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
            digitalWrite(INDICATOR_PIN, LOW); delay(1000);
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
        }
    } else if (httpResponseCode == 500) {
        for (int i = 0; i < 5; i++) {
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
            digitalWrite(INDICATOR_PIN, LOW); delay(1000);
            digitalWrite(INDICATOR_PIN, HIGH); delay(1000);
        }
    }

    clientESP32CAM.end();
    doHTTPPOSTimage = false;
}

/* setup() function
- Function to initialize the device
- Initialize EEPROM memory with .begin() method in size of EEPROM_SIZE
- Disable brownout detection with WRITE_PERI_REG() function
- Call taskInitCamera() function to initialize camera
- Call taskInitMicroSD() function to initialize MicroSD card
- Configure GPIO pin for Wi-Fi connection indicator
- Implementing error handling with while loop
    - Loop breaks if Wi-Fi is connected
    - Reconnect to Wi-Fi every 3 seconds if Wi-Fi is not connected
*/
void setup() {
    delay(100);

    EEPROM.begin(EEPROM_SIZE);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    taskInitCamera();

    taskInitMicroSD();

    pinMode(INDICATOR_PIN, OUTPUT);

    digitalWrite(INDICATOR_PIN, HIGH);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(3000);
    }
}

/* loop() function
- Function to run the device, repeatedly
- Implementing error handling using if-else statement
    - in case the device is offline, it will reconnect to Wi-Fi network before doing anything else
    - only executing the HTTP request task when the Wi-Fi is connected
- Ensure chained, serial execution of the task by checking the flag value in each if-else statement
*/
void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED, Wi-Fi is connected
        delay(2000); // Delay for each HTTP GET request
        taskHTTPGETtrigger(); // Check for trigger to capture image with HTTP GET request
        if (doHTTPPOSTimage == true) {
            taskHTTPPOSTimage(imagePath); // Send image to cloud server with HTTP POST request
        }
    } else {
        digitalWrite(INDICATOR_PIN, HIGH); // Turn off LED, Wi-Fi is disconnected
        do {
            Serial.println("Reconnecting to Wi-Fi...");
            delay(3000);
        } while (WiFi.status() != WL_CONNECTED);
    }
}