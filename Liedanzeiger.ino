#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <WiFiServer.h> 
#include <DNSServer.h>
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "Cantate";
const char *password = "YourPasswordHere";

ESP8266WebServer server(80);
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

#define ACTION_IDLE 0
#define ACTION_SHOW 1
#define ACTION_HIDE 2

unsigned long refMillis = millis();
int displayTime = 60;
int action = ACTION_IDLE;
char number[] = {0,0,0,0,0};
bool shown = false;


/* Go to http://192.168.1.1 in a web browser connected to this access point. */
void handleRoot() {
    server.send(200, "text/html", F("<!DOCTYPE HTML>\n\
<html>\n\
  <head>\n\
    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>\n\
<script type='text/javascript'>\n\
var nr='', st='', anr=''; ast='', adur=10; ael=-1, cur=0; \n\
function ag(url,rh) { var xh = new XMLHttpRequest(); xh.open('GET', url, true); xh.onreadystatechange = function () { if (xh.readyState == 4) { rh(xh.responseText); }}; xh.send(null); } \n\
function show(nr,str) { ag('show?n=' + nr + '&s=' + str, status);} \n\
function hide() {ag('hide', status); } \n\
function status() {ag('status', function(s) {var st = JSON.parse(s); adur=st.d; if(st.n) { anr = st.n; ast = st.s; ael=st.e; } else {anr = \"\"; ast = \"\"; ael = -1; } au(); }); } \n\
\n\
function l() {u(); au(); setInterval(tick, 100); status(); } \n\
function c(n) { if(cur == 0) { if(nr.length == 3) nr = ''; nr += n; if(nr.length == 3) cur=1; } else if(cur == 1) { if(st != '1') st = ''; st+=n; } u(); } \n\
function num() {cur=0; nr=''; st=''; u(); } \n\
function str() {cur=1; st=''; u(); } \n\
function s() {show(nr,st); if(st=='') {nr=''; cur=0;} else {st='';} u(); } \n\
function d() {st=''; nr=''; cur=0; u(); hide(); } \n\
 \n\
function u() {nume = document.getElementById('num'); nume.innerText=nr==''?' ':nr; nume.className=(cur==0?'sel':'');  stre = document.getElementById('str'); stre.innerText= st==''?' ':st; stre.className=(cur==1?'sel':''); } \n\
function au() {if(ael>adur) {anr = ''; ast=''; ael=-1; } document.getElementById('anum').innerText=anr; document.getElementById('astr').innerText = ast; document.getElementById('progress').style.width = (ael==-1)?'0%':(((ael/adur)*100) + '%'); } \n\
function tick() {if(ael>=0) {ael+=0.1; } au();} \n\
</script> \n\
<style type='text/css'> \n\
button {width: 32%; font-size: 3em; } \n\
#anum, #astr {width: 48%; font-size: 2em; height: 1em; float: left; text-align: center; }  \n\
#num, #str {width: 48%; font-size: 3em; height: 1.2em; float: left; text-align: center; } \n\
.sel {background: #cfc;} \n\
#progress {width: 0; margin-bottom: 0.3em; height: 0.2em; background: #070; clear: left; } \n\
#number, #buttons, #active {clear: left; } \n\
#active {background: #ccc; height: 2.4em; } \n\
#number {height: 4.2em; } \n\
</style> \n\
  </head> \n\
  <body onload='l();'> \n\
    <div id='active'><div id='anum'>000</div><div id='astr'>00</div></div> \n\
    <div id='progress'></div> \n\
    <div id='number'><div id='num' onclick='num();'>000</div><div id='str' onclick='str();'>00</div></div> \n\
    <div id='buttons'> \n\
    <button onclick='c(7);'>7</button><button onclick='c(8);'>8</button><button onclick='c(9);'>9</button><br> \n\
    <button onclick='c(4);'>4</button><button onclick='c(5);'>5</button><button onclick='c(6);'>6</button><br> \n\
    <button onclick='c(1);'>1</button><button onclick='c(2);'>2</button><button onclick='c(3);'>3</button><br> \n\
    <button onclick='d();'>DEL</button><button onclick='c(0);'>0</button><button onclick='s();'>OK</button><br> \n\
    </div> \n\
  </body> \n\
</html>"));
}

