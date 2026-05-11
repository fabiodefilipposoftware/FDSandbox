# FDSandbox
A DLL to run processes in a SandBox

## HOW TO USE IT:
1. Create a dll project c++/cli;
2. Copy, <strong>CHECK</strong> and Paste the code in c++ project;
3. Select x64 arch;
4. Compile it;
5. Create a new C# project;
6. In dependecies, add Project Reference, add dll;
7. Use this code:
```
using FDSandbox;

try 
{
    // 1. Class wrapper
    SecureLauncher launcher = new SecureLauncher();

    // 2. Path of suspicious exe
    string programPath = @"C:\Path\suspicious.exe";

    // 3. run the process
    bool result = launcher.Launch(programPath);

    if (result) {
        Console.WriteLine("Process launched cin FDSandbox (No Admin)!");
    } else {
        Console.WriteLine("Error during execution of sandbox.");
    }
}
catch (Exception ex) 
{
    Console.WriteLine($"CRITIC ERROR: {ex.Message}");
}

```
