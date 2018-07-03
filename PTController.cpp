#include "stdafx.h"
#include "PTController.h"


PTController::PTController()
{
}


PTController::~PTController()
{
}
void PTController::PTMove(int dir, int speed)
{
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;
	str0 = "FF";
	str2 = "00";

	// dir
	// 7 8 9 
	// 4 5 6 
	// 1 2 3
	if (dir == 5)
	{
		str3 = "00";
		str4 = "00";
		str5 = "00";
	}
	else
	{
		int dirStr = 0;
		if (dir == 3 || dir == 6 || dir == 9)
			dirStr += 2;
		else if (dir == 1 || dir == 4 || dir == 7)
			dirStr += 4;
		if (dir == 7 || dir == 8 || dir == 9)
			dirStr += 8;
		else if (dir == 1 || dir == 2 || dir == 3)
			dirStr += 16;
		str3.Format(_T("%02X"), dirStr & 0xFF);
		str4.Format(_T("%02X"), speed & 0xFF);
		str5.Format(_T("%02X"), speed & 0xFF);
	}
	byGetDataT = addChecksum(str0 + strAddress + str2 + str3 + str4 + str5);
	//m_EditCommunicationSend.SetSel(-1, 0);
	//m_EditCommunicationSend.ReplaceSel(str0 + " " + strAddress + " " + str2 + " " + str3 + " " + str4 + " " + str5 + " " + strCrc1 +"\n");
	OnWriteComm(byGetDataT);
}

CString PTController::addChecksum(CString str)
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

void PTController::OnWriteComm(CString str)
{
	m_EditCommunicationSend.SetSel(-1, 0);
	m_EditCommunicationSend.ReplaceSel(str + "\n");
	int bufPos = 0;
	int datasize, bufsize, i, j;
	BYTE *Send_buff, byHigh, byLow;

	str.Replace(" ", "");// ���� ���ֱ� 
	str.Replace("\r\n", "");//���� ���ֱ�
	datasize = str.GetLength(); // ������ ���� ���ڿ� ���� ��� 

								// ���� ���̸� %2�� ���� ���� 0�� �ƴϸ� Ȧ�� �̱� ������ ������ ���ڸ� ó�� ����� ��
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
	//������ ���ڰ� 1�ڸ��� �϶� ó�� �ϱ� ����  ��) 1 -> 01
	if (datasize % 2 != 0)
	{
		Send_buff[bufPos++] = byCode2AsciiValue(str[datasize - 1]);
	}

	int etc = bufPos % 8;
	//��Ʈ�� �����͸� 8���� ���� ����
	//�������� ���̰� 8�� ����� �ƴϸ� ������ �����ʹ� ���� ������
	for (j = 0; j < bufPos - etc; j += 8)//8�� ��� ����
	{
		m_ComuPort.WriteComm(&Send_buff[j], 8);
	}
	if (etc != 0)//������ ������ ����
	{
		m_ComuPort.WriteComm(&Send_buff[bufPos - etc], etc);// ��Ʈ�� ������ ���� 
	}

	delete[] Send_buff;
}

BYTE PTController::byCode2AsciiValue(char cData)
{
	//�� �Լ��� char���ڸ� hex������ ������ �ִ� �Լ� �Դϴ�.
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