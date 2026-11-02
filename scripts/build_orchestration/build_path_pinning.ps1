# SPDX-License-Identifier: GPL-3.0-only

# Guard only Windows build-root components. The trusted root's ancestors must
# not be writable by untrusted principals. This is not a general filesystem sandbox.
function Enter-PinnedBuildPath {
    param([string]$Root,[string]$Candidate)
    if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT) {
        throw 'Pinned build paths require Windows.'
    }
    $rootPath = [System.IO.Path]::GetFullPath($Root).TrimEnd([System.IO.Path]::DirectorySeparatorChar,[System.IO.Path]::AltDirectorySeparatorChar)
    $candidatePath = [System.IO.Path]::GetFullPath($Candidate)
    $buildRoot = [System.IO.Path]::GetFullPath((Join-Path $rootPath 'build')).TrimEnd([System.IO.Path]::DirectorySeparatorChar,[System.IO.Path]::AltDirectorySeparatorChar)
    if (-not $candidatePath.StartsWith(($buildRoot + [System.IO.Path]::DirectorySeparatorChar),[System.StringComparison]::OrdinalIgnoreCase)) {
        throw 'Pinned build path must be beneath repository build/.'
    }
    if (-not ([System.Management.Automation.PSTypeName]'SurgeDirectoryPinning').Type) {
        Add-Type -ErrorAction Stop -TypeDefinition @'
using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

public static class SurgeDirectoryPinning
{
    [StructLayout(LayoutKind.Sequential)]
    private struct NativeFileTime { public uint Low, High; }
    [StructLayout(LayoutKind.Sequential)]
    private struct NativeFileInformation
    {
        public uint Attributes;
        public NativeFileTime Created, Accessed, Written;
        public uint VolumeSerial, SizeHigh, SizeLow, LinkCount, IndexHigh, IndexLow;
    }

    [DllImport("kernel32.dll", EntryPoint="CreateFileW", CharSet=CharSet.Unicode, SetLastError=true)]
    private static extern SafeFileHandle OpenDirectory(string path, uint access, uint sharing,
        IntPtr security, uint disposition, uint flags, IntPtr templateFile);
    [DllImport("kernel32.dll", EntryPoint="CreateDirectoryW", CharSet=CharSet.Unicode, SetLastError=true)]
    private static extern bool CreateDirectory(string path, IntPtr security);
    [DllImport("kernel32.dll", SetLastError=true)]
    private static extern bool GetFileInformationByHandle(SafeFileHandle handle, out NativeFileInformation information);

    public static void CreateIfMissing(string path)
    {
        if (CreateDirectory(path, IntPtr.Zero)) return;
        int error = Marshal.GetLastWin32Error();
        if (error != 183) // ERROR_ALREADY_EXISTS: inspect using the handle, not Test-Path.
            throw new Win32Exception(error, "Cannot create build path component: " + path);
    }

    public static SafeFileHandle Pin(string path)
    {
        // FILE_READ_ATTRIBUTES; FILE_SHARE_READ|FILE_SHARE_WRITE, not FILE_SHARE_DELETE.
        // OPEN_EXISTING; FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT.
        SafeFileHandle handle = OpenDirectory(path, 0x80, 0x3, IntPtr.Zero, 3, 0x02200000, IntPtr.Zero);
        if (handle == null || handle.IsInvalid)
        {
            int error = Marshal.GetLastWin32Error();
            if (handle != null) handle.Dispose();
            throw new Win32Exception(error, "Cannot open pinned build directory: " + path);
        }
        try
        {
            NativeFileInformation information;
            if (!GetFileInformationByHandle(handle, out information))
                throw new Win32Exception(Marshal.GetLastWin32Error(), "Cannot inspect pinned directory: " + path);
            if ((information.Attributes & 0x400) != 0)
                throw new IOException("Build path component is a reparse point: " + path);
            if ((information.Attributes & 0x10) == 0)
                throw new IOException("Build path component is not a directory: " + path);
            return handle;
        }
        catch
        {
            handle.Dispose();
            throw;
        }
    }
}
'@
    }

    $handles = New-Object 'System.Collections.Generic.List[System.IDisposable]'
    $pendingHandle = $null
    try {
        # The parent of the checkout root is part of the documented trust model.
        $pendingHandle = [SurgeDirectoryPinning]::Pin($rootPath)
        $handles.Add($pendingHandle)
        $pendingHandle = $null
        $current = $rootPath
        $relative = $candidatePath.Substring($rootPath.Length + 1)
        $parts = $relative.Split(
            [char[]]@([System.IO.Path]::DirectorySeparatorChar,[System.IO.Path]::AltDirectorySeparatorChar),
            [System.StringSplitOptions]::RemoveEmptyEntries)
        foreach ($part in $parts) {
            $current = Join-Path $current $part
            # The parent has already been opened without FILE_SHARE_DELETE.
            [SurgeDirectoryPinning]::CreateIfMissing($current)
            $pendingHandle = [SurgeDirectoryPinning]::Pin($current)
            $handles.Add($pendingHandle)
            $pendingHandle = $null
        }
        return ,$handles
    } catch {
        if ($null -ne $pendingHandle) { $pendingHandle.Dispose() }
        Exit-PinnedBuildPath $handles
        throw
    }
}

function Exit-PinnedBuildPath {
    param($Handles)
    if ($null -eq $Handles) { return }
    for ($index = $Handles.Count - 1; $index -ge 0; $index--) {
        $Handles[$index].Dispose()
    }
    $Handles.Clear()
}
