
#include "stdafx.h"
#include "Utils.h"
#include "Reports.h"
#include "Scanner.h"


bool IsDriveExcluded(CScannerOptions& options, std::wstring drive) {
  size_t excludedDriveCount = options.excludedDrives.size();
  for (size_t i = 0; i < excludedDriveCount; ++i) {
    if (0 == _wcsnicmp(options.excludedDrives[i].c_str(), drive.c_str(), options.excludedDrives[i].size())) return true;
  }
  return false;
}

bool IsDirectoryExcluded(CScannerOptions& options, std::wstring dir) {
  size_t excludedDirectoryCount = options.excludedDirectories.size();
  for (size_t i = 0; i < excludedDirectoryCount; ++i) {
    if (0 == _wcsnicmp(options.excludedDirectories[i].c_str(), dir.c_str(), options.excludedDirectories[i].size())) return true;
  }
  return false;
}

bool IsFileExcluded(CScannerOptions& options, std::wstring file) {
  size_t excludedFileCount = options.excludedFiles.size();
  for (size_t i = 0; i < excludedFileCount; ++i) {
    if (0 == _wcsnicmp(options.excludedFiles[i].c_str(), file.c_str(), options.excludedFiles[i].size())) return true;
  }
  return false;
}

bool IsOpenSSLArtifact(std::wstring& file) {
  std::vector<std::wstring> exts;
  exts.push_back(L".dll");
  std::transform(file.begin(), file.end(), file.begin(), ::tolower);
  if (IsKnownFileExtension(exts, file)) {
    if (wcsstr(file.c_str(), L"libcrypto") || wcsstr(file.c_str(), L"libssl") ||
        wcsstr(file.c_str(), L"libeay32") || wcsstr(file.c_str(), L"ssleay32")) {
      return true;
    }
  }
  return false;
}

bool IsCVE20223602Mitigated(CFileVersionInfo& info) {
  int major = 0, minor = 0, build = 0;
  if (ParseVersion(info.fileVersion, major, minor, build)) {
    if ((major < 3)) return true;
    if ((major == 3) && (minor == 0) && (build > 6)) return true;
    if ((major == 3) && (minor > 0)) return true;
    if ((major > 3)) return true;
  }
  return false;
}

bool IsCVE20223786Mitigated(CFileVersionInfo& info) {
  int major = 0, minor = 0, build = 0;
  if (ParseVersion(info.fileVersion, major, minor, build)) {
    if ((major < 3)) return true;
    if ((major == 3) && (minor == 0) && (build > 6)) return true;
    if ((major == 3) && (minor > 0)) return true;
    if ((major > 3)) return true;
  }
  return false;
}

int32_t ScanFileZIPArchive(CScannerOptions& options, std::wstring file,
                           std::wstring file_physical) {
  int32_t                   rv = ERROR_SUCCESS;
  unzFile                   zf = NULL;
  unz_file_info64           file_info;
  char*                     p = NULL;
  char                      filename[_MAX_PATH];
  std::wstring              wFilename;
  std::wstring              tmpFilename;

  zlib_filefunc64_def zfm = { 0 };
  fill_win32_filefunc64W(&zfm);

  if (!file_physical.empty()) {
    zf = unzOpen2_64(file_physical.c_str(), &zfm);
  } else {
    zf = unzOpen2_64(file.c_str(), &zfm);
  }
  if (NULL != zf) {
    ReportProcessCompressedFile();

    rv = unzGoToFirstFile(zf);
    if (UNZ_OK == rv) {
      do {
        rv = unzGetCurrentFileInfo64(zf, &file_info, filename, _countof(filename), NULL, 0, NULL, 0);

        if (UNZ_OK == rv) {
          //
          // Add Support for nested OpenSSL file Checks
          //
          wFilename = A2W(filename);
          if (IsOpenSSLArtifact(wFilename)) {
            tmpFilename = GetTempporaryFilename();

            if (UncompressZIPContentsToFile(zf, tmpFilename)) {
              std::wstring masked_filename = file + L"!" + wFilename;
              std::wstring alternate_filename = tmpFilename;

              ScanFile(options, masked_filename, alternate_filename);
            }

            DeleteFile(tmpFilename.c_str());
          }

          //
          // Add Support for nested ZIP files
          //
          wFilename = A2W(filename);
          if (IsKnownFileExtension(options.knownZipExtensions, wFilename)) {
            tmpFilename = GetTempporaryFilename();

            if (UncompressZIPContentsToFile(zf, tmpFilename)) {
              std::wstring masked_filename = file + L"!" + wFilename;
              std::wstring alternate_filename = tmpFilename;

              ScanFileZIPArchive(options, masked_filename, alternate_filename);
            }

            DeleteFile(tmpFilename.c_str());
          }

        }
        rv = unzGoToNextFile(zf);
      } while (UNZ_END_OF_LIST_OF_FILE != rv);
    }
    unzClose(zf);
  }
  rv = ERROR_SUCCESS;
  return rv;
}

