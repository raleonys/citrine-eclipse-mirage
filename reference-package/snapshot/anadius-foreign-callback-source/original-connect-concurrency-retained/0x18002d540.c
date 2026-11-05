/* 0x18002d540 FUN_18002d540; Evidence ev_53566f426581ee5026cc02fda62aabed3b9849f460e2d315566e5aada6038d6a */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 * FUN_18002d540(undefined8 param_1,undefined8 *param_2)

{
  byte *pbVar1;
  LPVOID pvVar2;
  undefined1 auStack_1a8 [40];
  undefined8 *local_180;
  LPVOID local_178 [3];
  ulonglong local_160;
  undefined **local_158 [2];
  undefined1 *local_148;
  undefined1 local_140 [248];
  undefined8 *local_48;
  undefined8 local_40 [3];
  int local_28;
  ulonglong local_18;
  
  local_18 = DAT_1800a1580 ^ (ulonglong)auStack_1a8;
  local_180 = param_2;
  FUN_1800019b0(local_158,0,0,0);
  FUN_180004080((longlong *)local_158,0x180084c58,'\0');
  FUN_180004080((longlong *)local_158,0x180084c5c,'\0');
  FUN_180005560((longlong *)local_158,0x180084c64,&DAT_180084d8c);
  FUN_180004080((longlong *)local_158,0x1800862e0,'\0');
  pbVar1 = (byte *)&DAT_1800a3c98;
  if (0xf < DAT_1800a3cb0) {
    pbVar1 = DAT_1800a3c98;
  }
  FUN_180005560((longlong *)local_158,0x180084cb0,pbVar1);
  FUN_180005560((longlong *)local_158,0x1800862f8,(byte *)"release");
  pbVar1 = (byte *)FUN_180034cf0((longlong *)local_178);
  if (0xf < *(ulonglong *)(pbVar1 + 0x18)) {
    pbVar1 = *(byte **)pbVar1;
  }
  FUN_180005560((longlong *)local_158,0x180084ca8,pbVar1);
  if (0xf < local_160) {
    pvVar2 = local_178[0];
    if ((0xfff < local_160 + 1) &&
       (pvVar2 = *(LPVOID *)((longlong)local_178[0] + -8),
       0x1f < (ulonglong)((longlong)local_178[0] + (-8 - (longlong)pvVar2)))) {
                    /* WARNING: Subroutine does not return */
      FUN_18005c974();
    }
    thunk_FUN_18005caec(pvVar2);
  }
  FUN_180002f20((longlong *)local_158,'\0');
  FUN_180002f20((longlong *)local_158,'\0');
  FUN_180002f20((longlong *)local_158,'\0');
  *param_2 = 0;
  param_2[1] = 0;
  param_2[2] = 0;
  param_2[3] = 0;
  FUN_180006b30(param_2,local_48,(longlong)local_28);
  local_158[0] = tinyxml2::XMLPrinter::vftable;
  if (local_48 != local_40) {
    thunk_FUN_18005caec(local_48);
  }
  if (local_148 != local_140) {
    thunk_FUN_18005caec(local_148);
  }
  return param_2;
}



