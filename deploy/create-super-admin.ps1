param(
    [string]$InstallDir = "C:\OJ\OnlineJudgeDeploy",
    [string]$Username = "admin",
    [string]$Password = "Admin@123456",
    [string]$Email = "admin@example.com",
    [string]$BackendContainer = "oj-backend"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    throw "未找到 docker 命令。请先安装 Docker 并启动 OnlineJudge。"
}

if (-not (Test-Path $InstallDir)) {
    throw "部署目录不存在: $InstallDir"
}

Push-Location $InstallDir
try {
    $running = docker ps --format "{{.Names}}" | Select-String -SimpleMatch $BackendContainer
    if (-not $running) {
        throw "未找到运行中的后端容器 $BackendContainer。请先执行 bootstrap-onlinejudge.ps1 并等待容器启动完成。"
    }

    docker exec $BackendContainer python3 manage.py inituser --username $Username --password $Password --action=reset

    Write-Host ""
    Write-Host "超级管理员已初始化/重置:"
    Write-Host "用户名: $Username"
    Write-Host "密码: $Password"
    Write-Host "邮箱: $Email"
} finally {
    Pop-Location
}
