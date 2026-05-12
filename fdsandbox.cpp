#include <windows.h>
#include <aclapi.h>
#include <vcclr.h> // Indispensabile per PtrToStringChars


#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")


using namespace System;


namespace FDSandbox {
    public ref class SecureLauncher {
    private:
        // Funzione interna che gestisce la logica Win32 nativa
        UInt32 InternalSetting(LPCWSTR cmd) {
            HANDLE hToken = NULL;
            HANDLE hNewToken = NULL;
            HANDLE hJob = NULL;
            UInt32 pid = 0;

            // 1. Apertura del token del processo attuale
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &hToken)) {
                return;
            }


            // 2. Recupero del SID del gruppo Administrators per disabilitarlo
            BYTE adminSid[SECURITY_MAX_SID_SIZE];
            DWORD sidSize = sizeof(adminSid);
            CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSid, &sidSize);


            SID_AND_ATTRIBUTES sidsToDisable[1];
            sidsToDisable[0].Sid = (PSID)&adminSid;
            sidsToDisable[0].Attributes = 0; // Impostato a Deny-only


            // 3. Creazione del Restricted Token
            if (CreateRestrictedToken(
                hToken,
                DISABLE_MAX_PRIVILEGE, // Rimuove i privilegi pericolosi
                1, sidsToDisable,      // Disabilita il gruppo Admin
                0, NULL, 
                0, NULL, 
                &hNewToken)) 
            {
                // 4. Configurazione del Job Object per i limiti fisici
                hJob = CreateJobObject(NULL, NULL);
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };


                // Impostazione limiti: 100MB RAM e divieto di creare sottoprocessi
                jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY | 
                                                        JOB_OBJECT_LIMIT_JOB_MEMORY | 
                                                        JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
                jeli.ProcessMemoryLimit = 128 * 1024 * 1024; 
                jeli.JobMemoryLimit = 128 * 1024 * 1024;
                jeli.BasicLimitInformation.ActiveProcessLimit = 1; 


                SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));


                // 5. Preparazione avvio processo
                STARTUPINFO si = { sizeof(si) };
                PROCESS_INFORMATION pi;


                // Lancio in stato sospeso per poterlo inserire nel Job prima che esegua codice
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
                    // 6. Inserimento nel Job e ripristino dell'esecuzione
                    AssignProcessToJobObject(hJob, pi.hProcess);
                    ResumeThread(pi.hThread);
                    pid = pi.dwProcessId;

                    // Pulizia degli handle del processo figlio
                    CloseHandle(pi.hThread);
                    CloseHandle(pi.hProcess);
                }
                
                // Pulizia handle locali
                if (hJob) CloseHandle(hJob);
                CloseHandle(hNewToken);
            }
            
            if (hToken) CloseHandle(hToken);
            return pid;
        }


    public:
        // Metodo pubblico richiamabile da C#
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
