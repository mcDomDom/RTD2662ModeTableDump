#pragma once

#ifdef WIN32
#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <WinSock.h>
#else
#include <arpa/inet.h>
#endif

