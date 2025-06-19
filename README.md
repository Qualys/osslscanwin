THIS SCRIPT IS PROVIDED TO YOU "AS IS." TO THE EXTENT PERMITTED BY LAW, QUALYS HEREBY DISCLAIMS ALL WARRANTIES AND LIABILITY FOR THE PROVISION OR USE OF THIS SCRIPT. IN NO EVENT SHALL THESE SCRIPTS BE DEEMED TO BE CLOUD SERVICES AS PROVIDED BY QUALYS

# Project Deprecated
Functionality has been rolled into our product offerings, see https://www.qualys.com/apps/software-composition-analysis/ for details.

Project will be removed/deleted on October 1, 2025.

# OSSLScan
## Description
The OSSLScan.exe utility helps to detect CVE-2022-3602 and CVE-2022-3786 vulnerabilities.
The utility will scan the entire hard drive(s) including archives (and nested JARs) for the OpenSSL libraries that indicates the application contains OpenSSL libraries. The utility will output its results to a console.

QID descriptions can be found here:
https://blog.qualys.com/vulnerabilities-threat-research/2022/10/31/qualys-research-alert-prepare-for-a-critical-vulnerability-in-openssl-3-0

Qualys customers should use the following to run the tool on any asset they want to scan, from an elevated command prompt:
> OSSLScan.exe /scan /report_sig

## Usage
```
/scan
  Scan local drives for vulnerable files used by various Java applications.
/scan_network
  Scan network drives for vulnerable files used by various Java applications.
/scan_directory "C:\Some\Path"
  Scan a specific directory for vulnerable files used by various Java applications.
/scan_file "C:\Some\Path\Some.jar"
  Scan a specific file for supported CVE(s).
/scaninclmountpoints
  Scan local drives including mount points for vulnerable files used by various Java applications.
/exclude_drive "C:\"
  Exclude a drive from the scan.
/exclude_directory "C:\Some\Path"
  Exclude a directory from a scan.
/exclude_file "C:\Some\Path\Some.jar"
  Exclude a file from a scan.
/knownTarExtension ".tar"
/knownGZipTarExtension ".tgz"
/knownBZipTarExtension ".tbz"
/knownZipExtension ".jar"
  Add additional file type extensions to the scanner.
/report
  Generate a JSON report of possible detections of supported CVE(s).
/report_pretty
  Generate a human readable JSON report of possible detections of supported CVE(s).
/report_sig
  Generate a signature report of possible detections of supported CVE(s).
/lowpriority
  Lowers the execution and I/O priority of the scanner.
/help
  Displays this help page.
```

Sample Usage (from an elevated command prompt) - The following command helps you scan local drives for vulnerable JAR, WAR, EAR, and ZIP.
> OSSLScan.exe /scan

Sample Usage (from an elevated command prompt) - The following command helps you scan local drives for vulnerable files and writes a signature report to C:\ProgramData\Qualys
> OSSLScan.exe /scan /report_sig

## Output - The following output shows the detection
```
C:\Src\Qualys\OOB\osslscanwin\build\x64\Release\output>OSSLScan.exe /scan_directory C:\Data\OpenSSL
Qualys OpenSSL Vulnerability Scanner 1.0.0.0
https://www.qualys.com/
Dependencies: minizip/1.1, zlib/1.2.11, bzip2/1.0.8, rapidjson/1.1.0
Supported CVE(s): CVE-2022-3602, CVE-2022-3786

Known TAR Extensions            : .tar
Known GZIP TAR Extensions       : .tgz, .tar.gz
Known BZIP TAR Extensions       : .tbz, .tbz2, .tar.bz, .tar.bz2
Known ZIP Extensions            : .zip, .jar, .war, .ear, .par, .kar, .sar, .rar, .jpi, .hpi, .apk

Scanning 'C:\Data\OpenSSL\'...
OpenSSL Found: 'c:\data\openssl\libcrypto-1_1-x64.dll' ( Product Name: The OpenSSL Toolkit, Product Version: 1.1.1n, File Description: OpenSSL library, File Version: 1.1.1n, CVE Status: Mitigated )
OpenSSL Found: 'c:\data\openssl\libssl-1_1-x64.dll' ( Product Name: The OpenSSL Toolkit, Product Version: 1.1.1n, File Description: OpenSSL library, File Version: 1.1.1n, CVE Status: Mitigated )

Scan Summary:
        Scan Date:                       2022-11-01T07:18:52-0700
        Scan Duration:                   0 Seconds
        Scan Error Count:                0
        Scan Status:                     Success
        Files Scanned:                   2
        Directories Scanned:             0
        Compressed File(s) Scanned:      0
        JAR(s) Scanned:                  0
        WAR(s) Scanned:                  0
        EAR(s) Scanned:                  0
        TAR(s) Scanned:                  0
        Vulnerabilities Found:           0
```