int32_t ScanFileTarball(CScannerOptions& options, std::wstring file, std::wstring file_physical) {
  int32_t           rv = ERROR_SUCCESS;
  tarlib::tarFile   tar_file;
  tarlib::tarEntry  tar_entry;
  std::wstring      tmpFilename;

  if (file_physical.size()) {
    tar_file.open(W2A(file_physical.c_str()), tarlib::tarModeRead);
  } else {
    tar_file.open(W2A(file.c_str()), tarlib::tarModeRead);
  }
  if (tar_file.is_open()) {
    ReportProcessTARFile();
    tar_entry = tar_file.get_first_entry();
    do 
    {
      if (tar_entry.header.indicator == tarlib::tarEntryNormalFile) {         
        tmpFilename = GetTempporaryFilename();

        std::wstring masked_filename = file + L"!" + A2W(tar_entry.header.filename);
        std::wstring alternate_filename = tmpFilename;

        if (tar_entry.extractfile_to_file(W2A(alternate_filename.c_str()))) {
          if (IsKnownFileExtension(options.knownZipExtensions, masked_filename.c_str())) {
            ReportProcessCompressedFile();
            ScanFileZIPArchive(options, masked_filename, alternate_filename);
          }
        }

        DeleteFile(tmpFilename.c_str());
      }
      tar_entry = tar_file.get_next_entry();
    } while(!tar_entry.is_empty());
  }

  return rv;
}

int32_t ScanFileCompressedBZIPTarball(CScannerOptions& options, std::wstring file, std::wstring file_physical) {
  int32_t       rv = ERROR_SUCCESS;
  BZFILE*       bzf = NULL;
  std::wstring  tmpFilename;

  if (file_physical.size()) {
    bzf = BZ2_bzopen(W2A(file_physical).c_str(), "rb");
  } else {
    bzf = BZ2_bzopen(W2A(file).c_str(), "rb");
  }
  if (NULL != bzf) {
    ReportProcessCompressedFile();
    tmpFilename = GetTempporaryFilename();

    if (UncompressBZIPContentsToFile(bzf, tmpFilename)) {
      ScanFileTarball(options, file, tmpFilename);
    }

    BZ2_bzclose(bzf);
    DeleteFile(tmpFilename.c_str());
  }

  return rv;
}

int32_t ScanFileCompressedGZIPTarball(CScannerOptions& options, std::wstring file, std::wstring file_physical) {
  int32_t       rv = ERROR_SUCCESS;
  gzFile        gzf = NULL;
  std::wstring  tmpFilename;

  if (file_physical.size()) {
    gzf = gzopen_w(file_physical.c_str(), "rb");
  } else {
    gzf = gzopen_w(file.c_str(), "rb");
  }
  if (NULL != gzf) {
    ReportProcessCompressedFile();
    tmpFilename = GetTempporaryFilename();

    if (UncompressGZIPContentsToFile(gzf, tmpFilename)) {
      ScanFileTarball(options, file, tmpFilename);
    }

    gzclose(gzf);
    DeleteFile(tmpFilename.c_str());
  }

  return rv;
}