char numberBitMask(char number) {
  if(number == '0') {
    return B0000;
  } else if(number == '1') {
    return B0001;
  } else if(number == '2') {
    return B0010;
  } else if(number == '3') {
    return B0011;
  } else if(number == '4') {
    return B0100;
  } else if(number == '5') {
    return B0101;
  } else if(number == '6') {
    return B0110;
  } else if(number == '7') {
    return B0111;
  } else if(number == '8') {
    return B1000;  
  } else if(number == '9') {
    return B1001;
  } else if(number == 'A') {
    return B1010;
  } else if(number == ' ') {
    return B1011;
  } else {
    return B11111111;
  }
}

char extraBitMask(char number) {
  if(number == '1') {
    return B1010; 
  } else if(number == ' ') {
    return B1011;
  } else {
    return B11111111; 
  }
}

bool showNumber(bool lampOn, char num[]) {
  char data[7] = { B00011011, B00101011, B01001011, B10001011, B00001011, B00001111, B11111111 };
  char bm;
  if((bm = numberBitMask(num[0])) == B11111111) return false; else data[0] = B00010000 | bm;
  if((bm = numberBitMask(num[1]))  == B11111111) return false; else data[1] = B00100000 | bm;
  if((bm = numberBitMask(num[2]))   == B11111111) return false; else data[2] = B01000000 | bm;
  if((bm = numberBitMask(num[4]))   == B11111111) return false; else data[3] = B10000000 | bm;
  if((bm = extraBitMask(num[3]))   == B11111111) return false; else data[4] = B00000000 | bm;
  if(lampOn) bm = B1110; else bm = B1111; data[5] = B00000000 | bm;
  Serial.write(data, 7);
  return true;
}

void handleHide() {
  action = ACTION_HIDE;
  server.send(200, "text/javascript", F("true"));
}

void handleShow() {
  action = ACTION_SHOW;
  String num = server.arg("n");
  String str = server.arg("s");
  number[0] = number[1] = number[2] = number[3] = number[4] = ' ';
  int start = 3-num.length();
  for(int i=start; i<3; i++) {
    number[i] = num.charAt(i-start);
  }
  start = 2 - str.length();
  for(int i=start; i<2; i++) {
    number[3+i] = str.charAt(i-start);
  }
  String dur = server.arg("d");
  if(dur != "") displayTime = dur.toInt(); 
  server.send(200, "text/javascript", F("true"));
}

void handleStatus() {
  if(shown) {
    server.send(200, "text/javascript", String("{\"n\": \"") + number[0] + number[1] + number[2] + "\", \"s\":\"" + number[3] + number[4] + "\", \"d\":" + displayTime + ", \"e\":" + ((millis() - refMillis)/1000) + "}");
  } else {
    server.send(200, "text/javascript", String("{\"d\":") + displayTime + "}");
  }
}

void handleCaptive() {
  server.sendHeader("Location", String("http://192.168.1.1/cantate"), true);
  server.send ( 302, "text/plain", ""); 
  server.client().stop();
}

void setup() {
  Serial.begin(4800, SERIAL_8N1, SERIAL_TX_ONLY);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/cantate", handleRoot);
  server.on("/show", handleShow);
  server.on("/hide", handleHide);
  server.on("/status", handleStatus);
  server.onNotFound(handleCaptive);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if(action == ACTION_IDLE && shown && displayTime > 0 && (millis() - refMillis) > 1000*displayTime) {
    action = ACTION_HIDE;
  }

  if(action == ACTION_SHOW) {
    showNumber(true, number);
    refMillis = millis();
    shown = true;
    action = ACTION_IDLE;
  } else if(action == ACTION_HIDE) {
    showNumber(false, number);
    shown = false;
    action = ACTION_IDLE;
  }
}
