param(
    [string]$InstallDir = "C:\OJ\OnlineJudgeDeploy",
    [string]$Username = "admin",
    [string]$Password = "Admin@123456",
    [string]$Email = "admin@example.com",
    [string]$BackendContainer = "oj-backend"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    throw "Command not found: docker. Please install Docker and start OnlineJudge first."
}

if (-not (Test-Path $InstallDir)) {
    throw "Deploy directory does not exist: $InstallDir"
}

Push-Location $InstallDir
try {
    $running = docker ps --format "{{.Names}}" | Select-String -SimpleMatch $BackendContainer
    if (-not $running) {
        throw "Backend container is not running: $BackendContainer. Run bootstrap-onlinejudge.ps1 first and wait for startup."
    }

    $code = @'
import os
from account.models import AdminType, ProblemPermission, User, UserProfile

username = os.environ["GYOJ_ADMIN_USERNAME"]
password = os.environ["GYOJ_ADMIN_PASSWORD"]
email = os.environ.get("GYOJ_ADMIN_EMAIL", "")

user = User.objects.filter(username=username).first()
if user is None:
    user = User.objects.filter(id=1).first()

if user is None:
    user = User.objects.create(
        username=username,
        admin_type=AdminType.SUPER_ADMIN,
        problem_permission=ProblemPermission.ALL,
    )
else:
    user.username = username
    user.admin_type = AdminType.SUPER_ADMIN
    user.problem_permission = ProblemPermission.ALL

if hasattr(user, "email"):
    user.email = email
user.set_password(password)
user.save()
UserProfile.objects.get_or_create(user=user)
print(f"{user.id} {user.username} {getattr(user, 'email', '')} {user.admin_type} {user.problem_permission}")
'@

    $code | docker exec -i `
        -e "GYOJ_ADMIN_USERNAME=$Username" `
        -e "GYOJ_ADMIN_PASSWORD=$Password" `
        -e "GYOJ_ADMIN_EMAIL=$Email" `
        $BackendContainer python3 manage.py shell

    Write-Host ""
    Write-Host "Super admin initialized or reset:"
    Write-Host "Username: $Username"
    Write-Host "Password: $Password"
    Write-Host "Email: $Email"
} finally {
    Pop-Location
}
