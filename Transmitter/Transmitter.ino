void setup() {
  Serial.begin(9600);
  
  pinMode(4, INPUT); // First IR sensor
  pinMode(5, INPUT); // Second IR sensor
  pinMode(6, INPUT); // Third IR sensor
  pinMode(7, INPUT); // Fourth IR sensor
  pinMode(8, OUTPUT); // Digital 0 pin of ht12e
  pinMode(9, OUTPUT); // Digital 1 pin of ht12e
  pinMode(10, OUTPUT); // Digital 2 pin of ht12e
  pinMode(11, OUTPUT); // Digital 3 pin of ht12e
  pinMode(12, OUTPUT); // TE pin of ht12e
}

void loop() {
  // Read data from IR sensors
  int sensor1 = digitalRead(4);
  int sensor2 = digitalRead(5);
  int sensor3 = digitalRead(6);
  int sensor4 = digitalRead(7);
  
  // Send data to ht12e
  digitalWrite(8, sensor1);
  digitalWrite(9, sensor2);
  digitalWrite(10, sensor3);
  digitalWrite(11, sensor4);
  
  // Trigger data transmission
  digitalWrite(12, HIGH);
  delay(10);
  digitalWrite(12, LOW);
}
