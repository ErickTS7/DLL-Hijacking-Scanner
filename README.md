# DLL-Hijacking-Scanner
Project made using MinHook to hook the API calls "LoadLibraryW", "LoadLibraryA" and "LoadLibraryExW" in kernel32.


Usage for x64 target process:

```DLLScanner_x64.exe <Path_To_File.exe>```


Usage for x86 target process:

```DLLScanner_x86.exe <Path_To_File.exe>```


# Features
## Check whether the DLL is being called using a relative or absolute path.
Possible returns:

\[-\] Absolute Path used -> Not Vulnerable

\[+\] Relative Path used -> Vulnerable

## Checks if the DLL called exists or is missing in the system
Possible returns:

\[-\] DLL exists -> Not Vulnerable

\[-\] DLL exists (LOADED) -> Not Vulnerable

\[+\] DLL doesn't exist -> Vulnerable



