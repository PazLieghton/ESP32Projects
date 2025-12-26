int val = 0;

void setup() {
  Serial.begin(9600);
}

// put your main code here, to run repeatedly
void loop() {
  // read hall effect sensor value
  val = hallRead();
  // print the results to the serial monitor
  Serial.println(val); 
  delay(1);
}
//Solo FUNCIONA CON ESP32 BOARD MANAGER VERSION 2.0.14