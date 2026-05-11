# FDSandbox
A DLL to run processes in a SandBox

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
9. In Project select x64 arch;
10. Compile it!
