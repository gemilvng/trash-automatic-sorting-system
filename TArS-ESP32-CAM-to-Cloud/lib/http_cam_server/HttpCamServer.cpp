// #include <HttpCamServer.h>

// #ifdef ENABLE_WEBSERVER
// WebServer httpCamServer(80);

// void handle_jpg_stream(void)
// {
//     WiFiClient client = httpCamServer.client();
//     String response = "HTTP/1.1 200 OK\r\n";
//     response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
//     httpCamServer.sendContent(response);

//     while (1)
//     {
//         cam.run();
//         if (!client.connected())
//             break;
//         response = "--frame\r\n";
//         response += "Content-Type: image/jpeg\r\n\r\n";
//         httpCamServer.sendContent(response);

//         client.write((char *)cam.getfb(), cam.getSize());
//         httpCamServer.sendContent("\r\n");
//         if (!client.connected())
//             break;
//     }
// }

// void handle_jpg(void)
// {
//     WiFiClient client = httpCamServer.client();

//     cam.run();
//     if (!client.connected())
//     {
//         return;
//     }
//     String response = "HTTP/1.1 200 OK\r\n";
//     response += "Content-disposition: inline; filename=capture.jpg\r\n";
//     response += "Content-type: image/jpeg\r\n\r\n";
//     httpCamServer.sendContent(response);
//     client.write((char *)cam.getfb(), cam.getSize());
// }

// void handleNotFound()
// {
//     String message = "Server is running!\n\n";
//     message += "URI: ";
//     message += httpCamServer.uri();
//     message += "\nMethod: ";
//     message += (httpCamServer.method() == HTTP_GET) ? "GET" : "POST";
//     message += "\nArguments: ";
//     message += httpCamServer.args();
//     message += "\n";
//     httpCamServer.send(200, "text/plain", message);
// }

// void setupHttpServer()
// {
//     // Set up the ESP32 as an access point
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(ssid_name, ssid_password); // SSID and Password for the AP
    
//     // Wait for connection to Wi-Fi
//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.println("");
//     Serial.println("Connected to Wi-Fi.");
    
//     // Get and print the IP address of the ESP32
//     Serial.print("IP Address: ");
//     Serial.println(WiFi.localIP());

//     // // Get and print the IP address of the access point
//     // IPAddress ip = WiFi.softAPIP();
//     // Serial.println(F("Access Point started"));
//     // Serial.println("");
//     // Serial.println(ip);

//     httpCamServer.on("/", HTTP_GET, handle_jpg_stream);
//     httpCamServer.on("/jpg", HTTP_GET, handle_jpg);
//     httpCamServer.onNotFound(handleNotFound);
//     httpCamServer.begin();

//     Serial.println("HTTP server started");
// }

// void loopHttpServer()
// {
//     httpCamServer.handleClient();
// }

// #endif
