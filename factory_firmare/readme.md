# How to install the factory demo firmware with flash download tool?

#### 1.Preparation:

- USB C cable
- Laptop/PC
- Crowpanel Advance HMI ESP32 AI Display
- Flash download tool

Check whether the materials on the PCBA board are defective. Such as less material, false welding, tin and other bad phenomena.

#### 2.Flash the firmware 

##### Step1 Connect the ESP32 display to the laptop/PC with USB-C cable. The power indicator will light up.

![flash01](./flash01.jpg)

##### Step2 Open the flash download tool. 

![flash02](./flash02.jpg)



For the  Crowpanel 3.5 HMI ESP32 Display, the chip select ESP32:

![firmware1](https://github.com/user-attachments/assets/b7653b0b-3d26-45d9-9bb0-22c5454fc599)


##### Step3

①Choose the firmware files;

![flash0](./flash0.jpg)

②Enter the address: 

boot_app0.bin------0xe000

xxxx.ino.bin--------0x10000

xxxx.ino.bootloader.bin------0x1000

xxxx.ino.partitions.bin-------0x8000



③Check the firmware files;

④Select the correct port number;

⑤Click “Start” to download

![firmware2](https://github.com/user-attachments/assets/3f02f56c-e7ad-4182-85ec-64298db43ead)


##### Step5 After the download is complete, blue FINISH will be displayed, and the progress bar at the bottom shows that it has come to an end.

![firmware3](https://github.com/user-attachments/assets/4a44970a-cf36-4db8-a019-a5cb31820022)



##### Step6 Press the reset button to show the demo.

![flash8](./flash8.jpg)
