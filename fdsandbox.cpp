#include <windows.h>
#include <aclapi.h>
#include <vcclr.h> // raccomanded for PtrToStringChars


#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")


using namespace System;


namespace FDSandbox {
    public ref class SecureLauncher {
    private:
        HANDLE hJob = NULL;
        // Internat function that mamages the  native Win32 logic
        UInt32 InternalSetting(LPCWSTR cmd) {
            HANDLE hToken = NULL;
            HANDLE hNewToken = NULL;
            UInt32 pid = 0;

            // 1. opening of the process token
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &hToken)) {
                return -1;
            }


            // 2. getting of Administrators group SID for disabling
            BYTE adminSid[SECURITY_MAX_SID_SIZE];
            DWORD sidSize = sizeof(adminSid);
            CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSid, &sidSize);


            SID_AND_ATTRIBUTES sidsToDisable[1];
            sidsToDisable[0].Sid = (PSID)&adminSid;
            sidsToDisable[0].Attributes = 0; // Deny-only


            // 3. Creation of the Restricted Token
            if (CreateRestrictedToken(
                hToken,
                DISABLE_MAX_PRIVILEGE, // Removing high privileges
                1, sidsToDisable,      // Disabling group Admin
                0, NULL, 
                0, NULL, 
                &hNewToken)) 
            {
                // 4. Configuration of the Job Object for limits
                hJob = CreateJobObject(NULL, NULL);
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };


                // Setting limits: 128MB RAM e forbide of subprocesses creation 
                jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY | 
                                                        JOB_OBJECT_LIMIT_JOB_MEMORY | 
                                                        JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
                jeli.ProcessMemoryLimit = 128 * 1024 * 1024; 
                jeli.JobMemoryLimit = 128 * 1024 * 1024;
                jeli.BasicLimitInformation.ActiveProcessLimit = 1; 


                SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));


                // 5. Preparation of starting process
                STARTUPINFO si = { sizeof(si) };
                PROCESS_INFORMATION pi;


                // Suspend the state because assignement in to the Job before code execution
                if (CreateProcessAsUser(
                    hNewToken, 
                    NULL, 
                    (LPWSTR)cmd, 
                    NULL, NULL, 
                    FALSE, 
                    CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB, 
                    NULL, NULL, 
                    &si, &pi)) 
                {
                    // 6. Assignement in to the Job and resume execution
                    AssignProcessToJobObject(hJob, pi.hProcess);
                    ResumeThread(pi.hThread);
                    pid = pi.dwProcessId;

                    // Cleaning son process handles
                    CloseHandle(pi.hThread);
                    CloseHandle(pi.hProcess);
                }
                
                // Cleaning local handles
                //if (hJob) CloseHandle(hJob);
                CloseHandle(hNewToken);
            }
            
            if (hToken) CloseHandle(hToken);
            return pid;
        }

   public:
        // Il distruttore pulisce il Job Object quando la classe viene rilasciata
        ~SecureLauncher() {
            if (hJob != NULL) {
                CloseHandle(hJob);
                hJob = NULL;
            }
        }
        
    public:
        // pubblic method callable from C#
        UInt32 Launch(String^ commandLine) {
            if (String::IsNullOrEmpty(commandLine)) return -1;


            // Blocca la stringa gestita in memoria per passarla al codice nativo
            pin_ptr<const wchar_t> pinnedCmd = PtrToStringChars(commandLine);
            
            try {
                return InternalSetting(pinnedCmd);
                
            } catch (...) {
                return -1;
            }
        }
    };
}
