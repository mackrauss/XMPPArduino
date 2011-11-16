#ifndef __XMPP_CLIENT_H__
#define __XMPP_CLIENT_H__

#include <Ethernet.h>
#include <avr/pgmspace.h>

enum XMPPState {
  INIT,
  AUTH,
  AUTH_STREAM,
  BIND,
  SESS,
  READY,
  WAIT
};

class XMPPClientClass {
private:
	Client *client;
	char *username;
	char *server;
	char *password;
	char *resource;
	XMPPState state;
		
	int sendTemplate(const prog_char *strTemplate, int fillLen, ...);	
	int openStream(char *server);
	int authenticate(char *username, char *password);
	int bindResource(char *resource);
	int openSession(char *server);	
	void processInput();
	int stateAction();
		
public:
	XMPPClientClass();
	int begin(byte* macAddr);
	void connect(char *username, char *server, char *resource, char *password);
	void connect(char *jid, char *password);
	void maintain();
	// int sendMessage(char *recipientJid, char *message);
	// int sendPresence();
	// int close();
	// int end();
};

extern XMPPClientClass XMPPClient;

#endif // __XMPP_CLIENT_H__
