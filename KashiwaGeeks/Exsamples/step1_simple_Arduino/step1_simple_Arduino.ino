                                                                                                                                                                                                                                                                      
void setup(void)
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

int cnt = 0;

void loop(void)
{
  Serial.print("Count=");
  Serial.println(cnt++, DEC);
    
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(1000);                
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

