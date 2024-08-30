function Is-Admin() {
    $currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function main() {
    if (-not (Is-Admin)) {
        Write-Host "error: administrator privileges required"
        return 1
    }

    if (Test-Path ".\build\") {
        Remove-Item -Path ".\build\" -Recurse -Force
    }

    mkdir ".\build\"

    # create folder structure
    mkdir ".\build\limit-nvpstate\"

    # build application
    MSBuild.exe ".\limit-nvpstate.sln" -p:Configuration=Release -p:Platform=x64

    # create final package
    Copy-Item ".\x64\Release\limit-nvpstate.exe" ".\build\limit-nvpstate\"
    Copy-Item ".\x64\Release\config.json" ".\build\limit-nvpstate\"

    windeployqt.exe ".\build\limit-nvpstate\limit-nvpstate.exe" --no-system-d3d-compiler --no-translations --no-opengl-sw --no-angle

    return 0
}

$_exitCode = main
Write-Host # new line
exit $_exitCode
