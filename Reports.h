#pragma once

class CReportSummary {
public:
  uint64_t scannedFiles;
  uint64_t scannedDirectories;
  uint64_t scannedCompressed;
  uint64_t scannedJARs;
  uint64_t scannedWARs;
  uint64_t scannedEARs;
  uint64_t scannedTARs;
  uint64_t foundVunerabilities;
  uint64_t scanStart;
  uint64_t scanEnd;
  uint64_t scanErrorCount;
  std::wstring scanStatus;
  std::vector<std::wstring> excludedDrives;
  std::vector<std::wstring> excludedDirectories;
  std::vector<std::wstring> excludedFiles;
  std::vector<std::wstring> knownTarExtensions;
  std::vector<std::wstring> knownGZipTarExtensions;
  std::vector<std::wstring> knownBZipTarExtensions;
  std::vector<std::wstring> knownZipExtensions;

  CReportSummary() {
    scannedFiles = 0;
    scannedDirectories = 0;
    scannedCompressed = 0;
    scannedJARs = 0;
    scannedWARs = 0;
    scannedEARs = 0;
    scannedTARs = 0;
    foundVunerabilities = 0;
    scanStart = 0;
    scanEnd = 0;
    scanErrorCount = 0;
    scanStatus.clear();
    excludedDrives.clear();
    excludedDirectories.clear();
    excludedFiles.clear();
    knownTarExtensions.clear();
    knownGZipTarExtensions.clear();
    knownBZipTarExtensions.clear();
    knownZipExtensions.clear();
  }
};

class CReportVulnerabilities {
 public:
  std::wstring file;
  std::wstring productName;
  std::wstring productVersion;
  std::wstring fileDescription;
  std::wstring fileVersion;
  bool cve20223602Mitigated;
  bool cve20223786Mitigated;
  std::wstring cveStatus;

  CReportVulnerabilities(std::wstring file, std::wstring productName,
                         std::wstring productVersion, std::wstring fileDescription, std::wstring fileVersion,
                         std::wstring cveStatus, bool cve20223602Mitigated,
                         bool cve20223786Mitigated)
  {
    this->file = file;
    this->productName = productName;
    this->productVersion = productVersion;
    this->fileDescription = fileDescription;
    this->fileVersion = fileVersion;
    this->cveStatus = cveStatus;
    this->cve20223602Mitigated = cve20223602Mitigated;
    this->cve20223786Mitigated = cve20223786Mitigated;
  }
};


extern CReportSummary repSummary;
extern std::vector<CReportVulnerabilities> repVulns;

int32_t ReportProcessJARFile();
int32_t ReportProcessWARFile();
int32_t ReportProcessEARFile();
int32_t ReportProcessTARFile();
int32_t ReportProcessCompressedFile();
int32_t ReportProcessDirectory();
int32_t ReportProcessFile();

int32_t GenerateJSONReport(bool pretty);
int32_t GenerateSignatureReport();
int32_t GenerateCARReport();
