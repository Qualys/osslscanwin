#pragma once


#define SAFE_CLOSE_HANDLE(x)                  \
  do {                                        \
    if ((x) && INVALID_HANDLE_VALUE != (x)) { \
      ::CloseHandle(x);                       \
      (x) = INVALID_HANDLE_VALUE;             \
    }                                         \
  } while (FALSE)


#define ARGX3(s1, s2, s3) \
  (!_wcsicmp(argv[i], s1) || !_wcsicmp(argv[i], s2) || !_wcsicmp(argv[i], s3))
#define ARG(S) ARGX3(L"-" #S, L"--" #S, L"/" #S)
#define ARGPARAMCOUNT(X) ((i + X) <= (argc - 1))


LONG CALLBACK CatchUnhandledExceptionFilter(PEXCEPTION_POINTERS pExPtrs);

extern std::vector<std::wstring> error_array;
uint32_t LogErrorMessage(bool verbose, const wchar_t* fmt, ...);


std::wstring A2W(const std::string& str);
std::string W2A(const std::wstring& str);

std::wstring FormatLocalTime(time_t datetime);
bool StartsWithCaseInsensitive(const std::wstring& text, const std::wstring& prefix);
bool GetDictionaryValue(std::string& dict, std::string name, std::string defaultValue, std::string& value);
bool SanitizeContents(std::string& str);
bool StripWhitespace(std::string& str);

bool ExpandEnvironmentVariables(const wchar_t* source, std::wstring& destination);
bool DirectoryExists(std::wstring directory);
bool IsKnownFileExtension(const std::vector<std::wstring>& exts, const std::wstring& file);
bool NormalizeDriveName(std::wstring& drive);
bool NormalizeDirectoryName(std::wstring& dir);
bool NormalizeFileName(std::wstring& file);
bool NormalizeFileExtension(std::wstring& ext);
bool ParseVersion(std::wstring version, int& major, int& minor, int& build);
std::wstring GetTempporaryFilename();
int32_t CleanupTemporaryFiles();

bool UncompressZIPContentsToString(unzFile zf, std::string& str);
bool UncompressBZIPContentsToFile(BZFILE* bzf, std::wstring file); 
bool UncompressGZIPContentsToFile(gzFile gzf, std::wstring file);
bool UncompressZIPContentsToFile(unzFile zf, std::wstring file);

std::wstring GetHostName();
std::wstring GetScanUtilityDirectory();
std::wstring GetReportDirectory();
std::wstring GetSignatureReportFindingsFilename();
std::wstring GetSignatureReportSummaryFilename();
std::wstring GetCARReportFindingsFilename();
std::wstring GetCARReportSummaryFilename();

class CFileVersionInfo {
 public:
  std::wstring productName;
  std::wstring productVersion;
  std::wstring fileDescription;
  std::wstring fileVersion;
};

bool GetFileResourceInfo(std::wstring file, CFileVersionInfo& info);
