
#include "stdafx.h"
#include "Utils.h"


constexpr wchar_t* qualys_program_data_legacy_location =  L"%SystemDrive%\\Documents and Settings\\All Users\\Application Data\\Qualys\\OSSLScan";
constexpr wchar_t* qualys_program_data_location = L"%ProgramData%\\Qualys\\OSSLScan";
constexpr wchar_t* report_sig_output_file = L"findings.out";
constexpr wchar_t* report_sig_summary_file = L"summary.out";


int DumpGenericException(const wchar_t* szExceptionDescription,
                         DWORD dwExceptionCode, PVOID pExceptionAddress) {
  wprintf(L"Unhandled Exception Detected - Reason: %s (0x%x) at address 0x%p\n\n",
    szExceptionDescription, 
    dwExceptionCode, 
    pExceptionAddress);

  return 0;
}

int DumpExceptionRecord(PEXCEPTION_POINTERS pExPtrs) {
  PVOID pExceptionAddress = pExPtrs->ExceptionRecord->ExceptionAddress;
  DWORD dwExceptionCode = pExPtrs->ExceptionRecord->ExceptionCode;

  switch (dwExceptionCode) {
    case 0xE06D7363:
      DumpGenericException(L"Out Of Memory (C++ Exception)", dwExceptionCode, pExceptionAddress);
      break;
    case EXCEPTION_ACCESS_VIOLATION:
      wchar_t szStatus[256];
      wchar_t szSubStatus[256];
      wcscpy_s(szStatus, L"Access Violation");
      wcscpy_s(szSubStatus, L"");
      if (pExPtrs->ExceptionRecord->NumberParameters == 2) {
        switch (pExPtrs->ExceptionRecord->ExceptionInformation[0]) {
          case 0:  // read attempt
            swprintf_s(szSubStatus, L"read attempt to address 0x%p",
                       (void*)pExPtrs->ExceptionRecord->ExceptionInformation[1]);
            break;
          case 1:  // write attempt
            swprintf_s(szSubStatus, L"write attempt to address 0x%p",
                       (void*)pExPtrs->ExceptionRecord->ExceptionInformation[1]);
            break;
        }
      }
      wprintf(
          L"Unhandled Exception Detected - Reason: %s(0x%x) at address 0x%p %s\n\n",
          szStatus, dwExceptionCode, pExceptionAddress, szSubStatus);
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      DumpGenericException(L"Data Type Misalignment", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_BREAKPOINT:
      DumpGenericException(L"Breakpoint Encountered", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_SINGLE_STEP:
      DumpGenericException(L"Single Instruction Executed", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      DumpGenericException(L"Array Bounds Exceeded", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      DumpGenericException(L"Float Denormal Operand", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      DumpGenericException(L"Divide by Zero", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_INEXACT_RESULT:
      DumpGenericException(L"Float Inexact Result", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_INVALID_OPERATION:
      DumpGenericException(L"Float Invalid Operation", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_OVERFLOW:
      DumpGenericException(L"Float Overflow", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_STACK_CHECK:
      DumpGenericException(L"Float Stack Check", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_FLT_UNDERFLOW:
      DumpGenericException(L"Float Underflow", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      DumpGenericException(L"Integer Divide by Zero", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_INT_OVERFLOW:
      DumpGenericException(L"Integer Overflow", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_PRIV_INSTRUCTION:
      DumpGenericException(L"Privileged Instruction", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_IN_PAGE_ERROR:
      DumpGenericException(L"In Page Error", dwExceptionCode, pExceptionAddress);
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      DumpGenericException(L"Illegal Instruction", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      DumpGenericException(L"Noncontinuable Exception", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_STACK_OVERFLOW:
      DumpGenericException(L"Stack Overflow", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_INVALID_DISPOSITION:
      DumpGenericException(L"Invalid Disposition", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_GUARD_PAGE:
      DumpGenericException(L"Guard Page Violation", dwExceptionCode,
                           pExceptionAddress);
      break;
    case EXCEPTION_INVALID_HANDLE:
      DumpGenericException(L"Invalid Handle", dwExceptionCode,
                           pExceptionAddress);
      break;
    case CONTROL_C_EXIT:
      DumpGenericException(L"Ctrl+C Exit", dwExceptionCode, pExceptionAddress);
      break;
    default:
      DumpGenericException(L"Unknown exception", dwExceptionCode,
                           pExceptionAddress);
      break;
  }

  return 0;
}

LONG CALLBACK CatchUnhandledExceptionFilter(PEXCEPTION_POINTERS pExPtrs) {
  WCHAR szMiniDumpFileName[MAX_PATH];
  HANDLE hDumpFile = NULL;
  SYSTEMTIME sysTime;
  SECURITY_ATTRIBUTES saMiniDumpSecurity;

  wprintf(L"Run status : Failed\n");

  // Attempt to dump an unhandled exception banner just in case things are
  // so bad that a minidump cannot be created.
  DumpExceptionRecord(pExPtrs);

  // Create a directory to dump the minidump files into
  SecureZeroMemory(&saMiniDumpSecurity, sizeof(saMiniDumpSecurity));
  saMiniDumpSecurity.nLength = sizeof(saMiniDumpSecurity);
  saMiniDumpSecurity.bInheritHandle = FALSE;

  // Construct a valid minidump filename that will be unique.
  // Use the '.mdmp' extension so it'll be recognize by the Windows debugging
  // tools.
  GetLocalTime(&sysTime);
  swprintf_s(szMiniDumpFileName,
             L"%s\\%0.2d%0.2d%0.4d%d%0.2d%0.2d%0.4d.mdmp",
             GetScanUtilityDirectory().c_str(),
             sysTime.wMonth,
             sysTime.wDay,
             sysTime.wYear,
             sysTime.wHour,
             sysTime.wMinute,
             sysTime.wSecond,
             sysTime.wMilliseconds);

  wprintf(L"Creating minidump file %s with crash details.\n",
                   szMiniDumpFileName);

  // Create the file to dump the minidump data into...
  //
  hDumpFile = CreateFile(szMiniDumpFileName, GENERIC_WRITE, NULL, NULL,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hDumpFile != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION eiMinidumpInfo;
    SecureZeroMemory(&eiMinidumpInfo, sizeof(eiMinidumpInfo));
    eiMinidumpInfo.ThreadId = GetCurrentThreadId();
    eiMinidumpInfo.ExceptionPointers = pExPtrs;
    eiMinidumpInfo.ClientPointers = FALSE;

    //
    // Write the Mini Dump to disk
    //
    if (!MiniDumpWriteDump(
            GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
            (MINIDUMP_TYPE)(MiniDumpNormal |
                            MiniDumpWithPrivateReadWriteMemory |
                            MiniDumpWithDataSegs | MiniDumpWithHandleData |
                            MiniDumpWithFullMemoryInfo |
                            MiniDumpWithThreadInfo |
                            MiniDumpWithUnloadedModules |
                            MiniDumpWithIndirectlyReferencedMemory),
            &eiMinidumpInfo, NULL, NULL)) {
      // Either the state of the process is beyond our ability to be able
      // to scape together a usable dump file or we are on XP/2k3 and
      // not all of the dump flags are supported.  Retry using dump flags
      // that are supported by XP.
      //
      if (!MiniDumpWriteDump(
              GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
              (MINIDUMP_TYPE)(MiniDumpNormal |
                              MiniDumpWithPrivateReadWriteMemory |
                              MiniDumpWithDataSegs | MiniDumpWithHandleData),
              &eiMinidumpInfo, NULL, NULL)) {
        // Well out XP/2k3 compatible list of parameters didn't work, it
        // doesn't look like we will be able to get anything useful.
        //
        // Close things down and delete the file if it exists.
        //
        SAFE_CLOSE_HANDLE(hDumpFile);
        DeleteFile(szMiniDumpFileName);

        wprintf(L"Failed to create minidump file %s.\n", szMiniDumpFileName);
      }
    }

    SAFE_CLOSE_HANDLE(hDumpFile);
  }

  TerminateProcess(GetCurrentProcess(), pExPtrs->ExceptionRecord->ExceptionCode);

  return 0;
}

std::vector<std::wstring> error_array;
uint32_t LogErrorMessage(bool verbose, const wchar_t* fmt, ...) {
  uint32_t retval = 0;
  va_list ap;
  wchar_t err[1024] = {0};

  if (fmt == NULL) return 0;

  if (verbose) {
    va_start(ap, fmt);
    vfwprintf(stdout, fmt, ap);
    fwprintf(stdout, L"\n");
    va_end(ap);
  }

  va_start(ap, fmt);
  retval = vswprintf(err, _countof(err), fmt, ap);
  va_end(ap);
  error_array.push_back(err);

  return retval;
}

std::wstring A2W(const std::string& str) {
  int length_wide = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, NULL, 0);
  wchar_t *string_wide = static_cast<wchar_t*>(_alloca((length_wide * sizeof(wchar_t)) + sizeof(wchar_t)));
  MultiByteToWideChar(CP_ACP, 0, str.data(), -1, string_wide, length_wide);
  std::wstring result(string_wide, length_wide - 1);
  return result;
}

std::string W2A(const std::wstring& str) {
  int length_ansi = WideCharToMultiByte(CP_ACP, 0, str.data(), -1, NULL, 0, NULL, NULL);
  char* string_ansi = static_cast<char*>(_alloca(length_ansi + sizeof(char)));
  WideCharToMultiByte(CP_ACP, 0, str.data(), -1, string_ansi, length_ansi, NULL, NULL);
  std::string result(string_ansi, length_ansi - 1);
  return result;
}


std::wstring FormatLocalTime(time_t datetime) {
  wchar_t buf[64] = {0};
  struct tm* tm = NULL;

  tm = localtime(&datetime);
  wcsftime(buf, _countof(buf) - 1, L"%FT%T%z", tm);

  return std::wstring(buf);
}

bool StartsWithCaseInsensitive(const std::wstring& text, const std::wstring& prefix) {
  return (prefix.empty() ||
    (text.size() >= prefix.size() &&
      std::mismatch(text.begin(), text.end(), prefix.begin(), prefix.end(),
        [](wchar_t first_char, wchar_t second_char) {
          return first_char == second_char || towlower(first_char) == towlower(second_char);
        }).second == prefix.end()));
}

bool GetDictionaryValue(std::string& dict, std::string name,
                        std::string defaultValue, std::string& value) {
  if (std::string::npos != dict.find(name.c_str(), 0)) {
    size_t pos = dict.find(name.c_str(), 0);
    size_t eol = dict.find("\n", pos);
    value = dict.substr(pos + name.size(), eol - (pos + name.size()));
    return true;
  }
  value = defaultValue;
  return false;
}

bool SanitizeContents(std::string& str) {
  auto iter = str.begin();
  while (iter != str.end()) {
    if (*iter == '\r') {
      iter = str.erase(iter);
    } else {
      ++iter;
    }
  }
  return true;
}

bool StripWhitespace(std::string& str) {
  while (1) {
    if (str.length() == 0) break;
    if (!isascii(str[0])) break;
    if (!isspace(str[0])) break;
    str.erase(0, 1);
  }

  int n = (int)str.length();
  while (n > 0) {
    if (!isascii(str[n - 1])) break;
    if (!isspace(str[n - 1])) break;
    n--;
  }
  str.erase(n, str.length() - n);
  return true;
}

bool ExpandEnvironmentVariables(const wchar_t* source, std::wstring& destination) {
  try {
    DWORD dwReserve = ExpandEnvironmentStrings(source, nullptr, 0);
    if (dwReserve == 0) {
      return false;
    }
    destination.resize(dwReserve);
    DWORD dwWritten = ExpandEnvironmentStrings(source, &destination[0],
                                               (DWORD)destination.size());
    if (dwWritten == 0) {
      return false;
    }
    // dwWritten includes the null terminating character
    destination.resize(dwWritten - 1);
  } catch (std::bad_alloc&) {
    return false;
  }
  return true;
}

bool DirectoryExists(std::wstring directory) {
  if (directory.empty()) {
    return false;
  }
  DWORD fileAttr = GetFileAttributes(directory.c_str());
  return (fileAttr != INVALID_FILE_ATTRIBUTES &&
          (fileAttr & FILE_ATTRIBUTE_DIRECTORY));
}

bool IsKnownFileExtension(const std::vector<std::wstring>& exts, const std::wstring& file) {
  for (const auto& ext : exts) {
    if ((file.size() >= ext.size()) &&
      (_wcsicmp(file.substr(file.size() - ext.size()).c_str(), ext.c_str()) == 0))
      return true;
  }
  return false;
}

bool NormalizeDriveName(std::wstring& drive) {
  if ((0 == drive.substr(0, 1).compare(L"\"")) || (0 == drive.substr(0, 1).compare(L"'"))) {
    drive.erase(0, 1);
  }
  if ((0 == drive.substr(drive.size() - 1, 1).compare(L"\"")) || (0 == drive.substr(drive.size() - 1, 1).compare(L"'"))) {
    drive.erase(drive.size() - 1, 1);
  }
  if (0 != drive.substr(drive.size() - 1, 1).compare(L"\\")) {
    drive += L"\\";
  }
  return true;
}

bool NormalizeDirectoryName(std::wstring& dir) {
  if ((0 == dir.substr(0, 1).compare(L"\"")) || (0 == dir.substr(0, 1).compare(L"'"))) {
    dir.erase(0, 1);
  }
  if ((0 == dir.substr(dir.size() - 1, 1).compare(L"\"")) || (0 == dir.substr(dir.size() - 1, 1).compare(L"'"))) {
    dir.erase(dir.size() - 1, 1);
  }
  if (0 != dir.substr(dir.size() - 1, 1).compare(L"\\")) {
    dir += L"\\";
  }
  return true;
}

bool NormalizeFileName(std::wstring& file) {
  if ((0 == file.substr(0, 1).compare(L"\"")) || (0 == file.substr(0, 1).compare(L"'"))) {
    file.erase(0, 1);
  }
  if ((0 == file.substr(file.size() - 1, 1).compare(L"\"")) || (0 == file.substr(file.size() - 1, 1).compare(L"'"))) {
    file.erase(file.size() - 1, 1);
  }
  return true;
}

bool NormalizeFileExtension(std::wstring& ext) {
  if ((0 == ext.substr(0, 1).compare(L"\"")) || (0 == ext.substr(0, 1).compare(L"'"))) {
    ext.erase(0, 1);
  }
  if ((0 == ext.substr(ext.size() - 1, 1).compare(L"\"")) || (0 == ext.substr(ext.size() - 1, 1).compare(L"'"))) {
    ext.erase(ext.size() - 1, 1);
  }
  return true;
}

bool UncompressZIPContentsToString(unzFile zf, std::string& str) {
  int32_t rv = ERROR_SUCCESS;
  char    buf[1024];

  rv = unzOpenCurrentFile(zf);
  if (UNZ_OK == rv) {
    do {
      memset(buf, 0, sizeof(buf));
      rv = unzReadCurrentFile(zf, buf, sizeof(buf));
      if (rv < 0 || rv == 0) break;
      str.append(buf, rv);
    } while (rv > 0);
    unzCloseCurrentFile(zf);
  }

  return true;
}

bool UncompressBZIPContentsToFile(BZFILE* bzf, std::wstring file) {
  int32_t rv = ERROR_SUCCESS;
  HANDLE  h = NULL; 
  DWORD   dwBytesWritten = 0;
  char    buf[1024];

  h = CreateFile(file.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (h != INVALID_HANDLE_VALUE) {
    do {
      memset(buf, 0, sizeof(buf));
      rv = BZ2_bzread(bzf, buf, sizeof(buf));
      if (rv < 0 || rv == 0) break;
      WriteFile(h, buf, rv, &dwBytesWritten, NULL);
    } while (rv > 0);
    CloseHandle(h);
  }

  return (h != INVALID_HANDLE_VALUE);
}

bool UncompressGZIPContentsToFile(gzFile gzf, std::wstring file) {
  int32_t rv = ERROR_SUCCESS;
  HANDLE  h = NULL; 
  DWORD   dwBytesWritten = 0;
  char    buf[1024];

  h = CreateFile(file.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (h != INVALID_HANDLE_VALUE) {
    do {
      memset(buf, 0, sizeof(buf));
      rv = gzread(gzf, buf, sizeof(buf));
      if (rv < 0 || rv == 0) break;
      WriteFile(h, buf, rv, &dwBytesWritten, NULL);
    } while (rv > 0);
    CloseHandle(h);
  }

  return (h != INVALID_HANDLE_VALUE);
}

bool UncompressZIPContentsToFile(unzFile zf, std::wstring file) {
  int32_t rv = ERROR_SUCCESS;
  HANDLE  h = NULL;
  DWORD   dwBytesWritten = 0;
  char    buf[1024];

  h = CreateFile(file.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_TEMPORARY, NULL);
  if (h != INVALID_HANDLE_VALUE) {
    rv = unzOpenCurrentFile(zf);
    if (UNZ_OK == rv) {
      do {
        memset(buf, 0, sizeof(buf));
        rv = unzReadCurrentFile(zf, buf, sizeof(buf));
        if (rv < 0 || rv == 0) break;
        WriteFile(h, buf, rv, &dwBytesWritten, NULL);
      } while (rv > 0);
      unzCloseCurrentFile(zf);
    }
    CloseHandle(h);
  }

  return (h != INVALID_HANDLE_VALUE);
}

std::wstring GetHostName() {
  wchar_t buf[1024] = {0};
  DWORD size = _countof(buf);
  std::wstring hostname;

  if (GetComputerNameEx(ComputerNameDnsFullyQualified, buf, &size)) {
    hostname = buf;
  }

  return hostname;
}

std::wstring GetScanUtilityDirectory() {
  wchar_t path[MAX_PATH] = {0};
  std::wstring utility_dir;
  std::wstring::size_type pos;
  if (GetModuleFileName(NULL, path, _countof(path))) {
    utility_dir = path;
    pos = utility_dir.find_last_of(L"\\");
    utility_dir = utility_dir.substr(0, pos);
  }
  return utility_dir;
}

std::wstring GetReportDirectory() {
  std::wstring destination_dir;
  std::wstring report_dir;
  DWORD dwVersion = 0; 
  DWORD dwMajorVersion = 0;
  DWORD dwMinorVersion = 0; 

  dwVersion = GetVersion();
  dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
  dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

  if ( (dwMajorVersion >= 6) ||
       (dwMajorVersion == 5 && dwMinorVersion >= 3) ) {
    //
    // Windows Vista or Better
    //
    if (ExpandEnvironmentVariables(qualys_program_data_location,
                                   destination_dir)) {
      if (!DirectoryExists(destination_dir.c_str())) {
        _wmkdir(destination_dir.c_str());
      }
      report_dir = destination_dir;
    }
  } else {
    //
    // Windows XP, Windows Server 2003, Windows Server 2003 R2
    //
    if (ExpandEnvironmentVariables(qualys_program_data_legacy_location,
                                   destination_dir)) {
      if (!DirectoryExists(destination_dir.c_str())) {
        _wmkdir(destination_dir.c_str());
      }
      report_dir = destination_dir;
    }
  }

  if (report_dir.empty()) {
    report_dir = GetScanUtilityDirectory();
  }
  return report_dir;
}

std::wstring GetSignatureReportFindingsFilename() {
  return GetReportDirectory() + L"\\" + report_sig_output_file;
}

std::wstring GetSignatureReportSummaryFilename() {
  return GetReportDirectory() + L"\\" + report_sig_summary_file;
}

std::wstring GetCARReportFindingsFilename() {
  return GetReportDirectory() + L"\\car_" + report_sig_output_file;
}

std::wstring GetCARReportSummaryFilename() {
  return GetReportDirectory() + L"\\car_" + report_sig_summary_file;
}

bool ParseVersion(std::wstring version, int& major, int& minor, int& build) {
  return (0 != swscanf_s(version.c_str(), L"%d.%d.%d", &major, &minor, &build));
}

std::wstring GetTempporaryFilename() {
  wchar_t tmpPath[_MAX_PATH + 1];
  wchar_t tmpFilename[_MAX_PATH + 1];

  GetTempPath(_countof(tmpPath), tmpPath);
  GetTempFileName(tmpPath, L"qua", 0, tmpFilename);

  return std::wstring(tmpFilename);
}

int32_t CleanupTemporaryFiles() {
  int32_t         rv = ERROR_SUCCESS;
  WIN32_FIND_DATA FindFileData;
  HANDLE          hFind;
  std::wstring    search;
  std::wstring    filename;
  std::wstring    fullfilename;
  wchar_t         tmpPath[_MAX_PATH + 1];

  GetTempPath(_countof(tmpPath), tmpPath);

  search = tmpPath + std::wstring(L"qua*.tmp");

  hFind = FindFirstFile(search.c_str(), &FindFileData);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      filename = FindFileData.cFileName;

      if ((filename.size() == 1) && (filename == L".")) continue;
      if ((filename.size() == 2) && (filename == L"..")) continue;

      std::wstring fullfilename = std::wstring(tmpPath) + filename;
      DeleteFile(fullfilename.c_str());

    } while (FindNextFile(hFind, &FindFileData));
    FindClose(hFind);
  }  else {
    rv = GetLastError();
  }

  return rv;
}


bool GetFileResourceInfo(std::wstring file, CFileVersionInfo& info) {
  DWORD dwHandle;
  LPVOID lpData;
  DWORD dwSize;
  wchar_t szQuery[256];
  LPVOID lpVar;
  UINT uiVarSize;
  VS_FIXEDFILEINFO* pFileInfo;
  wchar_t szVersionInfo[24];
  wchar_t szFileDescription[256];
  wchar_t szProductName[256];
  wchar_t szFileVersion[256];
  wchar_t szProductVersion[256];
  bool bFileVersionSupported = false;
  bool bFileVersionRetrieved = false;

  struct LANGANDCODEPAGE {
    WORD wLanguage;
    WORD wCodePage;
  } * lpTranslate;

  memset(&szQuery, '\0', sizeof(szQuery));
  memset(&szVersionInfo, '\0', sizeof(szVersionInfo));
  memset(&szFileDescription, '\0', sizeof(szFileDescription));
  memset(&szProductName, '\0', sizeof(szProductName));
  memset(&szFileVersion, '\0', sizeof(szFileVersion));
  memset(&szProductVersion, '\0', sizeof(szProductVersion));

  // Get File Version Information
  //
  dwSize = GetFileVersionInfoSize(file.c_str(), &dwHandle);
  if (dwSize) {
    lpData = (LPVOID)malloc(dwSize);
    if (GetFileVersionInfo(file.c_str(), dwHandle, dwSize, lpData)) {
      bFileVersionRetrieved = true;

      // Which language should be used to lookup the structure?
      wcscpy(szQuery, L"\\VarFileInfo\\Translation");
      VerQueryValue(lpData, szQuery, (LPVOID*)&lpTranslate, &uiVarSize);

      // Version specified as part of the root record.
      if (VerQueryValue(lpData, L"\\", (LPVOID*)&pFileInfo, &uiVarSize)) {
        _snwprintf(szVersionInfo, sizeof(szVersionInfo), L"%d.%d.%d.%d",
                    HIWORD(pFileInfo->dwFileVersionMS),
                    LOWORD(pFileInfo->dwFileVersionMS),
                    HIWORD(pFileInfo->dwFileVersionLS),
                    LOWORD(pFileInfo->dwFileVersionLS));
      }

      // Company Name.
      _snwprintf(szQuery, sizeof(szQuery),
                 L"\\StringFileInfo\\%04x%04x\\FileDescription",
                 lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
      if (VerQueryValue(lpData, szQuery, &lpVar, &uiVarSize)) {
        uiVarSize = _snwprintf(szFileDescription, sizeof(szFileDescription),
                               L"%s", (wchar_t*)lpVar);
        if ((sizeof(szFileDescription) == uiVarSize) || (-1 == uiVarSize)) {
          szFileDescription[255] = '\0';
        }
      }

      // Product Name.
      _snwprintf(szQuery, sizeof(szQuery),
                 L"\\StringFileInfo\\%04x%04x\\ProductName",
                 lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
      if (VerQueryValue(lpData, szQuery, &lpVar, &uiVarSize)) {
        uiVarSize = _snwprintf(szProductName, sizeof(szProductName), L"%s",
                               (wchar_t*)lpVar);
        if ((sizeof(szProductName) == uiVarSize) || (-1 == uiVarSize)) {
          szProductName[255] = '\0';
        }
      }

      // File Version.
      _snwprintf(szQuery, sizeof(szQuery),
                 L"\\StringFileInfo\\%04x%04x\\FileVersion",
                 lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
      if (VerQueryValue(lpData, szQuery, &lpVar, &uiVarSize)) {
        uiVarSize = _snwprintf(szFileVersion, sizeof(szFileVersion), L"%s",
                               (wchar_t*)lpVar);
        if ((sizeof(szFileVersion) == uiVarSize) || (-1 == uiVarSize)) {
          szFileVersion[255] = '\0';
        }
      }

      // Product Version.
      _snwprintf(szQuery, sizeof(szQuery),
                 L"\\StringFileInfo\\%04x%04x\\ProductVersion",
                 lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
      if (VerQueryValue(lpData, szQuery, &lpVar, &uiVarSize)) {
        uiVarSize = _snwprintf(szProductVersion, sizeof(szProductVersion),
                               L"%s", (wchar_t*)lpVar);
        if ((sizeof(szProductVersion) == uiVarSize) || (-1 == uiVarSize)) {
          szProductVersion[255] = '\0';
        }
      }

      free(lpData);
    }
  }

  if (bFileVersionRetrieved) {
    info.productName = szProductName;
    info.productVersion = szProductVersion;
    info.fileDescription = szFileDescription;
    info.fileVersion = szFileVersion;
    return true;
  }

  return false;
}
