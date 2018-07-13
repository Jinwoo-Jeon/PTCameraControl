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
namespace PTPos {
	enum Enum {
		PAN,
		TILT,
		FOCUS,
		ZOOM
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
	void PTQueryPosition(PTPos::Enum target);
	void PTMove(PTDir::Enum dir, int speed=0);
	void PelcoDController::trackTarget(int target_x, int target_y, int screenCntr_x, int screenCntr_y);
	CString addChecksum(CString str);
	void OnWriteComm(CString str);
	BYTE byCode2AsciiValue(char cData);
};

