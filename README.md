# wcpex
A tool to extract Windows Manifest files that can be found in the WinSxS folder

This fork fixes an issue 0xC0070715 in LoadFirstResourceLanguageAgnostic.
64 Bit version. Compiles in Visual Studio 2022 with cl.exe

Compiling instructions:

* Run x64 Native Tools Command Prompt for VS 2022
* copy wcpex.c to c:\temp\manifest
* cd C:\TEMP\manifest
* "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.34.31933\bin\Hostx64\x64\cl.exe" "C:\TEMP\manifest\wcpex.c"

Requires a wcp.dll from the servicing stack in the search path or the same directory as the wcpex.exe

* cd c:\windows\winsxs
* dir /s wcp.dll
* copy the latest version to the folder of your choice
* copy "C:\Windows\WinSxS\x86_microsoft-windows-servicingstack_31bf3856ad364e35_10.0.19041.2300_none_21f65238c42b469f\wcp.dll" c:\temp\manifest
