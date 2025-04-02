#include <windows.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

// Function to execute PowerShell command and capture the output
int ExecutePowerShellCommand(const char *command, char *outputBuffer, DWORD bufferSize)
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
    int result = ExecutePowerShellCommand(command, ResBuf, sizeof(outputBuffer));

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
