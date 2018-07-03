#pragma once
#include "CommThread.h"
#include "crc16.h"

namespace PTDir {
	enum Enum {
		LEFTDOWN,
		DOWN,
		RIGHTDOWN,
		LEFT,
		STOP,
		RIGHT,
		LEFTUP,
		UP,
		RIGHTUP
	};
}

class PelcoDController
{
public:
	CString		strAddress;
	CCommThread m_ComuPort;
	CEdit		m_EditCommunicationSend;

	PelcoDController();
	~PelcoDController();
	void PTMove(PTDir::Enum dir, int speed=0);
	CString addChecksum(CString str);
	void OnWriteComm(CString str);
	BYTE byCode2AsciiValue(char cData);
};

