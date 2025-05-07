param(
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug"
)

Write-Host "Cleaning previous build for $BuildType..."
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

Write-Host "`nConfiguring project with CMake ($BuildType)..."
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

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

$exePath = ".\build\$BuildType\TRPGEngine.exe"
if (Test-Path $exePath) {
    Write-Host "`nLaunching engine ($BuildType)..."
    Start-Process $exePath
} else {
    Write-Host "`nBuild succeeded, but EXE not found." -ForegroundColor Yellow
}
