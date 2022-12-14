#pragma once

#include "targetver.h"
#include <windows.h>
#include <dbghelp.h>
#include <fcntl.h>
#include <io.h>
#include <aclapi.h>

#include <cstdio>
#include <string>
#include <vector>
#include <stack>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <regex>
#include <ctime>
#include <memory>

#include "zlib/zlib.h"
#include "bzip2/bzlib.h"
#include "minizip/ioapi.h"
#include "minizip/zip.h"
#include "minizip/unzip.h"
#include "minizip/iowin32.h"
#include "tarlib/tarlib.h"
