param(
    [Parameter(Mandatory=$true)]
    [string]$ConfigFile,

    [Parameter(Mandatory=$true)]
    [ValidateSet('YES','NO')]
    [string]$AutoIncrement
)

$resolvedPath = (Resolve-Path $ConfigFile).Path
[xml]$xml = Get-Content $resolvedPath

$currentVersion = $xml.'play-publishing-config'.application.'version-name'

if ($AutoIncrement -eq 'YES') {
    $v = [version]$currentVersion
    $newVersion = New-Object version($v.Major, $v.Minor, ($v.Build + 1))
    $newVersionString = $newVersion.ToString()
    $xml.'play-publishing-config'.application.'version-name' = $newVersionString
    $xml.Save($resolvedPath)
    Write-Output $newVersionString
} else {
    Write-Output $currentVersion
}
