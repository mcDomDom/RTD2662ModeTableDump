// RTD2662ModeTableDump.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
// RTD2660やRTD2556のファームウェアからモードテーブルをダンプ
// 2022/12/06 mcDomDom
//

#include "stdafx.h"

#pragma pack(push, 1) 
	struct T_Info {
		BYTE	polarity;	// 極性フラグ 0x20:SDTV? 0x40:HDTV? 
		WORD	width;
		WORD	height;
		WORD	hfreq;
		WORD	vfreq;
		BYTE	htolerance;
		BYTE	vtolerance;
		WORD	htotal;
		WORD	vtotal;
		WORD	hstart;
		WORD	vstart;
	};
#pragma pack(pop) 

enum enModel
{
	UNKNOWN = 0,
	P2314H,				// DELL P2314H
	LHRD56_IPAD97,		// 青ジャック基板にiPad 9.7型液晶を使用した15KHzモニタ用→なんか良い愛称ないんでしょうか
	LHRD56_1366x768,	
	M_RT2556_FHD,		// 黒ジャック基板にgithubで見つけたファームウェア適用
	PHI_252B9,			// PHILIPS 252B9/11

};

void FindString(BYTE *buf, int nLength)
{

	const char CharTbl[256] = {
#if 0	// RTD2556 LCD Controller Board
	//  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
		'?', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',	// 00
		'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',	// 10
		'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',	// 20
		'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '?',	// 30
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',	// 40
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '-', '?', '?', '?', '?',	// 50
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',	// 60
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',	// 70
#else	// DELL P2314H
	//  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
		'?', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', ' ', 'C', 'D', 'E',	// 00
		'F', '-', '.', 'I', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '?', '?',	// 10
		'?', '?', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',	// 20
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',	// 30
		'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',	// 40
		'u', 'v', 'w', 'x', 'y', 'z', '?', '?', '?', '?', '?', '-', '?', '?', '?', '?',	// 50
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',	// 60
		'?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?',	// 70
#endif
	};
	int nStart = 0, nOffset = 0, nSize;
	bool bHit = false;
	for (int i=0+6; i<nLength-6; i++) {
		for (int j=0; j<0x80; j++) {
			if (buf[i+0] == '2'-'1'+j &&
				buf[i+1] == '3'-'1'+j &&
				buf[i+2] == '1'-'1'+j &&
				buf[i+3] == '4'-'1'+j
				/*
				&& buf[i+4] == 'u'-'1'+j
				&& buf[i+5] == 'm'-'1'+j
				*/
				) {
				printf("buf[%x] j=%d Hit!\n", i, j);
				bHit = true;
				nStart = i;
				nOffset = j;
				//break;
			}
		}
		//if (bHit) break;
	}
	//nStart = 0x345DA;
	nStart = 0x4D98F;	// P2314H Model Name & Firmware Code
	nSize = 32;
	//nStart = 0x42000;	// P2314H Language
	//nSize = 0x4400;
	for (int i=nStart; i<nStart+nSize; i++) {
		//int nChar = 'A'+buf[i]-nOffset;
		int nChar = CharTbl[buf[i]];
		if (isascii(nChar)) printf("%c", nChar);
	}
}

int FindModeTable(BYTE *buf, int nLength, int &nCount)
{
	int nStart = -1;

	nCount = 0;

	int nInfoSize = sizeof(T_Info);
	for (int i=0; i<nLength-nInfoSize*2; i++) {
		T_Info *pInfo1 = (T_Info *)&buf[i];
		T_Info *pInfo2 = (T_Info *)&buf[i+nInfoSize];
		short nWidth = ntohs(pInfo1->width);
		if (640 <= nWidth && nWidth <= 4096 &&
			((pInfo1->htolerance == 10 && pInfo1->vtolerance == 10 &&
			 (pInfo2->htolerance == 10 && pInfo2->vtolerance == 10) || 
			 (pInfo1->htolerance == 10 && pInfo1->vtolerance == 12 &&
			 (pInfo2->htolerance == 10 && pInfo2->vtolerance == 12))))) {
			nStart = i;
			break;
		}
	}

	if (0 <= nStart) {
		for (int i=0; i<999; i++) {
			T_Info *pInfo = (T_Info *)&buf[nStart+nInfoSize*i];
			short nWidth = ntohs(pInfo->width);
			short nHeight = ntohs(pInfo->height);
			if (nWidth < 640 || 4096 < nWidth ||
				nHeight < 240 || 2048 < nHeight) {
				break;
			}
			nCount++;
		}
	}

	return nStart;
}

