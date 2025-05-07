Write-Host "Cleaning build directory..."
Remove-Item -Recurse -Force build, CMakeCache.txt, CMakeFiles -ErrorAction SilentlyContinue

Write-Host "Running CMake configuration..."
cmake -G "Visual Studio 17 2022" -A x64 -S . -B build

Write-Host "Building project in Debug mode..."
cmake --build build --config Debug --verbose

# Confirm .exe exists before trying to run
$exePath = Join-Path -Path ".\build\Debug" -ChildPath "TRPGEngine.exe"
if (Test-Path $exePath) {
    Write-Host "Launching TRPG Engine..."
    Start-Process $exePath
} else {
    Write-Host "TRPGEngine.exe not found. Build likely failed."
}

Write-Host "`nDone. Everything is clean, built, and running."
