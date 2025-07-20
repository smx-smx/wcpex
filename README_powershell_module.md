# Basic decompression to file
Expand-WCPFile -InputFile "C:\Windows\WinSxS\Manifests\example.manifest" -OutputFile "C:\Temp\example.xml"

# Decompress to console (pipeline)
Expand-WCPFile -InputFile "compressed.manifest" | Out-File "decompressed.xml" -Encoding UTF8

# Use specific WCP DLL
Expand-WCPFile -InputFile "file.manifest" -OutputFile "output.xml" -WCPDllPath "C:\Windows\System32\wcp.dll"

# Enable verbose logging
Expand-WCPFile -InputFile "file.manifest" -OutputFile "output.xml" -Verbose