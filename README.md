# FDSandbox
A DLL to run processes in a SandBox

## WARNING:
<strong>USE THIS TOOL AS YOUR RISK!</strong>

<strong>THIS TOOL IS A WORKING IN PROGRESS!</strong>

<strong>USING THIS TOOL YOU WILL ACCEPT DI LICENSE OF USE!</strong>

<strong>I assume no responsibility for any ineffectiveness of this Sandbox or any damage it may cause to the operating system or the files contained within it.</strong>

## HOW TO USE IT:
1. Open Visual Studio e select "Create a new project";
2. Create a "CLR Class Library (.NET)" (Classes Libraries CLR).
3. Copy, <strong>CHECK</strong> and Paste the code in c++ project;
4. Select x64 arch;
5. Compile it;
6. Create a new C# project;
7. In dependecies, add Project Reference, add dll;
8. Use this code:
```
using FDSandbox;

static void Monitoring(uint targetPid)
{
    // Create a session to audit Windows Kernel
    using (var session = new TraceEventSession(KernelTraceEventParser.KernelSessionName))
    {
        session.EnableKernelProvider(KernelTraceEventParser.Keywords.FileIO | KernelTraceEventParser.Keywords.Registry | KernelTraceEventParser.Keywords.NetworkTCPIP |
KernelTraceEventParser.Keywords.Process | KernelTraceEventParser.Keywords.Thread | KernelTraceEventParser.Keywords.VirtualAlloc);

        // FILTER FOR FILES
        session.Source.Kernel.FileIOCreate += (data) =>
        {
            if (data.ProcessID == targetPid)
            Console.WriteLine($"[FILE CREATED] {data.FileName}");
        };

            // REGISTER FILTER
        session.Source.Kernel.RegistrySetValue += (data) =>
        {
            if (data.ProcessID == targetPid)
                 Console.WriteLine($"[REGISTER MODIFIED] Chiave: {data.KeyName}");
        };

        // IP FILTER
        session.Source.Kernel.TcpIpConnect += data =>
        {
            if (data.ProcessID == targetPid)
              Console.WriteLine($"TCP Connect: {data.daddr}:{data.dport}");
        };

        // DLL FILTER (IMAGE LOAD)
        session.Source.Kernel.ImageLoad += (data) =>
        {
            if (data.ProcessID == targetPid)
            {
                Console.WriteLine($"[DLL LOADED] {data.FileName} (Base Address: 0x{data.ImageBase:X})");
            }
    };

    session.Source.Kernel.ProcessStart += (data) =>
    {
        if (data.ParentID == targetPid)
        {
            Console.WriteLine($"[ALERT - SON PROCESSO] The malware has run: {data.ProcessName} (New PID: {data.ProcessID})");
        }
    };

    // 2. MONITORING MEMORY (ANALISI ANTI-UNPACKING / INJECTION)
    session.Source.Kernel.VirtualAlloc += (data) =>
    {
        if (data.ProcessID == targetPid)
        {
            Console.WriteLine($"[ALLOCATED MEMORY] Size: {data.Count} byte - Address: 0x{data.Address:X}");
        }
    };

    // 3. MONITORING CREATED THREAD
    session.Source.Kernel.ThreadStart += (data) =>
    {
        if (data.ProcessID == targetPid)
        {
            Console.WriteLine($"[THREAD CREATED] Thread ID: {data.ThreadID} in the malware process");
        }
    };

        // startint events audit
        session.Source.Process();
        }
    }

static void Main(string[] args)
{
    try 
    {
        // 1. Class wrapper
        using(SecureLauncher launcher = new SecureLauncher())
        {
        // 2. Path of suspicious exe (check and sanitize)
            string programPath = args[0];

        // 3. run the process
            int pid = launcher.Launch(programPath);

            if (pid > 0) {
                Console.WriteLine("Process launched in FDSandbox (No Admin)!");
                Task.Run(() => Monitoring(pid));
            } else {
                Console.WriteLine("Error during execution of sandbox.");
            }
        }
        catch (Exception ex) 
        {
            Console.WriteLine($"CRITIC ERROR: {ex.Message}");
        }
    }
}
```
9. In Project select x64 arch;
10. Compile it!

## RELEASE:
I have compiled the release version of this Sandbox. You can download the exe and dll files from the release page.
