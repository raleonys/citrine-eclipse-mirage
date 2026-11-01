# SPDX-License-Identifier: GPL-3.0-only
"""Enumerate owned direct/indirect I/O call sites; vendor internals are separately pinned."""
import hashlib,json,re
from pathlib import Path
# Includes indirect abstract effects and build/script operations. Not a dynamic trace.
PATTERN=re.compile(r'\b(?:CreateFile[AW]?|WriteFile|SetFilePointerEx|FlushFileBuffers|CloseHandle|LockFileEx|UnlockFileEx|GetFileSizeEx|ReplaceFileW|MoveFileExW|GetFileAttributesW|GetFileAttributesExW|GetFileInformationByHandleEx|FindFirstFileW|FindClose|GetModuleFileNameW|GetEnvironmentVariable[AW]|LoadLibraryExW|LoadLibraryW|GetErrorMode|SetErrorMode|FreeLibrary|load_list_\w+|unload_\w+|fopen|freopen_s|ifstream|ofstream|fstream|read_file|write_file|atomic_write|atomic_batch|read_binary_\w+|write_binary_\w+|write_ticket_\w+|write_ini_\w+|write_embedded_png_\w+|store_cipher_key|load_token_\w+|file_hash_matches_\w+|copy_overwrite_\w+|create_directories(?:_\w+)?|create_directory|remove(?:_\w+)?|rename|copy_file|copytree|copy2|copyfile|rmtree|unlink|mkdir|mkstemp|TemporaryDirectory|NamedTemporaryFile|read_bytes|write_bytes|read_text|write_text|open|file_size|status(?:_\w+)?|symlink_status|is_regular_file|exists(?:_\w+)?|directory_iterator|directory_entries|recursive_directory_iterator|temp_directory_path|fsync|flock|fstat|read|write|close|link|avatar|call_create_online|read_binary|persist_achievement|Get-Content|Set-Content|Copy-Item|New-Item|Get-FileHash)\b')
FUNC=re.compile(r'(?:(?:[A-Za-z_]\w*::)+)?([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\([^;]*\)\s*(?:const\s*|noexcept\s*|override\s*|final\s*)*\{')
def scan_calls(root):
 result=[]
 for folder in ['src','include','scripts','cmake']:
  for p in sorted((root/folder).rglob('*')):
   if not p.is_file() or p.suffix not in ['.cpp','.hpp','.h','.c','.py','.ps1','.cmd','.cmake']:continue
   scope='<module>';block=False
   for line,text in enumerate(p.read_text().splitlines(),1):
    stripped=text.strip()
    if stripped.startswith(('//','# ','#"','*')):continue
    matches=FUNC.findall(text)
    if matches:scope=next((x for x in matches if x not in ['if','for','while','switch','catch']),scope)
    if p.suffix=='.py' and stripped.startswith('def '):scope=stripped.split('(')[0][4:]
    apis=sorted(set(PATTERN.findall(text)))
    if not apis:continue
    relative=p.relative_to(root).as_posix()
    owner=relative.split('/')[1] if relative.startswith('src/') else 'build/QA'
    result.append({'source':relative,'line':line,'function_hint':scope,'apis':apis,'code_sha256':hashlib.sha256(text.encode()).hexdigest(),'owner':owner,'classification':'OWNED_CALLSITE','evidence':'static lexical inventory; function_hint is locator, review source for inline/lambda overload scope'})
 # CMake uses commands rather than C++ calls.
 for i,line in enumerate((root/'CMakeLists.txt').read_text().splitlines(),1):
  if re.search(r'\b(?:file|configure_file|add_library|add_executable|add_custom_command)\(',line):result.append({'source':'CMakeLists.txt','line':i,'function_hint':'CMake configure/build','apis':['CMake generation'],'code_sha256':hashlib.sha256(line.encode()).hexdigest(),'owner':'build/QA','classification':'OWNED_CALLSITE','evidence':'static lexical inventory'})
 return result
