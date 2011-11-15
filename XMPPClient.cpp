#include <string.h>
#include <stdarg.h>
#include <Base64.h>
#include <EthernetDHCP.h>
#include <EthernetDNS.h>

#include "XMPPClient.h"


// Just a utility function to nicely format an IP address.								// REMOVE!!!!
const char* ip_to_str(const uint8_t* ipAddr) {
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}


/****************/
/* XMPP STANZAS */
/***************/
static const prog_char PROGMEM open_stream_template[] = "<stream:stream " 
                             "xmlns='jabber:client' " 
                             "xmlns:stream='http://etherx.jabber.org/streams' " 
                             "to='%s' " 
                             "version='1.0'>";

static const prog_char PROGMEM plain_auth_template[] = "<auth " 
                            "xmlns='urn:ietf:params:xml:ns:xmpp-sasl' " 
                            "mechanism='PLAIN'>" 
                            "%s" 
                            "</auth>";

static const prog_char PROGMEM bind_template[] = "<iq " 
                      "type='set' " 
                      "id='bind_1'>" 
                      "<bind " 
                      "xmlns='urn:ietf:params:xml:ns:xmpp-bind'>" 
                      "<resource>%s</resource>" 
                      "</bind>" 
                      "</iq>";

static const prog_char PROGMEM session_request_template[] = "<iq " 
                                 "to='%s' " 
                                 "type='set' " 
                                 "id='ard_sess'>" 
                                 "<session " 
                                 "xmlns='urn:ietf:params:xml:ns:xmpp-session' />" 
                                 "</iq>";

static const prog_char PROGMEM presence_template[] = "<presence>" 
                          "<show/>" 
                          "</presence>";
                          
static const prog_char PROGMEM message_template[] = "<message " 
                         "to='%s' " 
                         "xmlns='jabber:client' " 
                         "type='chat' " 
                         "id='msg' " 
                         "xml:lang='en'>" 
                         "<body>%s</body>" 
                         "</message>";
static const prog_char PROGMEM close_template[] = "<presence type='unavailable'/>"
			 "</stream:stream>";


/********************/
/* TRANSITION TABLE */
/********************/
struct XMPPTransitionTableEntry {
  XMPPState currentState;
  XMPPState nextState;
  char *keyword;
};

int connTableSize = 6;
XMPPTransitionTableEntry connTable[] = {{INIT, AUTH, "PLAIN"},
                                        {AUTH, AUTH_STREAM, "success"},
                                        {AUTH_STREAM, BIND, "bind"},
                                        {BIND, SESS, "jid"},
                                        {SESS, READY, "session"},
                                        {READY, WAIT, ""},
                                        {WAIT, WAIT, ""}};

/*****************/
/* CLASS METHODS */
/*****************/

XMPPClientClass::XMPPClientClass() {
}

// return values:
// 1 on success
// 0 on error
int XMPPClientClass::begin(byte* macAddr) {
	if (EthernetDHCP.begin(macAddr)==1) {
		const byte* myAddr = EthernetDHCP.ipAddress();
		byte myIP[] = {myAddr[0], myAddr[1], myAddr[2], myAddr[3]};
		Ethernet.begin(macAddr, myIP);
		EthernetDNS.setDNSServer(EthernetDHCP.dnsIpAddress());
		byte dnsLookupAddr[4];
		DNSError err = EthernetDNS.resolveHostName("ltg.evl.uic.edu", dnsLookupAddr);
	    if (DNSSuccess == err) {
	      	Serial.print("The IP address for 'ltg.evl.uic.udu' is ");
	      	Serial.print(ip_to_str(dnsLookupAddr));
	      	Serial.println(".");
			return 1;
	    }
		
		// const byte* ipAddr = EthernetDHCP.ipAddress();
		// 		const byte* gatewayAddr = EthernetDHCP.gatewayIpAddress();
		// 		const byte* dnsAddr = EthernetDHCP.dnsIpAddress();
		// 
		// 		Serial.println("A DHCP lease has been obtained.");
		// 
		// 		Serial.print("My IP address is ");
		// 		Serial.println(ip_to_str(ipAddr));
		// 
		// 		Serial.print("Gateway IP address is ");
		// 		Serial.println(ip_to_str(gatewayAddr));
		// 
		// 		Serial.print("DNS IP address is ");
		// 		Serial.println(ip_to_str(dnsAddr));
	};
	return 0;
}


