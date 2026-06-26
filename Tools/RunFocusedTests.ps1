[CmdletBinding()]
param(
    [ValidateSet("List", "Run")]
    [string]$Action = "List",

    [string]$Preset = "windows-fast-gate",

    [string]$BuildDir = "",

    [string]$Configuration = "Debug",

    [string[]]$Test = @(),

    [string]$Regex = "",

    [string[]]$Label = @(),

    [string[]]$Module = @(),

    [string[]]$ChangedPath = @(),

    [switch]$Build,

    [string[]]$BuildTarget = @(),

    [switch]$ShowCommand
)

$ErrorActionPreference = "Stop"

function Add-UniqueValue {
    param(
        [System.Collections.Generic.List[string]]$Values,
        [string]$Value
    )

    if ([string]::IsNullOrWhiteSpace($Value)) {
        return
    }

    if ($Values.Contains($Value)) {
        return
    }

    $Values.Add($Value)
}

function ConvertTo-FocusedLabel {
    param(
        [string]$Name
    )

    $trimmed = $Name.Trim()
    if ([string]::IsNullOrWhiteSpace($trimmed)) {
        return ""
    }

    $lower = $trimmed.ToLowerInvariant()
    switch ($lower) {
        "runtimeasset" { return "RuntimeAssetData" }
        "runtimeassetdata" { return "RuntimeAssetData" }
        "rhi" { return "RHI" }
        default { break }
    }

    return $trimmed
}

function Add-ChangedPathLabel {
    param(
        [System.Collections.Generic.List[string]]$Labels,
        [string]$Path
    )

    $normalized = $Path.Replace("\", "/")
    if ($normalized -match "RuntimeAssetDataClosedLoopTests\.cpp$") {
        Add-UniqueValue $Labels "RuntimeAssetData"
        return
    }

    if ($normalized -match "^Src/YuEngine/([^/]+)/") {
        $label = ConvertTo-FocusedLabel $Matches[1]
        Add-UniqueValue $Labels $label
        return
    }

    if ($normalized -match "^Tests/([^/]+)/") {
        $label = ConvertTo-FocusedLabel $Matches[1]
        Add-UniqueValue $Labels $label
        return
    }
}

function Join-EscapedRegex {
    param(
        [string[]]$Values
    )

    $escaped_values = New-Object System.Collections.Generic.List[string]
    foreach ($value in $Values) {
        if ([string]::IsNullOrWhiteSpace($value)) {
            continue
        }

        $escaped_values.Add([regex]::Escape($value.Trim()))
    }

    if ($escaped_values.Count -eq 0) {
        return ""
    }

    return $escaped_values -join "|"
}

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path "build" "$Preset-vs"
}

$labels = New-Object System.Collections.Generic.List[string]
foreach ($item in $Label) {
    Add-UniqueValue $labels $item
}

foreach ($item in $Module) {
    $module_label = ConvertTo-FocusedLabel $item
    Add-UniqueValue $labels $module_label
}

foreach ($path in $ChangedPath) {
    Add-ChangedPathLabel $labels $path
}

$regex_parts = New-Object System.Collections.Generic.List[string]
$test_regex = Join-EscapedRegex $Test
if (![string]::IsNullOrWhiteSpace($test_regex)) {
    $regex_parts.Add("^($test_regex)$")
}

if (![string]::IsNullOrWhiteSpace($Regex)) {
    $regex_parts.Add("($Regex)")
}

if ($regex_parts.Count -eq 0 -and $labels.Count -eq 0) {
    Write-Error "Pass -Test, -Regex, -Label, -Module, or -ChangedPath to keep the run focused." -ErrorAction Continue
    exit 2
}

if ($Build) {
    $build_args = New-Object System.Collections.Generic.List[string]
    $build_args.Add("--build")
    $build_args.Add("--preset")
    $build_args.Add($Preset)
    foreach ($target in $BuildTarget) {
        if ([string]::IsNullOrWhiteSpace($target)) {
            continue
        }

        $build_args.Add("--target")
        $build_args.Add($target)
    }

    if ($ShowCommand) {
        Write-Host ("cmake " + ($build_args -join " "))
    }

    & cmake @build_args
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$ctest_args = New-Object System.Collections.Generic.List[string]
$ctest_args.Add("--test-dir")
$ctest_args.Add($BuildDir)
$ctest_args.Add("-C")
$ctest_args.Add($Configuration)
$ctest_args.Add("--output-on-failure")

if ($Action -eq "List") {
    $ctest_args.Add("-N")
}

if ($regex_parts.Count -gt 0) {
    $ctest_args.Add("-R")
    $ctest_args.Add(($regex_parts -join "|"))
}

if ($labels.Count -gt 0) {
    $label_regex = Join-EscapedRegex $labels.ToArray()
    $ctest_args.Add("-L")
    $ctest_args.Add($label_regex)
}

if ($ShowCommand) {
    Write-Host ("ctest " + ($ctest_args -join " "))
}

& ctest @ctest_args
exit $LASTEXITCODE
