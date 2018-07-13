#include "stdafx.h"
#include "PelcoDController.h"


PelcoDController::PelcoDController()
{
}


PelcoDController::~PelcoDController()
{
}


void PelcoDController::PTQueryPosition(PTPos::Enum target)
{
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;

	CString logStr = "";
	m_EditCommunicationSend.SetSel(-1, 0);
	switch (target)
	{
	case PTPos::PAN:
		logStr = "\r\n[Query] Pan Position\r\n";
		str3 = "51";
		break;
	case PTPos::TILT:
		logStr = "\r\n[Query] Tilt Position\r\n";
		str3 = "53";
		break;
	case PTPos::ZOOM:
		logStr = "\r\n[Query] Focus Position\r\n";
		str3 = "55";
		break;
	case PTPos::FOCUS:
		logStr = "\r\n[Query] Zoom Position\r\n";
		str3 = "61";
		break;
	}
	m_EditCommunicationSend.ReplaceSel(logStr);
	byGetDataT = addChecksum("FF" + strAddress + "00" + str3 + "0000");
	OnWriteComm(byGetDataT);
}

void PelcoDController::PTMove(PTDir::Enum dir, int speed)
{
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;
	str0 = "FF";
	str2 = "00";

	int dirStr = 0;
	CString logStr = "";
	m_EditCommunicationSend.SetSel(-1, 0);
	switch (dir)
	{
	case PTDir::STOP:
		logStr = "\r\n[PT Move] Stop\r\n";
		dirStr = 0;
		break;
	case PTDir::RIGHT:
		logStr.Format("\r\n[PT Move] Right w/ Speed: %d\r\n", speed);
		dirStr = 2;
		break;
	case PTDir::LEFT:
		logStr.Format("\r\n[PT Move] Left w/ Speed: %d\r\n", speed);
		dirStr = 4;
		break;
	case PTDir::UP:
		logStr.Format("\r\n[PT Move] Up w/ Speed: %d\r\n", speed);
		dirStr = 8;
		break;
	case PTDir::RIGHTUP:
		logStr.Format("\r\n[PT Move] RightUp w/ Speed: %d\r\n", speed);
		dirStr = 10;
		break;
	case PTDir::LEFTUP:
		logStr.Format("\r\n[PT Move] LeftUp w/ Speed: %d\r\n", speed);
		dirStr = 12;
		break;
	case PTDir::DOWN:
		logStr.Format("\r\n[PT Move] Down w/ Speed: %d\r\n", speed);
		dirStr = 16;
		break;
	case PTDir::RIGHTDOWN:
		logStr.Format("\r\n[PT Move] RightDown w/ Speed: %d\r\n", speed);
		dirStr = 18;
		break;
	case PTDir::LEFTDOWN:
		logStr.Format("\r\n[PT Move] LeftDown w/ Speed: %d\r\n", speed);
		dirStr = 20;
		break;
	}
	m_EditCommunicationSend.ReplaceSel(logStr);
	str3.Format(_T("%02X"), dirStr & 0xFF);
	str4.Format(_T("%02X"), speed & 0xFF);
	str5.Format(_T("%02X"), speed & 0xFF);
	byGetDataT = addChecksum(str0 + strAddress + str2 + str3 + str4 + str5);
	OnWriteComm(byGetDataT);
}


void PelcoDController::trackTarget(int target_x, int target_y, int screenCntr_x, int screenCntr_y)
{
	int centerMargin = 10;

	int dist_x = target_x - screenCntr_x;
	int dist_y = target_y - screenCntr_y;
	int dist = int(sqrt(dist_x*dist_x + dist_y*dist_y));
	int vel = int(abs(dist) * 24 / sqrt(screenCntr_x*screenCntr_x + screenCntr_y*screenCntr_y) + 40);
	if (dist_x > centerMargin)
	{
		if (dist_y > centerMargin)
		{
			PTMove(PTDir::RIGHTDOWN, vel);
		}
		else if (dist_y < -centerMargin)
		{
			PTMove(PTDir::RIGHTUP, vel);
		}
		else
		{
			PTMove(PTDir::RIGHT, vel);
		}
	}
	else if (dist_x < -centerMargin)
	{
		if (dist_y > centerMargin)
		{
			PTMove(PTDir::LEFTDOWN, vel);
		}
		else if (dist_y < -centerMargin)
		{
			PTMove(PTDir::LEFTUP, vel);
		}
		else
		{
			PTMove(PTDir::LEFT, vel);
		}
	}
	else
	{
		if (dist_y > centerMargin)
		{
			PTMove(PTDir::DOWN, vel);
		}
		else if (dist_y < -centerMargin)
		{
			PTMove(PTDir::UP, vel);
		}
		else
		{
			PTMove(PTDir::STOP);
		}
	}
}

CString PelcoDController::addChecksum(CString str)
{
	if (strlen(str) != 12)
	{
		ATLTRACE("Pelco D String Length Err");
	}
	else
	{
		BYTE ch[7];
		CString checksumStr, byGetDataT;
		for (unsigned int i = 0; i < strlen(str); i += 2) {
			CString byteString = str.Mid(i, 2);
			ch[i / 2] = HexString2Int(byteString);
		}
		ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];
		checksumStr.Format("%02X", ch[6]);
		return str + checksumStr;
	}
}

void PelcoDController::OnWriteComm(CString str)
{
	m_EditCommunicationSend.SetSel(-1, 0);
	for (unsigned int i = 0; i < strlen(str); i += 2) {
		CString byteString = str.Mid(i, 2);
		m_EditCommunicationSend.ReplaceSel(byteString + " ");
	}
	m_EditCommunicationSend.ReplaceSel("\r\n");
	int bufPos = 0;
	int datasize, bufsize, i, j;
	BYTE *Send_buff, byHigh, byLow;

	str.Replace(" ", "");
	str.Replace("\r\n", "");

	datasize = str.GetLength(); 
	if (datasize % 2 == 0)
	{
		bufsize = datasize;
	}
	else
	{
		bufsize = datasize - 1;
	}

	Send_buff = new BYTE[bufsize];

	for (i = 0; i < bufsize; i += 2)
	{
		byHigh = byCode2AsciiValue(str[i]);
		byLow = byCode2AsciiValue(str[i + 1]);
		Send_buff[bufPos++] = (byHigh << 4) | byLow;

	}
	if (datasize % 2 != 0)
	{
		Send_buff[bufPos++] = byCode2AsciiValue(str[datasize - 1]);
	}

	int etc = bufPos % 8;
	for (j = 0; j < bufPos - etc; j += 8)
	{
		m_ComuPort.WriteComm(&Send_buff[j], 8);
	}
	if (etc != 0)
	{
		m_ComuPort.WriteComm(&Send_buff[bufPos - etc], etc);
	}

	delete[] Send_buff;
}

BYTE PelcoDController::byCode2AsciiValue(char cData)
{
	BYTE byAsciiValue;
	if (('0' <= cData) && (cData <= '9'))
	{
		byAsciiValue = cData - '0';
	}
	else if (('A' <= cData) && (cData <= 'F'))
	{
		byAsciiValue = (cData - 'A') + 10;
	}
	else if (('a' <= cData) && (cData <= 'f'))
	{
		byAsciiValue = (cData - 'a') + 10;
	}
	else
	{
		byAsciiValue = 0;
	}
	return byAsciiValue;
}