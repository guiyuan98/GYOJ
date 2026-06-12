param(
    [string]$InstallDir = "C:\OJ\OnlineJudgeDeploy",
    [string]$PublicHost = "http://127.0.0.1"
)

$ErrorActionPreference = "Stop"

function Require-Command($Name, $InstallHint) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "未找到命令 $Name。$InstallHint"
    }
}

Require-Command git "请先安装 Git。"
Require-Command docker "请先安装 Docker Desktop 或 Docker Engine，并确认 docker 命令可用。"

$parent = Split-Path -Parent $InstallDir
if (-not (Test-Path $parent)) {
    New-Item -ItemType Directory -Path $parent | Out-Null
}

if (-not (Test-Path $InstallDir)) {
    git clone -b 2.0 https://github.com/QingdaoU/OnlineJudgeDeploy.git $InstallDir
}

Push-Location $InstallDir
try {
    if (Get-Command docker-compose -ErrorAction SilentlyContinue) {
        docker-compose up -d
    } else {
        docker compose up -d
    }

    Write-Host ""
    Write-Host "OnlineJudge 已启动。"
    Write-Host "访问地址: $PublicHost"
    Write-Host "下一步: .\deploy\create-super-admin.ps1 -InstallDir `"$InstallDir`" -Username admin -Password `"Admin@123456`""
} finally {
    Pop-Location
}
