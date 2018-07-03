#pragma once
#include "CommThread.h"
#include "crc16.h"

class PTController
{
public:
	CString		strAddress;
	CCommThread m_ComuPort;
	CEdit		m_EditCommunicationSend;

	PTController();
	~PTController();
	void PTMove(int dir, int speed=0);
	CString addChecksum(CString str);
	void OnWriteComm(CString str);
	BYTE byCode2AsciiValue(char cData);
};

