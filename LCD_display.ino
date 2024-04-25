#include <Wire.h>
//Include the Liquid crystal library for I2C
#include <LiquidCrystal_I2C.h> 
#include <SoftwareSerial.h>
//Define the transmission ports
#define RX 10
#define TX 11
//Network setup credentials
String SSID = "gomba";       // Wifi SSID
String PASSWORD = "testwifi04"; // WiFi password
String API = "1JOXVRX8UBODFK0Z";   // API key
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int bgVal = 0;
SoftwareSerial esp8266(RX,TX);
 
//Define I2C address
const int i2c_addr = 0x27;
int max_ir_read, new_ir_read;
float voltage, bg_value;
LiquidCrystal_I2C lcd(i2c_addr, 16,2);

//Array for storing voltage values
float voltage_array[5];
float average=0.0;
float sum=0.0;

void setup(){
  //Settting up things....
  //Initialize LCD display
  lcd.init();
  Serial.begin(9400);
  esp8266.begin(115200);
  pinMode(A0, INPUT);
  lcd.backlight();
  lcd.setCursor(0,0);
  //Print on the first row
  lcd.print("Hello There!");
  
  //Wait for 1 second
  delay(1000);

  //Print the second row
  lcd.setCursor(0, 1);
  lcd.print("Connecting wifi..");
  //Connect to the wifi
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ SSID +"\",\""+ PASSWORD +"\"",20,"OK");
  //
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place ur finger!");
  
}
void loop(){
  
  max_ir_read = analogRead(A0);
  delay(100);
  new_ir_read = analogRead(A0);
  if(new_ir_read>max_ir_read){
    max_ir_read = new_ir_read;
    voltage = max_ir_read * (5.0 / 1023.0);
    if(max_ir_read>=85){
      // bg_value = (287.40795*voltage)-47.48058;
      //  store voltage values in voltage array
      for (int i = 0; i < 5;i++) {
        voltage_array[i] = voltage;
      }
      //If all the values in the array are updated
      if(voltage_array[5]>0){
        for (const float &v : voltage_array) {
          //  calculate the sum of voltages
          sum += v;
        }
        average = sum/5;
        //Change all the values back to 0
        for (int i = 0; i < 5;i++) {
          voltage_array[i] = 0;
        }
      }
      if(average>0.0){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BG val");
        lcd.setCursor(0, 1);
        bg_value = (287.4079529*average)-47.4808542;
        
        lcd.print(bg_value);
        lcd.print(" mg/dL");
        delay(1000);
        sendBG(bg_value);
        Serial.println(bg_value);
        delay(2000);
      }
      sum=0.0;
      average = 0.0;
    }
    Serial.println(max_ir_read);
  }
}
//Responsible for sending data to thingspeak cloud
void sendBG(float data){

  String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(data);
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);
  delay(1500);
  // Wait for a response from the server
  unsigned long startTime = millis();
  while (millis() - startTime < 15000) {
    if (esp8266.find("SEND OK")) {
      break;
    }
  }
  // Check the connection status before closing
  esp8266.println("AT+CIPSTATUS");
  delay(100); // Give some time for the response to arrive
  countTrueCommand++;
  // Close the TCP connection
  sendCommand("AT+CIPCLOSE=0",5,"OK");
}
//Responsible for performing AT commands
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". AT command => ");
  Serial.print(command);
  Serial.print("-> ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }