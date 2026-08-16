# test/run_tests.ps1 - 自动化 GUI 冒烟测试（更严谨的验证流程）
# 每个目标按两阶段验证：
#   1) 浸泡: 运行 SoakSeconds 秒, 每秒轮询 2 次, 任何提前退出(崩溃/ASan 报错)即失败
#   2) 关窗: 向所有可见窗口发送 WM_CLOSE, 期待 15s 内干净退出且退出码为 0
# 用法:
#   pwsh test/run_tests.ps1                     # Debug 构建 (out/build/x64-Debug)
#   pwsh test/run_tests.ps1 -Asan               # ASan 构建 (out/build/x64-asan), 检测内存错误
#   pwsh test/run_tests.ps1 -Build              # 先构建再测试
#   pwsh test/run_tests.ps1 -SoakSeconds 20 -Iterations 3
param(
    [switch]$Asan,
    [switch]$Build,
    [int]$SoakSeconds = 8,
    [int]$Iterations = 1
)
$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
$cfg = if ($Asan) { "x64-asan" } else { "x64-Debug" }
$exeDir = Join-Path $repo "out\build\$cfg\test"
$targets = @("test", "test2", "test3")

if (-not (Test-Path $exeDir) -and -not $Build) {
    Write-Error "build dir not found: $exeDir - configure first (or use -Build)"
}

if ($Build) {
    Write-Host "building $cfg ..."
    $vcvars = Get-ChildItem "C:\Program Files\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat" -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $vcvars) { Write-Error "vcvars64.bat not found" }
    $buildDir = Join-Path $repo "out\build\$cfg"
    if (-not (Test-Path (Join-Path $buildDir "build.ninja"))) {
        # 全新构建树：先配置（ASan 树需要 /fsanitize=address）
        Write-Host "configuring $cfg ..."
        $ninja = Get-ChildItem "C:\Program Files\Microsoft Visual Studio\*\*\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
        $cfgFlags = "-DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=`"$ninja`""
        if ($Asan) { $cfgFlags = $cfgFlags + " -DCMAKE_CXX_FLAGS_DEBUG=`"/fsanitize=address /Zi`" -DCMAKE_EXE_LINKER_FLAGS_DEBUG=`"/fsanitize=address /DEBUG`"" }
        $cfgCmd = "call `"$($vcvars.FullName)`" >nul && cmake -B `"$buildDir`" -S `"$repo`" -G Ninja $cfgFlags >nul 2>&1"
        cmd /c $cfgCmd
        if ($LASTEXITCODE -ne 0) { Write-Error "configure failed" }
    }
    $cmd = "call `"$($vcvars.FullName)`" >nul && cmake --build `"$buildDir`" --target $(($targets + "unit_tests") -join " ") >nul 2>&1"
    cmd /c $cmd
    if ($LASTEXITCODE -ne 0) { Write-Error "build failed" }
}

if ($Asan) {
    $rt = Get-ChildItem "C:\Program Files\Microsoft Visual Studio\*\*\VC\Tools\MSVC\*\bin\Hostx64\x64\clang_rt.asan_dynamic-x86_64.dll" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($rt) {
        Copy-Item $rt.FullName $exeDir -Force
        Copy-Item $rt.FullName (Join-Path $repo "out\build\$cfg\test\") -Force
    } else {
        Write-Warning "ASan runtime DLL not found; exes may fail with 0xC0000135"
    }
}

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public class Win32Test {
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lp);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lp);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr wp, IntPtr lp);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll", CharSet = CharSet.Auto)] public static extern int GetWindowText(IntPtr hWnd, StringBuilder sb, int max);
}
"@

function Get-VisibleWindows([int]$procId) {
    $wins = [System.Collections.Generic.List[IntPtr]]::new()
    $cb = [Win32Test+EnumWindowsProc]{ param($h, $lp) $p2 = 0; [Win32Test]::GetWindowThreadProcessId($h, [ref]$p2) | Out-Null; if ($p2 -eq $procId -and [Win32Test]::IsWindowVisible($h)) { $wins.Add($h) }; return $true }
    [Win32Test]::EnumWindows($cb, [IntPtr]::Zero) | Out-Null
    return $wins
}

$failures = 0

# 单元测试（gtest + CTest）
Write-Host ""
Write-Host "=== unit tests (gtest, config=$cfg) ==="
ctest --test-dir (Join-Path $repo "out\build\$cfg\test") --output-on-failure
if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL: unit tests"
    $failures++
}
foreach ($t in $targets) {
    $exe = Join-Path $exeDir "$t.exe"
    if (-not (Test-Path $exe)) { Write-Host "SKIP $t (not built)"; continue }
    for ($iter = 1; $iter -le $Iterations; $iter++) {
        Write-Host ""
        Write-Host "=== ${t} (iteration $iter/$Iterations, config=$cfg) ==="
        $stdout = Join-Path $env:TEMP "imgui_mk_test_$t.out"
        $stderr = Join-Path $env:TEMP "imgui_mk_test_$t.err"
        Remove-Item $stdout, $stderr -ErrorAction SilentlyContinue
        $p = Start-Process $exe -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
        $crashed = $false
        for ($s = 1; $s -le (2 * $SoakSeconds); $s++) {
            Start-Sleep -Milliseconds 500
            if ($p.HasExited) { $crashed = $true; break }
        }
        if ($crashed) {
            $code = "0x{0:X8}" -f $p.ExitCode
            Write-Host "FAIL ${t}: crashed during soak (~$([math]::Ceiling($s / 2))s), code=$code"
            if (Test-Path $stderr) { Get-Content $stderr | Select-Object -Last 8 }
            $failures++
            continue
        }
        Write-Host "OK   ${t}: survived ${SoakSeconds}s soak"
        $wins = Get-VisibleWindows $p.Id
        Write-Host "     closing $($wins.Count) visible window(s)"
        foreach ($w in $wins) { [void][Win32Test]::PostMessage($w, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) }
        if (-not $p.WaitForExit(15000)) {
            Write-Host "FAIL ${t}: did not exit after WM_CLOSE"
            Stop-Process -Id $p.Id -Force
            $failures++
            continue
        }
        if ($p.ExitCode -ne 0) {
            $code = "0x{0:X8}" -f $p.ExitCode
            Write-Host "FAIL ${t}: exit code=$code (expect 0)"
            if (Test-Path $stderr) { Get-Content $stderr | Select-Object -Last 8 }
            $failures++
            continue
        }
        Write-Host "OK   ${t}: clean exit, code 0"
    }
}
Write-Host ""
if ($failures -gt 0) { Write-Host "RESULT: $failures failure(s)"; exit 1 }
Write-Host "RESULT: ALL PASSED"
exit 0
