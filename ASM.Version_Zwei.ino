         //-----Andrew Spiewak ADXL355 Accelerometer 
                       //-----2/13/19
                       
#include <SPI.h>  //------------------------------------ library for SPI in arduino must be added to run SPI protocal.
// telling the arduino where to read/write, after assignment there is no options needed.
const int WRITE_TO = 0x00; //-----------------------------This is the address we will give data to.
const int READ_FROM = 0x01; //----------------------------This is the address we will get data from.
// There are 3 addresses per axis that data is stored in 8+8+4 bits. We will need to tell it to not read 4-7 on all of the _1 locations
const int Z_1 = 0x10; //----------------------------------Bits 3,2,1,0 only will store Z data.
const int Z_2 = 0x0F; //----------------------------------We use all 8 bits at this address.
const int Z_3 = 0x0E; //----------------------------------We use all 8 bits at this address.
const int Y_1 = 0x0D; //----------------------------------Bits 3,2,1,0 only will store Y data.
const int Y_2 = 0x0C; //----------------------------------We use all 8 bits at this address.
const int Y_3 = 0x0B; //----------------------------------We use all 8 bits at this address.
const int X_1 = 0x0A; //----------------------------------Bits 3,2,1,0 only will store x data.
const int X_2 = 0x09; //----------------------------------We use all 8 bits at this location.
const int X_3 = 0x08; //----------------------------------We use all 8 bits at this location.
// We want to have the filter active
const int FILTER = 0x28; //----------------------------This is the filter address.
const int SET_FILTER = 0x6A; //------------------------------This creates a bandpass (Table 43, pg. 37)
// We want the Range to measure 2G and below.
const int RANGE = 0x2C; //--------------------------------This is the address to set the range setting.
const int MEAS_2G = 0x01; //------------------------------This is the code to set the range to 2G.
// We need to turn the device off/on, and set it to measure only acceleration.
const int POWER = 0x2D; //---------------------------------This is the address for the power setting.
const int PW_SET = 0x06; //---------------------------------First digit DRDY (1=off, 0=on), second digit TEMP (1=off, 0=on), third digit STANDBY (1=standby, 0=measure)
// We need to select a digital pin on the arduino to run the slave.
const int ARD_PIN = 10; //---------------------------------Specifies pin 10 as chip select pin.
const int EMGPIN = A1;
//Naming the reset functions
const int RESET = 0x2F; //---------------------------------This is the location to reset the device
const int NEW = 0x00; //-----------------------------------This is the code to sent to reset device
const int EXPANDSIGNBASE = 0xFFF;
int EXPANDSIGNSHIFTED = EXPANDSIGNBASE << 20;
int CalibrationCounter = 0;
int CalibrationCheck =0;
int microa;
int microb;
int rate;
int CalibrationSamples = 501;
int CalibrationCalculationCount = 0;
float CalculationStoragea = 0;
float CalculationStorageb = 0;
float CalculationStoragec = 0;
float XCalibrationValue=0;
float YCalibrationValue=0;
float ZCalibrationValue=0;