// int XMPPClient::connect(char *username, char *server, char *resource, char *password) {
//     boolean connected = false, error = false;
//     this->username = username;
//     this->server = server;
//     this->resource = resource;
//     this->password = password;
// 
//     while(!client.connect()) {
// 	/* Retry connection every 1s and block until we're successful */
// 	delay(1000);
//     }
// 
//     while(!connected && !error) {
// 	/* Connect dat shit^H^H^H^Hcuss */
// 	if(!client.connected()) {
// 	    error = true;
// 	    continue;
// 	}
// 
// 	int ret;
// 	ret = stateAction();
// 
// 	if(ret == -1) {
// 	    error = true;
// 	}
// 
// 	if(ret == 1) {
// 	    connected = true;
// 	}
// 
// 	if(ret) {
// 	    continue;
// 	}
// 
// 	processInput();
//     }
// 
//     if(error || !connected) {
// 	return 0;
//     } else {
// 	return 1;
//     }
// }
// 
// int XMPPClient::connect(char *jid, char *password) {
// 	printf(jid);
// 	//cout << password;
//     /* Split the JID */
//     /* Call connect(char*,char*,char*,char*) */
// }
// 
// int XMPPClient::sendMessage(char *recipientJid, char *message) {
//   sendTemplate(message_template, strlen(recipientJid) + strlen(message), recipientJid, message);
// }
// 
// int XMPPClient::sendPresence() {
//   sendTemplate(presence_template, 0);
// }
// 
// int XMPPClient::close() {
//   sendTemplate(close_template, 0);
// }

int XMPPClientClass::openStream(char *server) {
    sendTemplate(open_stream_template, strlen(server), server);
}

int XMPPClientClass::authenticate(char *username, char *password) {
  int plainStringLen = strlen(username) + strlen(password) + 2;
  int encStringLen = base64_enc_len(plainStringLen);
  char plainString[plainStringLen];
  char encString[encStringLen];
  
  /* Set up our plain auth string. It's in the form:
   * "\0username\0password"
   * where \0 is the null character
   */
  memset(plainString, '\0', plainStringLen);
  memcpy(plainString + 1, username, strlen(username));
  memcpy(plainString + 2 + strlen(username), password, strlen(password));
  
  /* Encode to base64 */
  base64_encode(encString, plainString, plainStringLen);
  sendTemplate(plain_auth_template, encStringLen, encString);
}

int XMPPClientClass::bindResource(char *resource) {
  sendTemplate(bind_template, strlen(resource), resource);
}

int XMPPClientClass::openSession(char *server) {
  sendTemplate(session_request_template, strlen(server), server);
}

int XMPPClientClass::sendTemplate(const prog_char *temp_P, int fillLen, ...) {
  int tempLen = strlen_P(temp_P);
  char temp[tempLen];
  char buffer[tempLen + fillLen];
  va_list args;

  strcpy_P(temp, temp_P);

  va_start(args, fillLen);
  vsprintf(buffer, temp, args);
  client.write(buffer);

  return 1;
}

int XMPPClientClass::stateAction() {
 /*
 Serial.print("State = ");
 Serial.println(state);
 */
 switch(state) {
  case INIT:
    openStream(server);
    break;
  case AUTH:
    authenticate(username, password);
    break;
  case AUTH_STREAM:
    openStream(server);
    break;
  case BIND:
    bindResource(resource);
    break;
  case SESS:
    openSession(server);
    break;
  case READY:
    return 1;
    break;
  case WAIT:
    break;
  default:
    return -1;
    break;
 }
 return 0;
}

void XMPPClientClass::processInput() {
  int bufLen = 8;
  char buffer[bufLen];
  int i = 0;
  memset(buffer, '\0', bufLen);
  boolean stateChanged = false;

  if(!client.connected()) {
    state = WAIT;
    return;
  }

  // TODO: This process is pretty inefficient and naively implemented
  // It might be an idea to rewrite it cleverer
  while(!stateChanged) {
    if(client.available()) {
      /* Push a character from the ethernet interface into the buffer */
      for(i = 0 ; i < bufLen; i++) {
        buffer[i] = buffer[i+1];
      }
      buffer[i] = client.read();
      
      
      /* Ignore what we've read if it's an empty string */
      if(!strlen(buffer)) {
        continue;
      } else {
         //Serial.println(buffer);
      }
      
      for(int i = 0; i < connTableSize; i++) {
        if(state == connTable[i].currentState && strstr(buffer,connTable[i].keyword)) {
          
	  /*
          Serial.println(buffer);
          Serial.println(connTable[i].keyword);
          Serial.println((int)strstr(buffer, connTable[i].keyword)); 
          
          Serial.print(connTable[i].keyword);
          Serial.println(" seen, transitioning");
	  */
          
          state = connTable[i].nextState;
          client.flush();
          stateChanged = true;
          break;
        }
      }
    } else {
      delay(10);
    }
  }
}

XMPPClientClass XMPPClient;
