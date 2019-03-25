# ADXL355ArudinoCode
Included in this repository is a selection of code meant to be used with an EVAL-ADXL35X specifically the board for an ADXL355 and ArduinoDue 

In addition to aquiring data from ADXL355 the code includes the ability to sample data from an EMG sensor. The EMG sensor We used was the (INSERT SENSOR HERE).

This code should also work when using an EVAL-ADXL355-PMDZ

This code was based upon code created by _____ that was unsucessful in our implementation. available here--

The ADXL355 Technical documentation can be found here--
ADXL355 chip--https://www.analog.com/media/en/technical-documentation/data-sheets/adxl354_355.pdf
EVAL-ADXL35X--https://www.analog.com/media/en/technical-documentation/user-guides/EVAL-ADXL356-357-UG-1119.pdf


The program utilizes the SPI library of Arduino (a default library included with the arduino program)

The pins utilized on the EVAL-ADXL35X inculde:
  P1 pins 1,3,and 5 for connection to the board 3.3V Power Supply and two(2) double A batteries.
  P2 pins 2,4,5, and 6 are used for establishing connections for SPI communication
  


The Arduino Due uses pin 10 to interface with the Chip Select Pin (P2 pin2) of the ADXL355. This setting can be adjusted to other built in chip selsect digital pins on the Due so long as the code variable ARD_PIN is changed to reflect the new pinout.

