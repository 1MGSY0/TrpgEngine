param( 
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug"
)

Write-Host "Cleaning previous build for $BuildType..."
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

Write-Host "`nConfiguring project with CMake ($BuildType)..."
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=$BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed." -ForegroundColor Red
    exit 1
}

Write-Host "`nBuilding project in $BuildType mode..."
cmake --build build --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed." -ForegroundColor Red
    exit 1
}

# New path based on bin/ output structure
$exePath = ".\build\bin\$BuildType\TRPGEngine.exe"
if (Test-Path $exePath) {
    Write-Host "`nLaunching engine from $exePath..."
    Start-Process $exePath
} else {
    Write-Host "`nBuild succeeded, but EXE not found at expected path:" -ForegroundColor Yellow
    Write-Host " → $exePath"
    Write-Host "Please check if CMAKE_RUNTIME_OUTPUT_DIRECTORY is set in CMakeLists.txt"
}
