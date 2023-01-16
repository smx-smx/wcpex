# wcpex
A tool to extract Windows Manifest files that can be found in the WinSxS folder

Compiles in Visual Studio 2022 with cl.exe

Compiling instructions:

* Run x64 Native Tools Command Prompt for VS 2022
* copy wcpex.c to c:\temp\manifest
* cd C:\TEMP\manifest
* "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\bin\Hostx64\x64\cl.exe" "C:\TEMP\manifest\wcpex.c"

Requires a wcp.dll from the servicing stack in the search path or the same directory as the wcpex.exe

* cd c:\windows\winsxs
* dir /s wcp.dll
* copy the latest version to the folder of your choice
* copy "c:\Windows\WinSxS\amd64_microsoft-windows-servicingstack_31bf3856ad364e35_10.0.19041.2300_none_7e14edbc7c88b7d5\wcp.dll" c:\temp\manifest
