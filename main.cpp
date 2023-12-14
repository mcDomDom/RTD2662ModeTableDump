#include "stdafx.h"

int RTD2662ModeTableDump(const char *szPath, bool bModify);

int main(int argc, char* argv[])
{
	int ret = 0;
	bool bModify = false;

	if (argc < 2) {
		fprintf(stderr, "%s firmware-path (-modify)\n", argv[0]);
		goto L_RET;
	}
	else if (2 < argc && strcmp(argv[2], "-modify")) {
		bModify = true;
	}
	ret = RTD2662ModeTableDump(argv[1], bModify);

L_RET:
	return ret;
}


