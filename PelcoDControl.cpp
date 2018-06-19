#include "PelcoDControl.h"

void PelcoDControl()
{

}

void PelcoDControl::PelcoDComm(BYTE byte[12])
{
	BYTE dByte[12];
	BYTE CByte;
	CString str;
	UINT m_nPanGetPos, m_nTiltGetPos;
	UINT m_nZoomGetPos, m_nFocusGetPos;

	for (int i = 0; i < 12; i++) {
		dByte[i] = byte[i];
	}

	if (dByte[0] == 0xFF) {
		if (dByte[1] == 0x01) {
			if (dByte[3] == 0x59) {
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte) {
					m_EditPanPos.SetSel(0, -1, TRUE);
					m_EditPanPos.Clear();

					m_nPanGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nPanGetPos);

					m_EditPanPos.SetSel(-1, 0);
					m_EditPanPos.ReplaceSel(str);
				}
				else {
					AfxMessageBox(_T("Checksum Error"));
					Clear();
				}
			}

			if (dByte[3] == 0x5B) {
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte) {
					m_EditTiltPos.SetSel(0, -1, TRUE);
					m_EditTiltPos.Clear();

					m_nTiltGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nTiltGetPos);

					m_EditTiltPos.SetSel(-1, 0);
					m_EditTiltPos.ReplaceSel(str);
				}
				else {
					AfxMessageBox(_T("Checksum Error"));
					Clear();
				}
			}

			if (dByte[3] == 0x5D) {
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte) {
					m_EditZoomPos.SetSel(0, -1, TRUE);
					m_EditZoomPos.Clear();

					m_nZoomGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nZoomGetPos);

					m_EditZoomPos.SetSel(-1, 0);
					m_EditZoomPos.ReplaceSel(str);
				}
				else {
					AfxMessageBox(_T("Checksum Error"));
					Clear();
				}
			}

			if (dByte[3] == 0x63) {
				CByte = dByte[1] + dByte[2] + dByte[3] + dByte[4] + dByte[5];
				if (dByte[6] == CByte) {
					m_EditFocusPos.SetSel(0, -1, TRUE);
					m_EditFocusPos.Clear();

					m_nFocusGetPos = ((dByte[4] & 0xFFFF) << 8) + (dByte[5] & 0xFFFF);
					str.Format(_T("%d"), m_nFocusGetPos);

					m_EditFocusPos.SetSel(-1, 0);
					m_EditFocusPos.ReplaceSel(str);
				}
				else {
					AfxMessageBox(_T("Checksum Error"));
					Clear();
				}
			}
		}
		else {
			AfxMessageBox(_T("Camera Address Error"));
			Clear();
		}
	}
	else {
		AfxMessageBox(_T("Sync Error"));
		Clear();
	}

	memset(dByte, NULL, 12);
}