void DumpModeTableRecord(FILE *fpCsv, T_Info *pInfo, int nNo, int nOffset)
{
	short	nWidth, nHeight, nHFreq, nVFreq, nHTotal, nVTotal, nHStart, nVStart;

	nWidth = ntohs(pInfo->width);
	nHeight = ntohs(pInfo->height);
	nHFreq = ntohs(pInfo->hfreq);
	nVFreq = ntohs(pInfo->vfreq);
	nHTotal = ntohs(pInfo->htotal);
	nVTotal = ntohs(pInfo->vtotal);
	nHStart = ntohs(pInfo->hstart);
	nVStart = ntohs(pInfo->vstart);

	CString str;
	str.AppendFormat(_T("%3d,0x%04X,"), nNo, nOffset);
	str.AppendFormat(_T("0x%02X,"), pInfo->polarity);
	if (pInfo->polarity & 1)	str+="HN_VN";
	else					str+="     ";
	str+=_T("|");
	if (pInfo->polarity & 2)	str+="HP_VN";
	else					str+="     ";
	str+=_T("|");
	if (pInfo->polarity & 4)	str+="HN_VP";
	else					str+="     ";
	str+=_T("|");
	if (pInfo->polarity & 8)	str+="HP_VP";
	else					str+="     ";
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nWidth);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nHeight);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nHFreq);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nVFreq);
	str+=_T(",");
	str.AppendFormat(_T("%3d"), pInfo->htolerance);
	str+=_T(",");
	str.AppendFormat(_T("%3d"), pInfo->vtolerance);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nHTotal);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nVTotal);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nHStart);
	str+=_T(",");
	str.AppendFormat(_T("%4d"), nVStart);
	str+="\n";

	_fputts(str, fpCsv);
}

