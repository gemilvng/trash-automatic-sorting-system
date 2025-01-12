# 0. Description
This repository section contains descriptions of embbeded system of TArS, an IoT-based automatic waste management system. It serves as IoT device that communicates with a cloud server to transmit and receive data, such as numerical values and image, over the internet. The system works as follows: The ESP32-CAM captures an image of the waste being deposited, which are then sent to a cloud server hosting a machine learning model. The server processes the image through inference process and store the classification results. Then, ESP32-S3 will send request to retrieve classification result, which will be taken as an input to control the servo motor to sort the waste accordingly. Finally, the ESP32-S3 triggers an ultrasonic sensor to measure the remaining capacity of the waste bin and send the measured data to the cloud server, allowing user to monitor remotely.
# 1. Components and Tools used in this project
Name of Components (Quantity)
1. YD-ESP32-S3 Development Board (1)
2. ESP32-CAM (1)
3. ESP32-CAM-MB (1)
4. HC-SR04 Ultrasonic Sensor (3)
5. MG99R Servo Motor (2)
6. 20x4 LCD with I2C Module (1)
7. Tactile Switch Push Button TC1212T (1)
8. 6V/1200mA AA Battery (8)
9. 4xAA Battery Holder with DC Jack 5.5mm (2)
10. DC Jack Female Power Adapter 5.5x2.1mm (2)
11. Jumper Cable (As much as needed)
12. 5V/2100mA Power Bank (1)
13. Breadboard (As much as needed)
14. Platform IO IDE as extensions in VS Code
# 2. Wiring Table
This summarizes the interface between each device (Wiring diagram will be uploaded in the future).
How to read: Interface of device listed in 1st column <-> Interface of device listed in 1st Row
|Components|ESP32-S3|1st Battery Holder|2nd Battery Holder|ESP32-CAM-MB|
|---|---|---|---|---|
|1st HC-SR04|TRIG <-> PIN4, ECHO <-> PIN5, VCC <-> 3V3, GND <-> GND||||
|2nd HC-SR04|TRIG <-> PIN6, ECHO <-> PIN7, VCC <-> 3V3, GND <-> GND||||
|3rd HC-SR04|TRIG <-> PIN1, ECHO <-> PIN2, VCC <-> 3V3, GND <-> GND||||
|1st MG996R|PWM <-> PIN8, GND <-> GND|VCC <-> VCC, GND <-> GND|||
|2nd MG996R|PWM <-> PIN21, GND <-> GND||VCC <-> VCC, GND <-> GND||
|LCD 20x4 I2C Module|SDA <-> PIN10, SCL <-> PIN9, VCC <-> 3V3, GND <-> GND||||
|Power Bank|USB <-> USB|||USB <-> USB|
|TC1212T|1st Pin Group <-> PIN47, 2nd Pin Group <-> GND||||
# 3. Installing Platform IO
All steps needed to install and use Platform IO is in [here](https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode) (You might need to install VS Code as well).
# 4. Clone the Repository (in Windows)
Navigate to any folder you wish and create a new folder, let's say `test-clone-repo`, preferably under `C:\Users\YourComputerName\`. Under `test-clone-repo`, open Powershell terminal and run this following command (make sure [Git](https://git-scm.com/) is installed).

```
git clone https://github.com/gemilvng/trash-automatic-sorting-system.git
```
# 5. Upload the program
To upload the program to ESP32-S3:
1. open Powershell terminal under `test-clone-repo\TArS-IoT-System`. Then, run this command to open VS Code: 
```
code .
```
2. Wait until VS Code and Platform IO setup finished (until no pop-ups left).
3. Locate the `build` button, click and wait for the process to be finished.
4. Locate the menu to select the USB port connected to your ESP32-S3.
5. Click `upload` button next to `build` button.
6. After the upload process is finished, you can switch the power source of ESP32-S3 to Power Bank instead of Laptop/PC.

Repeat the exact same steps if you wish to upload the code to ESP32-CAM, but navigate to `test-clone-repo\TArS-ESP32-CAM` instead.
