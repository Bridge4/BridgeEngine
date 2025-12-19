@echo off
set GLSLC=C:\VulkanSDK\1.3.296.0\Bin\glslc.exe
set GLSLC_ALT=C:\VulkanSDK\1.3.250.0\Bin\glslc.exe

:: Check if primary GLSLC exists
if not exist "%GLSLC%" (
    echo Primary GLSLC not found at %GLSLC%
    if exist "%GLSLC_ALT%" (
        echo Falling back to alternate GLSLC at %GLSLC_ALT%
        set GLSLC=%GLSLC_ALT%
    ) else (
        echo ERROR: Neither primary nor alternate GLSLC found.
        pause
        exit /b 1
    )
)

echo Compiling all .vert and .frag shaders in %cd% ...

for %%f in (*.vert *.frag) do (
    echo Compiling %%f ...
    "%GLSLC%" %%f -o %%~nf.spv
    if errorlevel 1 (
        echo Failed to compile %%f
    ) else (
        echo Output: %%~nf.spv
    )
)

echo Done!
pause
