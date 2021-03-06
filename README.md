# AltitudeHeadingReminder

Free "EasyEDA" software is needed to view and modify the schematic and PCB design.

Free "openScade" software is needed to view the 3D model of the case

Arduino libraries are modifications of libraries freely found on the web. Those libraries need to be loaded into Arduino in order to compile. The correct type of Arduino and COM port needs to be configured in the Arduino configuration menus before code can be loaded. Because this project runs on a custom PCB board, you need to use the 4 pins on the board for loading a bootloader that has settings to use the onboard 8MHz clock, since there is no external resonator for the clock. Once the bootloader is loaded, you can use the 6-pin FTDI to upload the software using an FTDI adapter.
