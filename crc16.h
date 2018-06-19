#ifndef _CRC16_H_
#define _CRC16_H_

unsigned short crc16_ccitt(char *buf, int len);
//unsigned short crc16_ccitt(char buf, int len);
unsigned short make_crc_table( void );
CString ConvertToHex(CString data);
inline BYTE HexToByte(CString Ps_Hex);
char HexString2Int(CString hex);
//int DecToHex(int p_intValue);

#endif /* _CRC16_H_ */