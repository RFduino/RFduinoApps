An example Windows Store application which utilizes the Windows 8.1 Bluetooth (r) 
low energy API to interface with an RFduino compass. The application contains a simple
UI to visualize the current heading from the compass. The RFduino compass sends regular
notifications of which direction it is currently pointing and this application displays
that heading. There is a button which sends a calibration command to the compass which will
calibrate the compass when the user slowly spins the compass for the duration of the calibration.