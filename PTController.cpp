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

	BYTE ch[10] = { 0, };
	CString str0, str2, str3, str4, str5, strCrc1, byGetDataT;
	str0 = "FF";
	ch[0] = HexString2Int(str0);

	ch[1] = HexString2Int(strAddress);

	str2 = "00";
	ch[2] = HexString2Int(str2);

	// dir
	// 7 8 9 
	// 4 5 6 
	// 1 2 3
	if (dir == 5)
	{
		str3 = "00";
		ch[3] = HexString2Int(str3);

		str4 = "00";
		ch[4] = HexString2Int(str4);

		str5 = "00";
		ch[5] = HexString2Int(str5);
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
		ch[3] = HexString2Int(str3);

		str4.Format(_T("%02X"), speed & 0xFF);
		ch[4] = HexString2Int(str4);

		str5.Format(_T("%02X"), speed & 0xFF);
		ch[5] = HexString2Int(str5);
	}
	ch[6] = ch[1] + ch[2] + ch[3] + ch[4] + ch[5];

	strCrc1.Format("%01X", ch[6]);
	byGetDataT = str0 + strAddress + str2 + str3 + str4 + str5 + strCrc1;
	OnWriteComm(byGetDataT);
}

void PTController::OnWriteComm(CString str)
{
	int bufPos = 0;
	int datasize, bufsize, i, j;
	BYTE *Send_buff, byHigh, byLow;

	str.Replace(" ", "");// 공백 없애기 
	str.Replace("\r\n", "");//엔터 없애기
	datasize = str.GetLength(); // 공백을 없앤 문자열 길이 얻기 

								// 문자 길이를 %2로 나눈 값이 0이 아니면 홀수 이기 때문에 마지막 문자를 처리 해줘야 함
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
	//마지막 문자가 1자리수 일때 처리 하기 위해  예) 1 -> 01
	if (datasize % 2 != 0)
	{
		Send_buff[bufPos++] = byCode2AsciiValue(str[datasize - 1]);
	}

	int etc = bufPos % 8;
	//포트에 데이터를 8개씩 쓰기 위해
	//데이터의 길이가 8의 배수가 아니면 나머지 데이터는 따로 보내줌
	for (j = 0; j < bufPos - etc; j += 8)//8의 배수 보냄
	{
		m_ComuPort.WriteComm(&Send_buff[j], 8);
	}
	if (etc != 0)//나머지 데이터 전송
	{
		m_ComuPort.WriteComm(&Send_buff[bufPos - etc], etc);// 포트에 데이터 쓰기 
	}

	delete[] Send_buff;
}

BYTE PTController::byCode2AsciiValue(char cData)
{
	//이 함수는 char문자를 hex값으로 변경해 주는 함수 입니다.
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