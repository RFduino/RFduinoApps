A compass built on an RFduino. The compass uses an LSM303 to calculate heading and an OLED Display to display the current heading.
The LSM303 and OLED Display are connected to the RFduino via the I2C bus.
The RFduino publishes a Bluetooth low energy Service with the current heading and notifies a client when it changes. The compass accepts
a single command to initiate a calibration sequence for the LSM303.