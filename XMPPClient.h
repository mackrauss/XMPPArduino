#ifndef __XMPP_CLIENT_H__
#define __XMPP_CLIENT_H__

#include <Ethernet.h>
<<<<<<< HEAD
//#include "Client.h"
#include "Base64.h"
#include <string.h>
#include <stdarg.h>
=======
>>>>>>> parent of f82e78d... Revert 3d762a6eb9fe257e092c88cc7c669ca69d2a0b84^..HEAD
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
<<<<<<< HEAD


    public:
	XMPPClient();
	XMPPClient(uint8_t *ip, uint16_t port);

	int connect(char *username, char *server, char *resource, char *password);
	int connect(char *jid, char *password);

	int sendMessage(char *recipientJid, char *message);
	int sendPresence();

	int close();

=======
		
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
>>>>>>> parent of f82e78d... Revert 3d762a6eb9fe257e092c88cc7c669ca69d2a0b84^..HEAD
};

extern XMPPClientClass XMPPClient;

#endif // __XMPP_CLIENT_H__