int Rewrite(
FILE	*fpCsv,
BYTE	*buf,	
int		nModeTableStart,
int		nModeTableNo,
char	cPolarity,
WORD	nWidth,
WORD	nHeight,
WORD	nHFreq,
WORD	nVFreq,
BYTE	nHTol,
BYTE	nVTol,
WORD	nHTotal,
WORD	nVTotal,
WORD	nHStart,
WORD	nVStart
)
{	int nOffset = nModeTableStart+sizeof(T_Info)*nModeTableNo;
	struct T_Info *pInfo = (T_Info *)&buf[nOffset];	

	if (0 < cPolarity) pInfo->polarity = cPolarity;
	if (0 < nWidth) pInfo->width = htons(nWidth);
	if (0 < nHeight) pInfo->height = htons(nHeight);
	if (0 < nHFreq) pInfo->hfreq = htons(nHFreq);
	if (0 < nVFreq) pInfo->vfreq = htons(nVFreq);
	if (0 < nHTol) pInfo->htolerance = nHTol;
	if (0 < nVTol) pInfo->vtolerance = nVTol;
	if (0 < nHTotal) pInfo->htotal = htons(nHTotal);
	if (0 < nVTotal) pInfo->vtotal = htons(nVTotal);
	if (0 < nHStart) pInfo->hstart = htons(nHStart);
	if (0 < nVStart) pInfo->vstart = htons(nVStart);

//	DumpModeTableRecord(fpCsv, pInfo, nModeTableNo, nOffset);

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	int i, ret, nFileLen, nModeTableStart, nModeTableCount, nOffset;
	TCHAR	szPath[MAX_PATH], szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFilename[_MAX_FNAME], szExt[_MAX_EXT];
	FILE *fp = NULL;
	FILE *fpCsv = stdout;
	FILE *fpOut = NULL;
	BYTE *buf = NULL;
	enModel	model = UNKNOWN;

	if (argc < 2) {
		_ftprintf(stderr, _T("%s firmware-path (-rewrite)\n"), argv[0]);
		ret = 0;
		goto L_RET;
	}
	
	fp = _tfopen(argv[1], _T("rb"));
	if (!fp) {
		_ftprintf(stderr, _T("can't open %s\n"), argv[1]);
		ret = 1;
		goto L_RET;
	}

	_tsplitpath(argv[1], szDrive, szDir ,szFilename, szExt);
	_tmakepath(szPath, szDrive, szDir, szFilename, _T(".csv"));
	fpCsv = _tfopen(szPath, _T("wt"));
	if (!fpCsv) {
		_ftprintf(stderr, _T("can't open %s\n"), szPath);
		ret = 2;
		goto L_CLOSE;
	}

	nFileLen = _filelength(fileno(fp));
	buf = (BYTE *)malloc(nFileLen);
	if (!buf) {
		_ftprintf(stderr, _T("can't alloc memory %d\n"), nFileLen);
		ret = 3;
		goto L_CLOSE_CSV;
	}

	ret = fread(buf, nFileLen, 1, fp);
	if (!ret) {
		_ftprintf(stderr, _T("can't read %s\n"), argv[1]);
		ret = 4;
		goto L_FREE;
	}

	/* P2314の途中にある解像度テーブル？
	int ofs = 0x32344;
	for (i=0; i<77; i++) {
		printf("%d ofs(%x)=%d\n", i, 0x134cc+i*2, buf[0x134cc+i*2]*256+buf[0x134cc+i*2+1]);
	}

	for (i=0; i<256; i++) {
		printf("%d ofs(%x)=%d\n", i, ofs+i*2, buf[ofs+i*2]*256+buf[ofs+i*2+1]);
	}
	return 0;
	*/

	nModeTableStart = FindModeTable(buf, nFileLen, nModeTableCount);
	if (nModeTableStart < 0) {
		_ftprintf(stderr, _T("can't find mode table\n"));
		ret = 5;
		goto L_FREE;
	}

	// ﾓﾃﾞﾙ自動判定 ModeTable開始位置から判定 中華液晶基板ではﾌｧｰﾑｳｪｱが頻繁に変わるからあまり意味なし
	// 手持ちのRealtekｺﾝﾄﾛｰﾗ使用ﾓﾆﾀはほとんどﾌﾟﾘｾｯﾄ内容同じだったんで判別する意味も無し…
	if (0x200A == nModeTableStart) {	
		model = P2314H;
		FindString(buf, nFileLen);
	}
	else if (0x5819 == nModeTableStart) {	// LH-RD56(V+H)-01 例のiPad9.7型液晶を使用した15KHzモニタ用 2048x1536.bin
		model = LHRD56_IPAD97;
	}
	else if (0xD97E == nModeTableStart) {	// 同上 1366x768 UIは黒ジャックと同じ？
		model = LHRD56_1366x768;
	}
	else if (0x4803C == nModeTableStart) {	// 黒ジャック
		model = M_RT2556_FHD;
	}
	else if (0x32A74 == nModeTableStart) {	// 252B9 P2314Hと同じプリセットだった
		model = PHI_252B9;
	}
	
	if (2 < argc && _tcsicmp(argv[2], _T("-modify")) == 0) {
		// X68 15KHzノンインタレースは元からある1440x240の定義が使用されるから映る インタレースはテーブル定義されていない480i設定が使用される？
		//Rewrite(fpCsv, buf, nModeTableStart, 87, 0, 1460, 240, 157, 600, 10, 10, 1716, 262, 238, 20);
		Rewrite(fpCsv, buf, nModeTableStart, 139, 0, 1472, 240, 157, 600, 5, 5, 1716, 262, 238, 20); // こちらが使用される 許容誤差？を10->5

		// X68 24KHz
		Rewrite(fpCsv, buf, nModeTableStart, 10, 0x0F, 1024, 424, 247, 531, 5, 5, 1408, 464, 158, 41);	// P2314ではHStartは258くらいがベスト

		// X68 memtest68k→なぜか標準の31KHzと少し違うので専用プリセット用意
		Rewrite(fpCsv, buf, nModeTableStart, 18, 0x0F, 768, 512, 340, 554, 10, 10, 1130, 612, 320, 41);

		// X68 31KHz 
		Rewrite(fpCsv, buf, nModeTableStart, 17, 0x0F, 768, 512, 315, 556, 5, 5, 1104, 568, 264, 41);	// P2314 HS:360x
		Rewrite(fpCsv, buf, nModeTableStart, 24, 0x0F, 768, 536, 315, 543, 5, 5, 1176, 580, 308, 38);	// ダッシュ野郎

		//Rewrite(fpCsv, buf, nModeTableStart, 14, 0x0F,672, 560, 315, 530, 5, 5, 1104, 595, 108, 5+26);	// Druaga→ダメぽ
		//Rewrite(fpCsv, buf, nModeTableStart, 15, 0x0F, 640, 448, 242, 517, 10, 10, 944, 468, 234, 19);	// FZone 24K→ダメ
		Rewrite(fpCsv, buf, nModeTableStart, 14, 0x0F,1472, 256, 157, 550, 3, 3, 1716, 283, 238, 20);		// R-TYPE基板 15.7KHz/55Hz KAPPY.さん提供
		Rewrite(fpCsv, buf, nModeTableStart, 15, 0x0F,1472, 224, 157, 591, 3, 3, 1716, 262, 238, 20);		// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供

		// TOWNS SRMJ P2&P3
		//Rewrite(fpCsv, buf, nModeTableStart, 9, 0x0F, 736, 480, 322, 611, 5, 5, 872, 525, 124, 36);
		//Rewrite(fpCsv, buf, nModeTableStart, 16, 0x0F, 736, 480, 322, 611, 10, 10, 872, 525, 124, 36);
		// TOWNS Raiden Trad
		Rewrite(fpCsv, buf, nModeTableStart, 9, 0x0F, 768, 512, 323, 603, 3, 3, 1104, 536, 240, 19);

		_tsplitpath(argv[1], szDrive, szDir ,szFilename, szExt);
		_tcscat(szFilename, _T("_mod"));
		_tmakepath(szPath, szDrive, szDir, szFilename, szExt);
		fpOut = _tfopen(szPath, _T("wb"));
		if (fpOut) {
			ret = fwrite(buf, nFileLen, 1, fpOut);
			if (!ret) {
				_ftprintf(stderr, _T("can't write %s\n"), szPath);
			}
			fclose(fpOut);
		}
		else {
			_ftprintf(stderr, _T("can't open %s\n"), szPath);
		}
	}

	nOffset = nModeTableStart;
					//   0	        1         2         3         4         5         6         7         8        
					//   123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
	_ftprintf(fpCsv, _T("No ,Offset,PF  ,PolarityFlag           ,W   ,H   ,HFrq,VFrq,HTl,VTl,HTot,VTot,HSta,VSta\n"));
	for (i=0; i<nModeTableCount; i++) {
		struct T_Info *pInfo = (T_Info *)&buf[nOffset];
		DumpModeTableRecord(fpCsv, pInfo, i, nOffset);
		nOffset += sizeof(T_Info);
	}

	//FindString(buf, nFileLen);


	ret = 0;

L_FREE:
	free(buf);
L_CLOSE_CSV:
	fclose(fpCsv);
L_CLOSE:
	fclose(fp);
L_RET:
	return ret;
}


