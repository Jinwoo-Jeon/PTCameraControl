#include "stdafx.h"
#include "PelcoDController.h"


PelcoDController::PelcoDController()
{
}


PelcoDController::~PelcoDController()
{
}
void PelcoDController::PTMove(PTDir::Enum dir, int speed)
{
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;
	str0 = "FF";
	str2 = "00";

	if (dir == PTDir::STOP)
	{
		str3 = "00";
		str4 = "00";
		str5 = "00";
	}
	else
	{
		int dirStr = 0;
		switch (dir)
		{
		case PTDir::RIGHT:
		case PTDir::RIGHTUP:
		case PTDir::RIGHTDOWN:
			dirStr += 2;
			break;
		case PTDir::LEFT:
		case PTDir::LEFTUP:
		case PTDir::LEFTDOWN:
			dirStr += 4;
			break;
		}
		switch(dir)
		{
		case PTDir::LEFTUP:
		case PTDir::UP:
		case PTDir::RIGHTUP:
			dirStr += 8;
			break;
		case PTDir::LEFTDOWN:
		case PTDir::DOWN:
		case PTDir::RIGHTDOWN:
			dirStr += 16;
			break;
		}
		str3.Format(_T("%02X"), dirStr & 0xFF);
		str4.Format(_T("%02X"), speed & 0xFF);
		str5.Format(_T("%02X"), speed & 0xFF);
	}
	byGetDataT = addChecksum(str0 + strAddress + str2 + str3 + str4 + str5);
	OnWriteComm(byGetDataT);
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
		checksumStr.Format("%01X", ch[6]);
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
	m_EditCommunicationSend.ReplaceSel("\n");
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