#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "Smart-Vibes";
char *password = "smartvibes12";
String uuid = "d64aeb74-f935-4715-9d82-334b069c0616";
String key = "amazingkeyski";
int pin1 = 16 ; // GPIO13


char *ssid2 = "AndroidAP";
char *password2 = "pasaport";

int coupled = 0;  //Boolean
ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */

void readSetValues(){
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);
  File f = SPIFFS.open("/f.txt", "r");  
  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");
    // open the file in write mode
    File f = SPIFFS.open("/f.txt", "w");
    if (!f) {
      Serial.println("file creation failed");
    }
    // now write two lines in key/value style with  end-of-line characters
    f.println("")
    
  }else {
    while(f.available()) {
      String line = f.readStringUntil('n');
      Serial.println(line);
   }
    Serial.println("scan start");

  }
  f.close();
}

void turnVibrationOn(){
  digitalWrite(pin1, HIGH); 
  }

void turnVibrationOff(){
  digitalWrite(pin1, LOW); 
  }
  
void handleOn() {
	server.send(200, "text/json", "{\"status\":\"ON\"}");
  turnVibrationOn();
}
void handleOff() {
  server.send(200, "text/json", "{\"status\":\"OFF\"}");
  turnVibrationOff();
}

 
void connectWifi(){
    
  WiFi.begin(ssid2, password2);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void couple() {
  server.send(200, "text/json", "{\"deviceId\":\""+uuid+"\", \"key\":\""+key+"\"}");
  WiFi.softAPdisconnect(true);
  delay(1000);//15 secs to stop connection!!!!!!!!!!!!!!#@#$
  connectWifi();  
  String payload = "{\"deviceId\":\""+uuid+"\"}";  //Request to server
  post("145.24.222.157", 8080, "/couple/device", payload);
  coupled=1;  //WRITE AND READ FROM FILE
}

void changePassword() {
  server.send(200, "text/json", "change password here.");
  //Change Wifi password, How to get input?
}

String post(char http_site[], int http_port, String endpoint, String payload){
  WiFiClient client;
  if (!client.connect(http_site, http_port) ) {
    return "";
  }
  client.println("POST "+endpoint+" HTTP/1.1");
  client.print("Host:  ");
  client.println(http_site);
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded;");
  client.print("Content-Length: ");
  client.println(payload.length());
  client.println();
  client.println(String(payload));
  String result = "";
  while (client.connected()){
  if (client.available()){
    char a = client.read();
    result = result + a;
    Serial.println(a);
  }      
}
Serial.println("========");
Serial.println(result);
return result;
 }
void pollServer(){
    String payload = "{\"deviceId\":\""+uuid+"\"}";
    String response = post("145.24.222.157", 8080, "/retrieve", payload);
    Serial.println("===PRINTIN");
    Serial.println(response);
    //DECRYPRION

    if(response.indexOf("{\"command\":\"none\"}")>=0){
      Serial.println("Command: None");
      return;
    }
    if(response.indexOf("{\"command\":\"ON\"}")>=0) {
      Serial.println("Command: ON");
      turnVibrationOn();
    }
    if(response.indexOf("{\"command\":\"OFF\"}")>=0) {
      Serial.println("Command: OFF");
      turnVibrationOff();
    }  

}  

//=======================================
void setup() {
  delay(1000);
  Serial.begin(115200);
  pinMode(pin1, OUTPUT);
  turnVibrationOff();

  if (coupled == 0){   //Create AP to access
    Serial.println();
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);
  
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/on", handleOn);
    server.on("/off", handleOff);
    server.on("/couple", couple);
    server.on("/password", changePassword);
    
    server.begin();
    Serial.println("HTTP server started");
  }
  else{ //connect to WiFi and ready to go 
    connectWifi();
    }
}


void loop() {
  if(coupled == 0){
	  server.handleClient();
  }
  if(coupled == 1){
    delay(5000);
    Serial.println("Polling Server");
    pollServer();
    }
}