// Now that our constants are set we can write the loop
void setup() {
 Serial.begin(74880); //------------------------------------This sets the number of bits per second. (sampling frequency x 20 bits). BAUD
 SPI.begin(); //--------------------------------------------This enables the Arduino SPI pin locations as follows... clock CLK is pin 13...MISO is pin 12...MOSI is pin 11.
 pinMode(ARD_PIN,OUTPUT); //--------------------------------Tells us pin 10 should be an output
// Reset the device to make sure it starts clean
writeRegister(RESET,NEW); //--------------------------------This resets all setting to default each time its plugged in.
delay(250);//-----------------------------------------------The reset takes time to finish
// Now we will configure the settings we named earlier
writeRegister(RANGE,MEAS_2G); //----------------------------This activates the range of 2G measurements
writeRegister(POWER,PW_SET); //-----------------------------This enables measurement and dissables TEMP and DRDY
//writeRegister(FILTER,SET_FILTER); //------------------------This enables filters

delay(100); //----------------------------------------------more initial setup time
}
void loop() {
// Begin our loop by organizing the data collected
int DataAddresses[] = {X_3,X_2,X_1,Y_3,Y_2,Y_1,Z_3,Z_2,Z_1}; //-------------------Creates a vector containing all the X-axis data addresses.
int DataMeasures[] = {0,0,0,0,0,0,0,0,0};
int dataSize = 9;
int XaxisAddresses[] = {X_3,X_2,X_1};
int XaxisMeasures[] = {0, 0, 0};//--------------------------------Creates a vector containing all the X-axis data points.
int YaxisAddresses[] = {Y_3,Y_2,Y_1}; //-------------------Creates a vector containing all the Y-axis data addresses.
int YaxisMeasures[] = {0, 0, 0};//--------------------------------Creates a vector containing all the Y-axis data points.
int ZaxisAddresses[] = {Z_3,Z_2,Z_1}; //-------------------Creates a vector containing all the Z-axis data addresses.
int ZaxisMeasures[] = {0, 0, 0};//--------------------------------Creates a vector containing all the Z-axis data points.
int axisdataSize = 3; //---------------------------------------------------------------State the size of the vectors
int XCalibrationStorage[CalibrationSamples];
int YCalibrationStorage[CalibrationSamples];
int ZCalibrationStorage[CalibrationSamples];

// Read the data from the accelerometer and EMG
readMultipleData(DataAddresses, dataSize,DataMeasures);
int XDATA = (DataMeasures[2] >> 4) + (DataMeasures[1] << 4) + (DataMeasures[0] << 12);
int YDATA = (DataMeasures[5] >> 4) + (DataMeasures[4] << 4) + (DataMeasures[3] << 12);
int ZDATA = (DataMeasures[8] >> 4) + (DataMeasures[7] << 4) + (DataMeasures[6] << 12);
int XTestValue = XDATA << 12;
int YTestValue = YDATA << 12;
int ZTestValue = ZDATA << 12;
//Correct the sign of the data
if (XTestValue < 0){
  XDATA = XDATA+EXPANDSIGNSHIFTED;
}
if (YTestValue < 0){
  YDATA = YDATA+EXPANDSIGNSHIFTED;
}
if (ZTestValue < 0){
  ZDATA = ZDATA+EXPANDSIGNSHIFTED;
}
//get EMG Data
analogReadResolution(12);
int EMGDATA = ((analogRead(EMGPIN)*500)-800000);



//get data capture rate
microa = micros();
rate = 1000000/(microa-microb);
microb = microa;

  if (CalibrationCheck == 0 ){
   // Serial.println("Accelerometer is gathering initial data for calibration");
    XCalibrationStorage[CalibrationCounter]=XDATA;
    YCalibrationStorage[CalibrationCounter]=YDATA;
    ZCalibrationStorage[CalibrationCounter]=ZDATA;
    if (++CalibrationCounter == CalibrationSamples){
      CalibrationCheck=1;
    }}else if (CalibrationCheck == 1){
    //Serial.println("Calibration has progressed to calculation stage");
    for (int CalibrationCalculationCount=0; CalibrationCalculationCount < CalibrationSamples-10; CalibrationCalculationCount++){
      CalculationStoragea = ((XCalibrationStorage[CalibrationCalculationCount])/(CalibrationSamples-10));
      //Serial.print(CalibrationCalculationCount);
      //Serial.print("\t");
      //Serial.print(CalculationStoragea);
      //Serial.print("\t");
      CalculationStorageb = ((YCalibrationStorage[CalibrationCalculationCount])/(CalibrationSamples-10));
      CalculationStoragec = ((ZCalibrationStorage[CalibrationCalculationCount])/(CalibrationSamples-10));
      XCalibrationValue += CalculationStoragea;
      //Serial.println(XCalibrationValue);
      YCalibrationValue += CalculationStorageb;
      ZCalibrationValue += CalculationStoragec;
    }
    Serial.print("The X-axis will be shifted by ");
    Serial.println(XCalibrationValue);
    Serial.print("The Y-axis will be shifted by ");
    Serial.println(YCalibrationValue);
    Serial.print("The Z-axis will be shifted by ");
    Serial.println(ZCalibrationValue);
    CalibrationCheck=2;
  } /*else if (CalibrationCheck == 2){
      Serial.println("Calibration has progressed to programming the device with obtained data");
    Xshifta=;
    Xshiftb=XCalibrationValue>>8;
    Yshifta=;
    Yshiftb=YCalibrationValue>>8;
    Zshifta=;
    Zshiftb=ZCalibrationValue>>8;
    WriteRegister(Xshifta,Xoffset1);
    WriteRegister(Xshifta,Xoffset2);
    WriteRegister(Yshifta,Yoffset1);
    WriteRegister(Yshifta,Yoffset2);
    WriteRegister(Zshifta,Zoffset1);
    WriteRegister(Zshifta,Zoffset2);
    delay(100);
  }*/else{
    //print data to be read
    //Serial.print("XDATA \t");
   Serial.print(XDATA);
    Serial.print("\t");
    //Serial.print("YDATA \t");
    Serial.print(YDATA);
    Serial.print("\t");
    //Serial.print("ZDATA \t");
    Serial.print(ZDATA);
    Serial.print("\t");
    //Serial.print("Rate \t");
    
    //Serial.print(rate);
    //Serial.print("\t EMGDATA \t");
    Serial.print(EMGDATA);
    Serial.print("\n");
    /*XDATA-=XCalibrationValue;
    YDATA-=YCalibrationValue;
    ZDATA-=ZCalibrationValue;
    
    Serial.print("\t SHIFTED XDATA \t");
    Serial.print(XDATA);
    Serial.print("\t SHIFTED YDATA \t");
    Serial.print(YDATA);
    Serial.print("\t SHIFTED ZDATA \t");
    Serial.print(ZDATA);
    */
  }

/*readMultipleData(XaxisAddresses, axisdataSize, XaxisMeasures); //------------------------How to read the data
// Merging split Data into one 
  int XDATA = (XaxisMeasures[2] >> 4) + (XaxisMeasures[1] << 4) + (XaxisMeasures[0] << 12); // shifts first entry right 4 bits, second entry left 4 bits, third entry left 12 bits 
int XTestValue = XDATA << 12;
if (XTestValue < 0){
  XDATA = XDATA+EXPANDSIGNSHIFTED;
}
//Serial.print("X=");//-------------------------------------COMMENTED OUT SO THAT THE PLOTTER WILL WORK, ALSO REMOVES LABELS IN SERIAL MONITOR         
Serial.print(XDATA);// -------------------------------------COMMENT OUT TO NOT GRAPH X        
Serial.print("\t");
//delay(.25);



readMultipleData(YaxisAddresses, axisdataSize, YaxisMeasures); //------------------------How to read the data
// Merging split Data into one 
  int YDATA = (YaxisMeasures[2] >> 4) + (YaxisMeasures[1] << 4) + (YaxisMeasures[0] << 12); // shifts first entry right 4 bits, second entry left 4 bits, third entry left 12 bits 
int YTestValue = YDATA << 12;
if (YTestValue < 0){
  YDATA = YDATA+EXPANDSIGNSHIFTED;
}
//Serial.print("Y=");//-------------------------------------COMMENTED OUT SO THAT THE PLOTTER WILL WORK, ALSO REMOVES LABELS IN SERIAL MONITOR         
Serial.print(YDATA);// -------------------------------------COMMENT OUT TO NOT GRAPH X        
Serial.print("\t");
//delay(.25);



readMultipleData(ZaxisAddresses, axisdataSize, ZaxisMeasures); //------------------------How to read the data
// Merging split Data into one 
  int ZDATA = (ZaxisMeasures[2] >> 4) + (ZaxisMeasures[1] << 4) + (ZaxisMeasures[0] << 12); // shifts first entry right 4 bits, second entry left 4 bits, third entry left 12 bits 
int ZTestValue = ZDATA << 12;
if (ZTestValue < 0){
  ZDATA = ZDATA+EXPANDSIGNSHIFTED;
}
//Serial.print("Z=");//-------------------------------------COMMENTED OUT SO THAT THE PLOTTER WILL WORK, ALSO REMOVES LABELS IN SERIAL MONITOR         
Serial.print(ZDATA);// -------------------------------------COMMENT OUT TO NOT GRAPH X        
Serial.print("\n");
//delay(.25);
*/



// Set this delay for the loop rate
//delay (1);// -----------------------------------------------milisecond delay to set data sampling rate
}
// This Changes the Data stored on the Accelerometer by a specific value
void writeRegister(byte thisRegister, byte thisValue) {
  byte dataToSend = (thisRegister << 1) | WRITE_TO;
  digitalWrite(ARD_PIN, LOW);
  SPI.transfer(dataToSend);
  SPI.transfer(thisValue);
  digitalWrite(ARD_PIN, HIGH);
}
// Read registry in specific device address
unsigned int readRegistry(byte thisRegister) {
  unsigned int result = 0;
  byte dataToSend = (thisRegister << 1) | READ_FROM;
  digitalWrite(ARD_PIN, LOW);
  SPI.transfer(dataToSend);
  result = SPI.transfer(0x00);
  digitalWrite(ARD_PIN, HIGH);
  return result;
}
// Read multiple registries
void readMultipleData(int *addresses, int dataSize, int *readedData) {
  digitalWrite(ARD_PIN, LOW);
  for(int i = 0; i < dataSize; i = i + 1) {
    byte dataToSend = (addresses[i] << 1) | READ_FROM;
    SPI.transfer(dataToSend);
    readedData[i] = SPI.transfer(0x00);
    //delay(2.5);
  }
  digitalWrite(ARD_PIN, HIGH);
}
