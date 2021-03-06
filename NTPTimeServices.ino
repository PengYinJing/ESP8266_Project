/*
 * NTPTimeServices.ino
 * 
 * Service for NTP.
 */

#include <WiFiUdp.h>
#include <TimeLib.h>

WiFiUDP Udp;
const char* ntpServerName = "au.pool.ntp.org";
//const char* ntpServerName = "xxx.ttt.ntp.org";
static int dnsFailCount = 0;
IPAddress timeServerIP; //IP address of NTP time server
unsigned int localPort = 8888;  // local port to listen for UDP packets
const int timeZone = 0; 
char isoTime[30];

char* GetISODateTime() {
  sprintf(isoTime, "%4d-%02d-%02dT%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
  return isoTime;
}

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  Udp.begin(localPort);
  
  while (Udp.parsePacket() > 0) ;
  
  if (!WiFi.hostByName(ntpServerName, timeServerIP))
  {
    dnsFailCount++;
    if (dnsFailCount == 5)
    {
      ESP.restart();
      delay(10000);
    }
    Serial.println("NTP Time Server DNS Fail");
  }
  Serial.println("Transmit NTP Request");
  
  sendNTPpacket(timeServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      Udp.stop();
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }

  Udp.stop();
  
  Serial.println("No NTP Response");
  return 0;
}

bool set_sync_ntp_time()
{
  int ntpRetryCount = 0;
  while (timeStatus() == timeNotSet && ++ntpRetryCount < 10)
  {
    // get NTP time
    setSyncProvider(getNtpTime);
    setSyncInterval(60 * 60);
  }
  if (timeStatus() == timeNotSet)
  {
    Serial.println("Sync NTP Fail");
    return false;
  }

  Serial.println("Sync NTP Success");
  return true;
}