int32_t ScanFile(CScannerOptions& options, std::wstring file, std::wstring file_physical) {
  int32_t rv = ERROR_SUCCESS;
  CFileVersionInfo fileVersionInfo;
  std::wstring cveStatus = L"Unknown";
  bool cve20223602Mitigated = false;
  bool cve20223786Mitigated = false;


  // Checking for excluded files
  if (IsFileExcluded(options, file)) return ERROR_NO_MORE_ITEMS;

  if (options.verbose) {
    wprintf(L"Processing File '%s'\n", file.c_str());
  }

  if (0) {
  } else if (IsKnownFileExtension(options.knownZipExtensions, file)) {
    rv = ScanFileZIPArchive(options, file, file_physical);
  } else if (IsKnownFileExtension(options.knownBZipTarExtensions, file)) {
    rv = ScanFileCompressedBZIPTarball(options, file, file_physical);
  } else if (IsKnownFileExtension(options.knownGZipTarExtensions, file)) {
    rv = ScanFileCompressedGZIPTarball(options, file, file_physical);
  } else if (IsKnownFileExtension(options.knownTarExtensions, file)) {
    rv = ScanFileTarball(options, file, file_physical);
  } else if (IsOpenSSLArtifact(file)) {
    if (GetFileResourceInfo(file, fileVersionInfo)) {

      if (wcsstr(fileVersionInfo.productName.c_str(), L"OpenSSL")) {

        if (IsCVE20223602Mitigated(fileVersionInfo)) {
          cve20223602Mitigated = true;
        }
        if (IsCVE20223786Mitigated(fileVersionInfo)) {
          cve20223786Mitigated = true;
        }
        if (cve20223602Mitigated && cve20223786Mitigated) {
          cveStatus = L"Mitigated";
        } else {
          repSummary.foundVunerabilities++;
          cveStatus = L"Potentially Vulnerable ( ";
          if (!cve20223602Mitigated) {
            cveStatus += L"CVE-2022-3602: Found, ";
          } else {
            cveStatus += L"CVE-2022-3602: NOT Found, ";
          }
          if (!cve20223786Mitigated) {
            cveStatus += L"CVE-2022-3786: Found ";
          } else {
            cveStatus += L"CVE-2022-3786: NOT Found ";
          }
          cveStatus += L")";
        }

        if (options.console) {
          wprintf(
              L"OpenSSL Found: '%s' ( Product Name: %s, Product Version: %s, File Description: %s, File Version: %s, CVE Status: %s )\n",
              file.c_str(), fileVersionInfo.productName.c_str(), fileVersionInfo.productVersion.c_str(), fileVersionInfo.fileDescription.c_str(),
              fileVersionInfo.fileVersion.c_str(), cveStatus.c_str());
        }

        repVulns.push_back(CReportVulnerabilities(
            file, fileVersionInfo.productName, fileVersionInfo.productVersion, fileVersionInfo.fileDescription, fileVersionInfo.fileVersion,
            cveStatus, cve20223602Mitigated, cve20223786Mitigated));
      
      }
    }
  }

  return rv;
}

int32_t ScanDirectory(CScannerOptions& options, std::wstring directory, std::wstring directory_physical) {
  int32_t         rv = ERROR_SUCCESS;
  WIN32_FIND_DATA FindFileData;
  HANDLE          hFind;
  std::wstring    search;
  std::wstring    dir;
  std::wstring    dir_phys;
  std::wstring    file;
  std::wstring    file_phys;

  // Checking for excluded directories
  if (IsDirectoryExcluded(options, directory)) return ERROR_NO_MORE_ITEMS;

  if (options.verbose) {
    wprintf(L"Processing Directory '%s'\n", directory.c_str());
  }

  if (directory_physical.size()) {
    search = directory_physical + std::wstring(L"*.*");
  } else {
    search = directory + std::wstring(L"*.*");
  }

  hFind = FindFirstFile(search.c_str(), &FindFileData);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      std::wstring filename(FindFileData.cFileName);

      if ((filename.size() == 1) && (filename == L".")) continue;
      if ((filename.size() == 2) && (filename == L"..")) continue;
      if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) continue;
      if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) == FILE_ATTRIBUTE_DEVICE) continue;
      if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) == FILE_ATTRIBUTE_OFFLINE) continue;
      if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) == FILE_ATTRIBUTE_TEMPORARY) continue;
      if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL) == FILE_ATTRIBUTE_VIRTUAL) continue;

      if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {

        dir = directory + filename + std::wstring(L"\\");
        if (directory_physical.size()) {
          dir_phys = directory_physical + filename + std::wstring(L"\\");
        } else {
          dir_phys.clear();
        }

        ReportProcessDirectory();

        rv = ScanDirectory(options, dir, dir_phys);
        if (ERROR_SUCCESS != rv) {
          LogErrorMessage(options.verbose, L"Failed to process directory '%s' (rv: %d)", dir.c_str(), rv);
        }

      } else {

        file = directory + filename;
        if (directory_physical.size()) {
          file_phys = directory_physical + filename;
        } else {
          file_phys.clear();
        }
        ReportProcessFile();

        rv = ScanFile(options, file, file_phys);
        if (ERROR_SUCCESS != rv) {
          LogErrorMessage(options.verbose, L"Failed to process file '%s' (rv: %d)", file.c_str(), rv);
        }

      }

    } while (FindNextFile(hFind, &FindFileData));
    FindClose(hFind);
  }  else {
    rv = GetLastError();
  }

  return rv;
}

