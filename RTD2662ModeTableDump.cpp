// RTD2662ModeTableDump.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
// RTD2660やRTD2556のファームウェアからモードテーブルをダンプ
// 2022/12/06 mcDomDom
//

#include "stdafx.h"

#pragma pack(push, 1) 
	struct T_Info {
		BYTE	polarity;	// 極性フラグ 0x20:SDTV? 0x40:HDTV?  0x10:Interlace?
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

	// RTD2668
	struct T_Info_23 {
		BYTE	no;
		BYTE	type;
		BYTE	polarity;	// 極性フラグ 0x20:SDTV? 0x40:HDTV?  0x10:Interlace?
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
		WORD	vcount;
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
	PCB800099,			// RTD2662使用基板
	RTD2668
};

enum enIndex
{
	X68_15K_I,
	X68_15K_P,
	X68_24K_I,
	X68_24K_P,
	X68_31K,
	X68_31K_MT,	// memtest68K
	X68_Dash,
	M72_RTYPE,
	FMT_Raiden,
	MAX_INDEX
};

BYTE *buf = NULL;
int nModeTableStart = 0;

void FindString(int nLength)
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

template <typename T>
int FindModeTable(int nLength, int &nCount)
{
	int nStart = -1, nSearchStart = 0;

	nCount = 0;

L_RETRY:
	int nInfoSize = sizeof(T_Info);
	for (int i=nSearchStart; i<nLength-nInfoSize*2; i++) {
		T *pInfo1 = (T *)&buf[i];
		T *pInfo2 = (T*)&buf[i+nInfoSize];
		short nWidth = ntohs(pInfo1->width);
		if (640 <= nWidth && nWidth <= 4096 &&
			5 <= pInfo1->htolerance && pInfo1->htolerance <= 10 &&
			5 <= pInfo1->vtolerance && pInfo1->vtolerance <= 10 &&
			5 <= pInfo2->htolerance && pInfo2->htolerance <= 10 &&
			5 <= pInfo2->vtolerance && pInfo2->vtolerance <= 10) {
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
	if (nCount == 0 && 0 <= nStart) {
		nSearchStart = nStart+1;
		goto L_RETRY;
	}

	return nStart;
}

template <typename T>
void DumpModeTableRecord(FILE *fpCsv, T *pInfo, int nNo, int nOffset)
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

template <typename T>
int SetParameter(
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
{	int nOffset = nModeTableStart+sizeof(T)*nModeTableNo;
	T *pInfo = (T *)&buf[nOffset];	

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
	int i, ret, nFileLen, nModeTableCount, nOffset;
	int	nIdxNo[MAX_INDEX] = {-1};
	TCHAR	szPath[MAX_PATH], szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFilename[_MAX_FNAME], szExt[_MAX_EXT];
	FILE *fp = NULL;
	FILE *fpCsv = stdout;
	FILE *fpOut = NULL;
	enModel	model = UNKNOWN;

	if (argc < 2) {
		_ftprintf(stderr, _T("%s firmware-path (-modify)\n"), argv[0]);
		ret = 0;
		goto L_RET;
	}
	
	fp = _tfopen(argv[1], _T("rb"));
	if (!fp) {
		_ftprintf(stderr, _T("can't open %s\n"), argv[1]);
		ret = 1;
		goto L_RET;
	}

	nFileLen = _filelength(_fileno(fp));
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

	nModeTableStart = FindModeTable<T_Info>(nFileLen, nModeTableCount);
	if (nModeTableStart < 0) {
		nModeTableStart = FindModeTable<T_Info_23>(nFileLen, nModeTableCount);
		if (nModeTableStart < 0) {
			_ftprintf(stderr, _T("can't find mode table\n"));
			ret = 5;
			goto L_FREE;
		}
		model = RTD2668;
	}

	// P2314Hの画面ﾓｰﾄﾞと使用ﾌﾟﾘｾｯﾄﾃｰﾌﾞﾙNoの紐づけ
	nIdxNo[X68_15K_I] = 0;
	nIdxNo[X68_15K_P] = 1;
	nIdxNo[X68_24K_I] = 4;
	nIdxNo[X68_24K_P] = 6;
	nIdxNo[X68_31K] = 24;
	nIdxNo[X68_31K_MT] = 18;	// 元の800x600ﾌﾟﾘｾｯﾄを上書き
	nIdxNo[X68_Dash] = 26;
	nIdxNo[M72_RTYPE] = 27;
	nIdxNo[FMT_Raiden] = 28;
	// 38-43も使ってもよさそう

	if (model == UNKNOWN) {
		// ﾓﾃﾞﾙ自動判定 ModeTable開始位置から判定 中華液晶基板ではﾌｧｰﾑｳｪｱが頻繁に変わるからあまり意味なし
		switch (nModeTableStart) {
		case 0x200A:	// P2314H
			model = P2314H;
			//FindString(nFileLen);
			break;
		case 0x32A74:	// 252B9
			model = PHI_252B9;
			// プリセットテーブルはP2314Hと同じ
			break;
		case 0x5819:	// LH-RD56(V+H)-01 例のiPad9.7型液晶を使用した15KHzモニタ用 2048x1536.bin
			model = LHRD56_IPAD97;
			nIdxNo[X68_15K_I] = 0;
			nIdxNo[X68_15K_P] = 1;
			nIdxNo[X68_24K_I] = 4;
			nIdxNo[X68_24K_P] = 6;
			nIdxNo[X68_31K] = 25;
			nIdxNo[X68_31K_MT] = 19;	// 元の800x600ﾌﾟﾘｾｯﾄを上書き
			nIdxNo[X68_Dash] = 27;
			nIdxNo[M72_RTYPE] = 28;
			nIdxNo[FMT_Raiden] = 37;
			// 38-43も使ってもよさそう
			break;
		case 0xD97E:	// 同上 1366x768 UIは黒ジャックと同じ？
			model = LHRD56_1366x768;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x4803C:	// M.RT2556 黒ジャック 1920x1080
			model = M_RT2556_FHD;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x39c7:	// PCB800099(RTD2660/RTD2662)
			model = PCB800099;
			nIdxNo[X68_15K_I] = 0;
			nIdxNo[X68_15K_P] = 3;
			nIdxNo[X68_24K_I] = 4;
			nIdxNo[X68_24K_P] = 13;
			nIdxNo[X68_31K] = 18;
			nIdxNo[X68_31K_MT] = 9;	// 元の800x600ﾌﾟﾘｾｯﾄを上書き
			nIdxNo[X68_Dash] = 20;
			nIdxNo[M72_RTYPE] = 21;
			nIdxNo[FMT_Raiden] = 22;
			// 22-31が空き
			break;
		}
	}
	
	_tsplitpath(argv[1], szDrive, szDir ,szFilename, szExt);
	if (2 < argc && _tcsicmp(argv[2], _T("-modify")) == 0 &&
		model != UNKNOWN && model != RTD2668) {
		//										 Pol   Wid   Hei  HFrq VFrq HT VT HTot  VTot HSB  VSB
  		SetParameter<T_Info>(nIdxNo[X68_15K_I],  0x1F,  512, 480, 159, 615, 5, 5,  608, 521,  78, 24);		// X68000  512x512 15KHz(interlace) -> 標準の480iﾌﾟﾘｾｯﾄが使用されるため無効
		SetParameter<T_Info>(nIdxNo[X68_15K_P],  0x0F,  512, 240, 159, 615, 5, 5,  608, 260,  80, 12);		// X68000  512x240 15KHz
		SetParameter<T_Info>(nIdxNo[X68_24K_I],  0x1F, 1024, 848, 246, 532, 5, 5, 1408, 931, 282, 46);		// X68000 1024x848 24KHz(interlace) 偶数・奇数ライン逆？
		SetParameter<T_Info>(nIdxNo[X68_24K_P],  0x0F, 1024, 424, 246, 532, 5, 5, 1408, 465, 282, 23);		// X68000 1024x424 24KHz
		SetParameter<T_Info>(nIdxNo[X68_31K],    0x0F,  768, 512, 314, 554, 5, 5, 1104, 568, 261, 32);		// X68000  768x512 31KHz
		//SetParameter<T_Info>(nIdxNo[X68_31K_MT], 0x0F,  768, 512, 338, 554, 5, 5, 1024, 613, 192, 35);		// X68000  768x512 31KHz memtest
		SetParameter<T_Info>(nIdxNo[M72_RTYPE],  0x0F,  768, 256, 157, 550, 5, 5, 1024, 284, 156, 24);		// R-TYPE基板 15.7KHz/55Hz KAPPY.さん提供
		SetParameter<T_Info>(nIdxNo[FMT_Raiden], 0x0F,  768, 512, 323, 603, 3, 3, 1104, 536, 240, 19);		// TOWNS 雷電伝説
		//どうも縦像度240未満は動作しない？
		//SetParameter<T_Info>(nIdxNo[FMT_Raiden]+1, 0x0F, 576, 224, 157, 591, 3, 3, 768, 263, 120, 24);		// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供

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

	_tmakepath(szPath, szDrive, szDir, szFilename, _T(".csv"));
	fpCsv = _tfopen(szPath, _T("wt"));
	if (!fpCsv) {
		_ftprintf(stderr, _T("can't open %s\n"), szPath);
		ret = 2;
		goto L_CLOSE;
	}
	nOffset = nModeTableStart;
					//   0	        1         2         3         4         5         6         7         8        
					//   123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
	_ftprintf(fpCsv, _T("No ,Offset,PF  ,PolarityFlag           ,W   ,H   ,HFrq,VFrq,HTl,VTl,HTot,VTot,HSta,VSta\n"));
	for (i=0; i<nModeTableCount; i++) {
		if (model == RTD2668) {
			struct T_Info_23 *pInfo = (T_Info_23 *)&buf[nOffset];
			DumpModeTableRecord<T_Info_23>(fpCsv, pInfo, i, nOffset);
			nOffset += sizeof(T_Info_23);
		}
		else {
			struct T_Info *pInfo = (T_Info *)&buf[nOffset];
			DumpModeTableRecord<T_Info>(fpCsv, pInfo, i, nOffset);
			nOffset += sizeof(T_Info);
		}
	}

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


