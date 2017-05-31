//	$Id$
// ----------------------------------------------------------------------------
// Filename:	S3SNMPTrap.h
//
// Description:	Send a trap or inform message. Based on Net-SNMP snmptrap.c
//				application.
//
//	Author:		Ian Harvey. ian.harvey@ppm.co.uk
//
//	Copyright (c) 2015 PPM Ltd
// ----------------------------------------------------------------------------
//

#define CS3SNMPTrap_MAX_STR	128
#define S3AGENT_APPNAME	"S3Agent"

class CS3SNMPTrap
{
	public:
		CS3SNMPTrap();
		~CS3SNMPTrap();

		int SendTrap(u_char inform);

		void SetCommunity(const char *c);
		void SetHost(const char *h);

		void GetCommunity(char *c);
		void GetHost(char *h);

	private:
		char m_Community[CS3SNMPTrap_MAX_STR];
		char m_Host[CS3SNMPTrap_MAX_STR];
};

// ----------------------------------------------------------------------------