int32_t ScanLocalDrives(CScannerOptions& options) {
  int32_t rv = ERROR_SUCCESS;
  DWORD   rt = 0;
  wchar_t drives[256];

  wcscpy_s(drives, L"");
  rt = GetLogicalDriveStrings(_countof(drives), drives);
  for (uint32_t i = 0; i < rt; i += 4) {
    wchar_t* drive = &drives[i];
    DWORD type = GetDriveType(drive);
    if ((DRIVE_FIXED == type) || (DRIVE_RAMDISK == type)) {

      // Checking for excluded drives
      if (IsDriveExcluded(options, drive)) continue;

      ScanDirectory(options, drive, L"");
    }
  }

  return rv;
}

int32_t ScanNetworkDrives(CScannerOptions& options) {
  int32_t rv = ERROR_SUCCESS;
  DWORD   rt = 0;
  wchar_t drives[256];

  wcscpy_s(drives, L"");
  rt = GetLogicalDriveStrings(_countof(drives), drives);
  for (uint32_t i = 0; i < rt; i += 4) {
    wchar_t* drive = &drives[i];
    DWORD type = GetDriveType(drive);
    if (DRIVE_REMOTE == type) {

      // Checking for excluded drives
      if (IsDriveExcluded(options, drive)) continue;

      ScanDirectory(options, drive, L"");
    }
  }

  return rv;
}

int32_t EnumMountPoints(CScannerOptions& options, std::wstring volume) {
  int32_t rv = ERROR_SUCCESS;
  HANDLE  hFindMountPoint;
  wchar_t mountPoint[MAX_PATH];

  // Find the first mount point.
  hFindMountPoint = FindFirstVolumeMountPoint(volume.c_str(), mountPoint, _countof(mountPoint));

  // If a mount point was found scan it
  if (hFindMountPoint != INVALID_HANDLE_VALUE) {
    ScanDirectory(options, (volume + mountPoint), L"");
  } else {
    if (options.verbose) {
      wprintf(L"No mount points.\n");
    }
    return rv;
  }

  // Find the next mountpoint(s)
  while (FindNextVolumeMountPoint(hFindMountPoint, mountPoint, _countof(mountPoint))) {
    ScanDirectory(options, (volume + mountPoint), L"");
  }

  FindVolumeMountPointClose(hFindMountPoint);
  return rv;
}

int32_t ScanLocalDrivesInclMountpoints(CScannerOptions& options) {
	int32_t rv = ERROR_SUCCESS;
	DWORD   rt = 0;
	wchar_t drives[256];

	wcscpy_s(drives, L"");
	rt = GetLogicalDriveStrings(_countof(drives), drives);
	for (uint32_t i = 0; i < rt; i += 4) {
		wchar_t* drive = &drives[i];
		DWORD type = GetDriveType(drive);
		if ((DRIVE_FIXED == type) || (DRIVE_RAMDISK == type)) {

      // Checking for excluded drives
      if (IsDriveExcluded(options, drive)) continue;

      ScanDirectory(options, drive, L"");

			// Enumerate mount points on the drive and scan them
			EnumMountPoints(options, drive);
		}
	}

	return rv;
}

int32_t ScanPrepareEnvironment(CScannerOptions& options) {
  int32_t rv = ERROR_SUCCESS;
  rv = CleanupTemporaryFiles();
  return rv;
}
