@echo off
set GLSLC=C:\VulkanSDK\1.3.296.0\Bin\glslc.exe

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
