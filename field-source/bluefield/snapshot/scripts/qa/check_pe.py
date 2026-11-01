# SPDX-License-Identifier: GPL-3.0-only
"""Read-only Windows x64 DLL, export ordinal and static CRT checks."""
import argparse,hashlib,json,struct,sys
from pathlib import Path
def inspect(path):
 data=path.read_bytes()
 def unpack(fmt,offset):
  if offset<0 or offset+struct.calcsize(fmt)>len(data):raise ValueError('PE bounds')
  return struct.unpack_from(fmt,data,offset)
 if data[:2]!=b'MZ':raise ValueError('not a PE file')
 pe=unpack('<I',0x3c)[0]
 if data[pe:pe+4]!=b'PE\0\0':raise ValueError('PE signature')
 machine,count,_,_,_,optional_size,flags=unpack('<HHIIIHH',pe+4)
 opt=pe+24
 if machine!=0x8664 or unpack('<H',opt)[0]!=0x20b:raise ValueError('not Windows x64 PE32+')
 if not flags&0x2000:raise ValueError('not a DLL')
 if not 1<=count<=96:raise ValueError('section count')
 dll_flags=unpack('<H',opt+70)[0]
 if dll_flags&0x160!=0x160:raise ValueError('DYNAMICBASE/NX/HIGHENTROPY missing')
 if not dll_flags&0x4000:raise ValueError('Control Flow Guard missing')
 entry=unpack('<I',opt+16)[0]
 sections=[]
 for i in range(count):
  at=opt+optional_size+i*40
  name=data[at:at+8].split(b'\0')[0].decode('ascii',errors='replace')
  virtual_size,rva,raw_size,raw=unpack('<IIII',at+8)
  sections.append({'name':name,'rva':rva,'virtual_size':virtual_size,'raw':raw,'raw_size':raw_size,'flags':unpack('<I',at+36)[0]})
 def file_offset(rva,size=1):
  for section in sections:
   delta=rva-section['rva']
   if 0<=delta and delta+size<=section['raw_size']:
    offset=section['raw']+delta
    if offset+size<=len(data):return offset
  raise ValueError('unmapped RVA')
 def string(rva):
  offset=file_offset(rva);end=data.find(b'\0',offset,min(len(data),offset+4096))
  if end<0:raise ValueError('unterminated PE string')
  return data[offset:end].decode('ascii')
 export_rva,export_size=unpack('<II',opt+112)
 exp=file_offset(export_rva,40)
 _,_,_,_,_,base,functions,names,address_table,name_table,ord_table=unpack('<IIHHIIIIIII',exp)
 if functions>4096 or names>4096:raise ValueError('export count limit')
 exports={}
 for i in range(names):
  name=string(unpack('<I',file_offset(name_table+4*i,4))[0]);index=unpack('<H',file_offset(ord_table+2*i,2))[0]
  if index>=functions:raise ValueError('export ordinal index')
  target=unpack('<I',file_offset(address_table+4*index,4))[0]
  if export_rva<=target<export_rva+export_size:raise ValueError('forwarded application export')
  exports[name]=base+index
 expected={'DllMain':1,'SURGE':2,'DllShutdown':3}
 if exports!=expected or functions!=3:raise ValueError('production ABI mismatch: '+str(exports))
 if not any(s['rva']<=entry<s['rva']+s['virtual_size'] and s['flags']&0x20000000 for s in sections):raise ValueError('entrypoint is not executable')
 imports=[];import_rva,_=unpack('<II',opt+120)
 if import_rva:
  for i in range(256):
   fields=unpack('<IIIII',file_offset(import_rva+20*i,20))
   if not any(fields):break
   imports.append(string(fields[3]).lower())
  else:raise ValueError('unterminated imports')
 forbidden=[n for n in imports if n.startswith(('vcruntime','msvcp','api-ms-win-crt')) or n in ['ucrtbase.dll','ucrtbased.dll']]
 if forbidden:raise ValueError('dynamic CRT imported under /MT policy: '+str(forbidden))
 return {'status':'PASS','scope':'static PE/ABI/CRT audit; no runtime or Golden claim','sha256':hashlib.sha256(data).hexdigest(),'exports':exports,'imports':imports,'machine':'x64','entry_rva':entry,'dll_characteristics':dll_flags,'sections':sections}
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('dll',type=Path);a=ap.parse_args()
 try:result=inspect(a.dll)
 except Exception as e:print(json.dumps({'status':'FAIL','error':str(e)}));return 1
 report=a.dll.parent/'PE_CONTRACT.json';report.write_text(json.dumps(result,indent=2)+'\n');print('PASS PE/ABI/static CRT; report='+str(report));return 0
if __name__=='__main__':sys.exit(main())
