#ifdef __linux__

//#include "../../config.h"
#include "config.h"
#include "ntripclient.h"
#include "LinuxSerial.h"

LinuxSerial SerialNTRIP("/dev/ttyUSB0");


void NTRIPClient::begin(){
  reconnectTimeout = 0;
  SerialNTRIP.begin(115200);
}

void NTRIPClient::connectNTRIP(){
  /*CONSOLE.println("Requesting SourceTable.");
  if(reqSrcTbl(NTRIP_HOST,NTRIP_PORT)){
    char buffer[512];
    delay(100);
    while(available()){
      readLine(buffer,sizeof(buffer));
      CONSOLE.print(buffer); 
    }
  }
  else{
    CONSOLE.println("SourceTable request error");
  }
  CONSOLE.print("Requesting SourceTable is OK\n");
  stop(); //Need to call "stop" function for next request.  
  */
  CONSOLE.println("Requesting MountPoint's Raw data");  
  if(!reqRaw(NTRIP_HOST,NTRIP_PORT,NTRIP_MOUNT,NTRIP_USER,NTRIP_PASS)){
    CONSOLE.println("Error requesting MointPoint");
  } else {
    CONSOLE.println("Requesting MountPoint is OK");
  }
}


void NTRIPClient::run(){
  int count = 0;
  if (millis() > reconnectTimeout){
    if (connected()) stop();          
    reconnectTimeout = millis() + 10000;
    CONSOLE.println("NTRIP disconnected - reconnecting...");
    connectNTRIP();        
  }
  if (!connected()) return;
  // transfer NTRIP client data to GPS...
  while(available()) {
    char ch = read();  
    SerialNTRIP.write(ch);  // send to GPS receiver (NTRIP port)
    count++;            
    //CONSOLE.print(ch);            
  }
  if (count > 0){
    CONSOLE.print("NTRIP:");
    CONSOLE.println(count);
    reconnectTimeout = millis() + 10000;    
  }
  // transfer GPS NMEA data (GGA message) to NTRIP client... 
  String nmea = "";
  while (SerialNTRIP.available()){
    char ch = SerialNTRIP.read();
    write(ch);             // send to NTRIP client
    nmea += ch;        
  }
  if (nmea != ""){
    CONSOLE.print(nmea);        
  }  
}


bool NTRIPClient::reqSrcTbl(char* host,int port)
{
  if(!connect(host,port)){
      CONSOLE.print("Cannot connect to ");
      CONSOLE.println(host);
      return false;
  }/*p = String("GET ") + String("/") + String(" HTTP/1.0\r\n");
  p = p + String("User-Agent: NTRIP Enbeded\r\n");*/
  print(
      "GET / HTTP/1.0\r\n"
      "User-Agent: NTRIPClient for Arduino v1.0\r\n"
      );
  unsigned long timeout = millis();
  while (available() == 0) {
     if (millis() - timeout > 5000) {
        CONSOLE.println("Client Timeout !");
        stop();
        return false;
     }
     delay(10);
  }
  char buffer[50];
  readLine(buffer,sizeof(buffer));
  if(strncmp((char*)buffer,"SOURCETABLE 200 OK",17))
  {
    CONSOLE.print((char*)buffer);
    return false;
  }
  return true;
    
}
bool NTRIPClient::reqRaw(char* host,int port,char* mntpnt,char* user,char* psw)
{
    if(!connect(host,port))return false;
    String p="GET /";
    String auth="";
    CONSOLE.println("Request NTRIP");
    
    p = p + mntpnt + String(" HTTP/1.0\r\n"
        "User-Agent: NTRIPClient for Arduino v1.0\r\n"
    );
    
    if (strlen(user)==0) {
        p = p + String(
            "Accept: */*\r\n"
            "Connection: close\r\n"
            );
    }
    else {
        auth = base64::encode(String(user) + String(":") + psw);
        #ifdef Debug
        CONSOLE.println(String(user) + String(":") + psw);
        #endif

        p = p + String("Authorization: Basic ");
        p = p + auth;
        p = p + String("\r\n");
    }
    p = p + String("\r\n");
    print(p);
    #ifdef Debug
    CONSOLE.println(p);
    #endif
    unsigned long timeout = millis();
    while (available() == 0) {
        if (millis() - timeout > 20000) {
            CONSOLE.println("Client Timeout !");
            return false;
        }
        delay(10);
    }
    char buffer[50];
    readLine(buffer,sizeof(buffer));
    if(strncmp((char*)buffer,"ICY 200 OK",10))
    {
      CONSOLE.print((char*)buffer);
      return false;
    }
    return true;
}
bool NTRIPClient::reqRaw(char* host,int port,char* mntpnt)
{
    return reqRaw(host,port,mntpnt,"","");
}
int NTRIPClient::readLine(char* _buffer,int size)
{
  int len = 0;
  while(available()) {
    _buffer[len] = read();
    len++;
    if(_buffer[len-1] == '\n' || len >= size) break;
  }
  _buffer[len]='\0';

  return len;
}


#endif