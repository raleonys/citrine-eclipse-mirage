/* Ghidra 12.1.4 pseudocode export. NOT original source; NOT a compilable C/C++ project. Binary SHA256: 36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846 */
/* Address 180001000; DllMain */

undefined8 DllMain(HMODULE param_1,int param_2)

{
  BOOL BVar1;
  char local_18 [8];
  undefined8 local_10;
  
                    /* 0x1000  1  DllMain */
  if (param_2 == 1) {
    BVar1 = DisableThreadLibraryCalls(param_1);
    if (BVar1 == 0) {
      FUN_1800018ff();
    }
    FUN_180001756((uint *)local_18,(undefined8 *)Process32NextW_exref,FUN_180001229);
    if (local_18[0] == '\0') {
      DAT_1800072c8 = local_10;
    }
    FUN_180001756((uint *)local_18,(undefined8 *)_Mtx_trylock_exref,FUN_180001139);
    if (local_18[0] == '\0') {
      DAT_1800072c0 = local_10;
    }
    FUN_180001756((uint *)local_18,(undefined8 *)CryptQueryObject_exref,FUN_180001264);
    if (local_18[0] == '\0') {
      DAT_1800072d0 = local_10;
    }
    FUN_1800016d3();
    FUN_18000207c(0);
  }
  return 1;
}


/* Address 1800010b8; FUN_1800010b8 */

void FUN_1800010b8(undefined8 *param_1)

{
  undefined8 *puVar1;
  code *pcVar2;
  
  puVar1 = *(undefined8 **)*param_1;
  *(longlong *)*param_1 = 0;
  if (puVar1 != (undefined8 *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800010ca. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    _Mtx_lock(*puVar1);
    return;
  }
                    /* WARNING: Does not return */
  pcVar2 = (code *)invalidInstructionException();
  (*pcVar2)();
}


/* Address 1800010d3; FUN_1800010d3 */

void FUN_1800010d3(undefined8 *param_1,longlong param_2)

{
  undefined8 *puVar1;
  code *pcVar2;
  undefined8 uVar3;
  
  puVar1 = *(undefined8 **)*param_1;
  *(longlong *)*param_1 = 0;
  if (puVar1 == (undefined8 *)0x0) {
                    /* WARNING: Does not return */
    pcVar2 = (code *)invalidInstructionException();
    (*pcVar2)();
  }
  if (*(char *)(param_2 + 4) != '\x01') {
    uVar3 = (*(code *)*puVar1)();
    *puVar1 = uVar3;
    puVar1[1] = param_2;
    return;
  }
  FUN_180004057();
                    /* WARNING: Does not return */
  pcVar2 = (code *)invalidInstructionException();
  (*pcVar2)();
}


/* Address 180001105; FUN_180001105 */

void FUN_180001105(undefined8 *param_1)

{
  undefined8 local_8;
  
  local_8 = *param_1;
  FUN_1800010b8(&local_8);
  return;
}


/* Address 18000111f; FUN_18000111f */

void FUN_18000111f(undefined8 *param_1,longlong param_2)

{
  undefined8 local_8;
  
  local_8 = *param_1;
  FUN_1800010d3(&local_8,param_2);
  return;
}


/* Address 180001139; FUN_180001139 */

void FUN_180001139(undefined8 **param_1)

{
  code *pcVar1;
  undefined1 local_31;
  undefined8 **local_30;
  undefined8 **local_28;
  undefined8 ***local_20;
  undefined4 *local_18;
  undefined1 *local_10;
  
  local_30 = param_1;
  if (DAT_180007010 != 0) {
    local_20 = &local_28;
    local_28 = (undefined8 **)&PTR_FUN_180007000;
    FUN_180003f8f((uint *)&DAT_180007010,'\x01',&local_20,0x180005298);
  }
  if ((((PTR_FUN_180007000 <= param_1) && (param_1 < DAT_180007008)) && (DAT_180007018 != 0)) &&
     (DAT_180007018 != 0)) {
    local_20 = &local_30;
    local_28 = &local_20;
    local_18 = &DAT_18000701c;
    local_10 = &local_31;
    FUN_180003f8f((uint *)&DAT_180007018,'\x01',&local_28,0x180005270);
  }
  if (DAT_1800072c0 != (code *)0x0) {
    (*DAT_1800072c0)(param_1);
    return;
  }
                    /* WARNING: Does not return */
  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}


/* Address 180001201; FUN_180001201 */

void FUN_180001201(void)

{
  code *pcVar1;
  HMODULE pHVar2;
  
  pHVar2 = GetModuleHandleA((LPCSTR)0x0);
  if (pHVar2 != (HMODULE)0x0) {
    return;
  }
  FUN_1800018ff();
                    /* WARNING: Does not return */
  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}


/* Address 180001229; FUN_180001229 */

int FUN_180001229(undefined8 param_1,longlong param_2)

{
  code *pcVar1;
  int iVar2;
  DWORD DVar3;
  
  if (DAT_1800072c8 != (code *)0x0) {
    iVar2 = (*DAT_1800072c8)();
    if ((param_2 != 0) && (iVar2 != 0)) {
      DVar3 = GetCurrentProcessId();
      if (*(DWORD *)(param_2 + 8) == DVar3) {
        *(DWORD *)(param_2 + 0x20) = DVar3;
      }
    }
    return iVar2;
  }
                    /* WARNING: Does not return */
  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}


/* Address 180001264; FUN_180001264 */

void FUN_180001264(int param_1,wchar_t *param_2,undefined4 param_3,undefined4 param_4)

{
  wchar_t *pwVar1;
  byte *pbVar2;
  byte bVar3;
  wchar_t wVar4;
  code *pcVar5;
  undefined1 uVar6;
  uint uVar7;
  int iVar8;
  byte *pbVar9;
  byte *pbVar10;
  ulonglong uVar11;
  ulonglong uVar12;
  undefined1 uVar13;
  uint uVar14;
  wchar_t *pwVar15;
  undefined1 uVar16;
  byte *pbVar17;
  byte *pbVar18;
  byte *pbVar19;
  bool bVar20;
  byte *pbVar21;
  uint local_70;
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  byte *local_60;
  byte *local_58;
  byte *local_50;
  byte local_48;
  undefined7 uStack_47;
  
  if (param_1 == 1) {
    if (param_2 != (wchar_t *)0x0) {
      pbVar21 = (byte *)0xffffffffffffffff;
      do {
        pbVar10 = pbVar21 + 1;
        pbVar21 = pbVar21 + 1;
      } while (param_2[(longlong)pbVar10] != L'\0');
      local_6c = param_1;
      local_68 = param_3;
      local_64 = param_4;
      if (pbVar21 == (byte *)0x0) {
        pbVar9 = (byte *)0x1;
        pbVar10 = (byte *)0x0;
      }
      else {
        pbVar9 = (byte *)FUN_1800015ff(1,(SIZE_T)pbVar21);
        pbVar10 = pbVar21;
        if (pbVar9 == (byte *)0x0) goto LAB_1800015fd;
      }
      local_50 = (byte *)0x0;
      pwVar1 = param_2 + (longlong)pbVar21;
      local_48 = 1;
      bVar20 = false;
      uVar14 = 1;
      pwVar15 = param_2;
      local_60 = pbVar10;
      local_58 = pbVar9;
      do {
        uVar7 = uVar14;
        if (!bVar20) {
          if (pwVar15 != pwVar1) {
            wVar4 = *pwVar15;
            pwVar15 = pwVar15 + 1;
            uVar7 = (uint)(ushort)wVar4;
            goto LAB_180001320;
          }
LAB_1800014cd:
          pbVar21 = local_50;
          pbVar10 = local_60;
          pbVar9 = local_58;
          if ((local_48 & 1) != 0) goto LAB_1800014ea;
          pbVar2 = local_58 + (longlong)local_50;
          pbVar19 = local_58;
          goto LAB_180001572;
        }
LAB_180001320:
        if (((ushort)uVar7 & 0xf800) == 0xd800) {
          if (0xdbff < (ushort)uVar7) {
            uVar11 = (ulonglong)(uVar7 << 0x10) + 1;
            goto LAB_180001344;
          }
          if (pwVar15 == pwVar1) {
            uVar11 = (ulonglong)(uVar7 << 0x10) + 1;
            bVar20 = false;
            pwVar15 = pwVar1;
          }
          else {
            wVar4 = *pwVar15;
            if ((ushort)(wVar4 + L'\x2000') < 0xfc00) {
              uVar11 = (ulonglong)(uVar7 << 0x10) + 1;
              bVar20 = true;
              pwVar15 = pwVar15 + 1;
              uVar14 = (uint)(ushort)wVar4;
            }
            else {
              uVar11 = ((ulonglong)((ushort)wVar4 & 0x3ff) << 0x20 |
                       (ulonglong)(uVar7 & 0x3ff) << 0x2a) + 0x1000000000000;
              bVar20 = false;
              pwVar15 = pwVar15 + 1;
            }
          }
        }
        else {
          uVar11 = (ulonglong)uVar7 << 0x20;
LAB_180001344:
          bVar20 = false;
        }
        if ((short)uVar11 == -1) goto LAB_1800014cd;
        if ((uVar11 & 1) == 0) {
          local_70 = (uint)(uVar11 >> 0x20);
        }
        else {
          local_70 = (uint)(uVar11 >> 0x10) & 0xffff;
          local_48 = 0;
        }
        uVar6 = (undefined1)local_70;
        if (local_70 < 0x80) {
          local_70 = local_70 & 0xff;
          uVar11 = 1;
        }
        else if (local_70 < 0x800) {
          local_70 = (ushort)(CONCAT11(uVar6,(char)(local_70 >> 6)) | 0xc0) & 0xffff3fff | 0x8000;
          uVar11 = 2;
        }
        else {
          uVar11 = 4 - (ulonglong)(local_70 < 0x10000);
          uVar13 = (undefined1)(local_70 >> 6);
          uVar16 = (undefined1)(local_70 >> 0xc);
          if (local_70 < 0x10000) {
            local_70 = (CONCAT12(uVar6,CONCAT11(uVar13,uVar16)) & 0xff3fff | 0x80e0) & 0xff3fffff |
                       0x800000;
          }
          else {
            local_70 = CONCAT13(uVar6,CONCAT12(uVar13,CONCAT11(uVar16,(char)(local_70 >> 0x12)))) &
                       0x3f3f3fff | 0x808080f0;
          }
        }
        if (((ulonglong)((longlong)local_60 - (longlong)local_50) < uVar11) &&
           (uVar12 = FUN_18000166f((ulonglong *)&local_60,(ulonglong)local_50,uVar11),
           pbVar9 = local_58, uVar12 != 0xffffffffffffffff)) goto LAB_1800015fd;
        pbVar21 = local_50;
        memcpy(pbVar9 + (longlong)local_50,&local_70,uVar11);
        local_50 = pbVar21 + uVar11;
      } while( true );
    }
    param_2 = L"EAAntiCheat.GameServiceLauncher.dll";
  }
  goto LAB_18000153a;
LAB_180001572:
  do {
    pbVar17 = pbVar19;
    if (pbVar17 == pbVar2) goto LAB_1800014ea;
    pbVar18 = pbVar17 + 1;
    bVar3 = *pbVar17;
    pbVar19 = pbVar18;
  } while (-1 < (char)bVar3);
  if (bVar3 < 0xe0) {
    if (pbVar18 != pbVar2) {
      pbVar19 = pbVar17 + 2;
    }
    goto LAB_180001572;
  }
  if (bVar3 != 0xed) {
    pbVar19 = pbVar17 + 2;
    if (pbVar18 == pbVar2) {
      pbVar19 = pbVar18;
    }
    pbVar19 = pbVar19 + (pbVar19 != pbVar2);
    if (0xef < bVar3) {
      pbVar19 = pbVar19 + (pbVar19 != pbVar2);
    }
    goto LAB_180001572;
  }
  pbVar19 = pbVar2;
  if (((pbVar18 == pbVar2) || (pbVar17 + 2 == pbVar2)) || (pbVar19 = pbVar17 + 3, pbVar17[1] < 0xa0)
     ) goto LAB_180001572;
  pbVar21 = (byte *)CONCAT71(uStack_47,local_48);
  pbVar10 = local_58;
  pbVar9 = local_50;
  if (local_60 != (byte *)0xffffffffffffffff) goto LAB_1800015fd;
LAB_1800014ea:
  if (((byte *)0xe < pbVar21) &&
     (iVar8 = memcmp("preloader_l.dll",pbVar9 + (longlong)pbVar21 + -0xf,0xf), iVar8 == 0)) {
    param_2 = L"EAAntiCheat.GameServiceLauncher.dll";
  }
  param_4 = local_64;
  param_3 = local_68;
  param_1 = local_6c;
  FUN_180001623((longlong)pbVar10,pbVar9,1,1);
LAB_18000153a:
  if (DAT_1800072d0 != (code *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001565. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*DAT_1800072d0)(param_1,param_2,param_3,param_4);
    return;
  }
LAB_1800015fd:
                    /* WARNING: Does not return */
  pcVar5 = (code *)invalidInstructionException();
  (*pcVar5)();
}


/* Address 1800015ff; FUN_1800015ff */

void FUN_1800015ff(ulonglong param_1,SIZE_T param_2)

{
  if (param_2 != 0) {
    FUN_180001886(param_1,param_2);
  }
  return;
}


/* Address 180001623; FUN_180001623 */

void FUN_180001623(longlong param_1,LPVOID param_2,ulonglong param_3,longlong param_4)

{
  ulonglong *puVar1;
  HANDLE hHeap;
  longlong lVar2;
  ulonglong local_10;
  longlong in_stack_fffffffffffffff8;
  
  puVar1 = &local_10;
  if (param_1 == 0) {
    lVar2 = 0;
  }
  else {
    lVar2 = param_1 * param_4;
    puVar1 = (ulonglong *)&stack0xfffffffffffffff8;
    local_10 = param_3;
  }
  *puVar1 = lVar2;
  if (local_10 != 0) {
    if (in_stack_fffffffffffffff8 == 0) {
      return;
    }
    if (0x10 < local_10) {
      param_2 = *(LPVOID *)((longlong)param_2 + -8);
    }
    hHeap = GetProcessHeap();
                    /* WARNING: Could not recover jumptable at 0x0001800017f8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    HeapFree(hHeap,0,param_2);
    return;
  }
  return;
}


/* Address 18000166f; FUN_18000166f */

ulonglong FUN_18000166f(ulonglong *param_1,ulonglong param_2,ulonglong param_3)

{
  ulonglong uVar1;
  SIZE_T SVar2;
  char local_30 [8];
  ulonglong local_28;
  
  if (CARRY8(param_2,param_3)) {
    local_28 = 0;
  }
  else {
    uVar1 = *param_1 * 2;
    if (uVar1 < param_2 + param_3) {
      uVar1 = param_2 + param_3;
    }
    SVar2 = 8;
    if (8 < uVar1) {
      SVar2 = uVar1;
    }
    FUN_180003ef6((undefined8 *)local_30,*param_1,(LPVOID)param_1[1],SVar2);
    if (local_30[0] == '\0') {
      param_1[1] = local_28;
      *param_1 = SVar2;
      local_28 = 0xffffffffffffffff;
    }
  }
  return local_28;
}


/* Address 1800016d3; FUN_1800016d3 */

void FUN_1800016d3(void)

{
  undefined1 local_9;
  undefined1 *local_8;
  
  if (DAT_18000701c != 0) {
    local_8 = &local_9;
    local_9 = 1;
    FUN_180003f8f((uint *)&DAT_18000701c,'\0',&local_8,0x180005318);
  }
  return;
}


/* Address 18000170d; FUN_18000170d */

void FUN_18000170d(undefined8 *param_1)

{
  undefined8 local_8;
  
  local_8 = *param_1;
  FUN_180001727(&local_8);
  return;
}


/* Address 180001727; FUN_180001727 */

void FUN_180001727(undefined8 *param_1)

{
  char cVar1;
  code *pcVar2;
  undefined8 uVar3;
  int iVar4;
  
  cVar1 = *(char *)*param_1;
  *(char *)*param_1 = '\0';
  if (cVar1 != '\0') {
    uVar3 = FUN_180002088();
    iVar4 = -2;
    if ((int)uVar3 != 0) {
      iVar4 = (int)uVar3;
    }
    if ((iVar4 == -2) || (iVar4 == 1)) {
      return;
    }
  }
                    /* WARNING: Does not return */
  pcVar2 = (code *)invalidInstructionException();
  (*pcVar2)();
}


/* Address 180001756; FUN_180001756 */

void FUN_180001756(uint *param_1,undefined8 *param_2,LPCVOID param_3)

{
  uint uVar1;
  undefined8 local_20;
  
  FUN_1800016d3();
  local_20 = 0;
  uVar1 = FUN_180001ec4(param_2,param_3,&local_20);
  if (uVar1 == 0) {
    *(undefined8 *)(param_1 + 2) = local_20;
  }
  else {
    param_1[1] = uVar1;
  }
  *param_1 = (uint)(uVar1 != 0);
  return;
}


/* Address 1800017a2; FUN_1800017a2 */

void FUN_1800017a2(undefined8 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 local_4;
  
  local_4 = param_2;
  iVar1 = WaitOnAddress(param_1,&local_4,4,0xffffffff);
  if (iVar1 != 1) {
    GetLastError();
  }
  return;
}


/* Address 1800017d3; FUN_1800017d3 */

void FUN_1800017d3(LPVOID param_1,ulonglong param_2)

{
  HANDLE hHeap;
  
  if (0x10 < param_2) {
    param_1 = *(LPVOID *)((longlong)param_1 + -8);
  }
  hHeap = GetProcessHeap();
                    /* WARNING: Could not recover jumptable at 0x0001800017f8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  HeapFree(hHeap,0,param_1);
  return;
}


/* Address 1800017ff; FUN_1800017ff */

LPVOID FUN_1800017ff(LPVOID param_1,ulonglong param_2,ulonglong param_3,ulonglong param_4)

{
  HANDLE hHeap;
  LPVOID pvVar1;
  
  if (param_2 < 0x11) {
    hHeap = GetProcessHeap();
                    /* WARNING: Could not recover jumptable at 0x000180001832. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    pvVar1 = HeapReAlloc(hHeap,0,param_1,param_4);
    return pvVar1;
  }
  pvVar1 = FUN_180001886(param_2,param_4);
  if (pvVar1 == (LPVOID)0x0) {
    pvVar1 = (LPVOID)0x0;
  }
  else {
    if (param_4 < param_3) {
      param_3 = param_4;
    }
    memcpy(pvVar1,param_1,param_3);
    FUN_1800017d3(param_1,param_2);
  }
  return pvVar1;
}


/* Address 180001886; FUN_180001886 */

LPVOID FUN_180001886(ulonglong param_1,SIZE_T param_2)

{
  LPVOID pvVar1;
  LPVOID pvVar2;
  longlong lVar3;
  
  if (param_1 < 0x11) {
    pvVar1 = FUN_1800018d0(param_2);
    return pvVar1;
  }
  pvVar1 = FUN_1800018d0(param_2 + param_1);
  if (pvVar1 == (LPVOID)0x0) {
    pvVar2 = (LPVOID)0x0;
  }
  else {
    lVar3 = param_1 - (param_1 - 1 & (ulonglong)pvVar1);
    pvVar2 = (LPVOID)((longlong)pvVar1 + lVar3);
    *(LPVOID *)((longlong)pvVar1 + lVar3 + -8) = pvVar1;
  }
  return pvVar2;
}


/* Address 1800018d0; FUN_1800018d0 */

LPVOID FUN_1800018d0(SIZE_T param_1)

{
  HANDLE hHeap;
  LPVOID pvVar1;
  
  hHeap = GetProcessHeap();
  if (hHeap != (HANDLE)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800018f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    pvVar1 = HeapAlloc(hHeap,0,param_1);
    return pvVar1;
  }
  return (LPVOID)0x0;
}


/* Address 1800018ff; FUN_1800018ff */

DWORD FUN_1800018ff(void)

{
  DWORD DVar1;
  
  DVar1 = GetLastError();
  if (0 < (int)DVar1) {
    DVar1 = DVar1 & 0xffff | 0x80070000;
  }
  if (DVar1 == 0) {
    DVar1 = 0x535f4f4b;
  }
  return DVar1;
}


/* Address 180001930; FUN_180001930 */

LPVOID FUN_180001930(void)

{
  LPVOID pvVar1;
  ulonglong uVar2;
  
  if (DAT_1800072e8 == (LPVOID)0x0) {
    DAT_1800072f0 = 0x20;
    DAT_1800072e8 = HeapAlloc(DAT_1800072e0,0,0x700);
    if (DAT_1800072e8 == (LPVOID)0x0) {
      return (LPVOID)0x0;
    }
  }
  else if (DAT_1800072f0 <= DAT_1800072f4) {
    pvVar1 = HeapReAlloc(DAT_1800072e0,0,DAT_1800072e8,(ulonglong)(DAT_1800072f0 * 2) * 0x38);
    if (pvVar1 == (LPVOID)0x0) {
      return (LPVOID)0x0;
    }
    DAT_1800072f0 = DAT_1800072f0 * 2;
    DAT_1800072e8 = pvVar1;
  }
  uVar2 = (ulonglong)DAT_1800072f4;
  DAT_1800072f4 = DAT_1800072f4 + 1;
  return (LPVOID)(uVar2 * 0x38 + (longlong)DAT_1800072e8);
}


/* Address 1800019d4; FUN_1800019d4 */

ulonglong FUN_1800019d4(uint param_1)

{
  uint uVar1;
  ulonglong uVar2;
  byte *pbVar3;
  uint uVar4;
  ulonglong uVar5;
  longlong local_18 [2];
  
  uVar4 = 0;
  if (DAT_1800072f4 != 0) {
    pbVar3 = (byte *)(DAT_1800072e8 + 0x20);
    do {
      if ((*pbVar3 >> 1 & 1) != param_1) {
        if (uVar4 == 0xffffffff) {
          return 0;
        }
        uVar2 = FUN_180001cdc(local_18,0xffffffff,(uint)(param_1 != 0));
        uVar5 = uVar2 & 0xffffffff;
        uVar1 = DAT_1800072f4;
        if ((int)uVar2 != 0) {
          return uVar5;
        }
        goto LAB_180001a78;
      }
      uVar4 = uVar4 + 1;
      pbVar3 = pbVar3 + 0x38;
    } while (uVar4 < DAT_1800072f4);
  }
  return 0;
LAB_180001a78:
  if (uVar1 <= uVar4) {
LAB_180001a7c:
    FUN_180002280(local_18);
    return uVar5;
  }
  if ((*(byte *)((ulonglong)uVar4 * 0x38 + 0x20 + DAT_1800072e8) >> 1 & 1) != param_1) {
    uVar2 = FUN_180001b78((ulonglong)uVar4,param_1);
    uVar5 = uVar2 & 0xffffffff;
    uVar1 = DAT_1800072f4;
    if ((int)uVar2 != 0) goto LAB_180001a7c;
  }
  uVar4 = uVar4 + 1;
  goto LAB_180001a78;
}


/* Address 180001a98; FUN_180001a98 */

ulonglong FUN_180001a98(longlong param_1,uint param_2)

{
  ulonglong uVar1;
  longlong *plVar2;
  ulonglong uVar3;
  uint uVar4;
  ulonglong uVar5;
  longlong local_18 [2];
  
  FUN_180001c9c();
  uVar5 = 0;
  if (DAT_1800072e0 == 0) {
    uVar1 = 2;
  }
  else if (param_1 == 0) {
    uVar1 = FUN_1800019d4(param_2);
    uVar1 = uVar1 & 0xffffffff;
  }
  else {
    plVar2 = DAT_1800072e8;
    if (DAT_1800072f4 != 0) {
      do {
        uVar4 = (uint)uVar5;
        if (param_1 == *plVar2) {
          if (uVar4 != 0xffffffff) {
            if ((*(byte *)(DAT_1800072e8 + uVar5 * 7 + 4) >> 1 & 1) == param_2) {
              uVar1 = (ulonglong)(6 - (param_2 != 0));
            }
            else {
              uVar3 = FUN_180001cdc(local_18,uVar4,1);
              uVar1 = uVar3 & 0xffffffff;
              if ((int)uVar3 == 0) {
                uVar1 = FUN_180001b78(uVar5,param_2);
                uVar1 = uVar1 & 0xffffffff;
                FUN_180002280(local_18);
              }
            }
            goto LAB_180001b5b;
          }
          break;
        }
        uVar5 = (ulonglong)(uVar4 + 1);
        plVar2 = plVar2 + 7;
      } while (uVar4 + 1 < DAT_1800072f4);
    }
    uVar1 = 4;
  }
LAB_180001b5b:
  LOCK();
  DAT_1800072d8 = 0;
  UNLOCK();
  return uVar1;
}


/* Address 180001b78; FUN_180001b78 */

undefined8 FUN_180001b78(ulonglong param_1,int param_2)

{
  SIZE_T dwSize;
  undefined1 *puVar1;
  longlong lVar2;
  BOOL BVar3;
  undefined8 uVar4;
  HANDLE hProcess;
  byte bVar5;
  undefined1 *lpAddress;
  longlong lVar6;
  size_t _Size;
  undefined1 *puVar7;
  DWORD local_res8 [2];
  
  lVar2 = DAT_1800072e8;
  lVar6 = (param_1 & 0xffffffff) * 0x38;
  puVar1 = *(undefined1 **)(lVar6 + DAT_1800072e8);
  bVar5 = *(byte *)(lVar6 + 0x20 + DAT_1800072e8) & 1;
  puVar7 = puVar1;
  if (bVar5 != 0) {
    puVar7 = puVar1 + -5;
  }
  lpAddress = puVar1 + -5;
  if (bVar5 == 0) {
    lpAddress = puVar1;
  }
  dwSize = (ulonglong)bVar5 * 2 + 5;
  BVar3 = VirtualProtect(lpAddress,dwSize,0x40,local_res8);
  if (BVar3 == 0) {
    uVar4 = 10;
  }
  else {
    if (param_2 == 0) {
      _Size = 7;
      if ((*(byte *)(lVar6 + 0x20 + lVar2) & 1) == 0) {
        _Size = 5;
      }
      memcpy(lpAddress,(void *)(lVar2 + 0x18 + lVar6),_Size);
    }
    else {
      *lpAddress = 0xe9;
      *(int *)(lpAddress + 1) = (*(int *)(lVar6 + 8 + lVar2) - (int)puVar7) + -5;
      if ((*(byte *)(lVar6 + 0x20 + lVar2) & 1) != 0) {
        **(undefined2 **)(lVar6 + lVar2) = 0xf9eb;
      }
    }
    VirtualProtect(lpAddress,dwSize,local_res8[0],local_res8);
    hProcess = GetCurrentProcess();
    FlushInstructionCache(hProcess,lpAddress,dwSize);
    bVar5 = ((char)param_2 * '\x02' ^ *(byte *)(lVar6 + 0x20 + lVar2)) & 2 ^
            *(byte *)(lVar6 + 0x20 + lVar2);
    *(byte *)(lVar6 + 0x20 + lVar2) = ((char)param_2 << 2 ^ bVar5) & 4 ^ bVar5;
    uVar4 = 0;
  }
  return uVar4;
}


/* Address 180001c9c; FUN_180001c9c */

int FUN_180001c9c(void)

{
  int iVar1;
  ulonglong uVar2;
  bool bVar3;
  
  uVar2 = 0;
  while( true ) {
    LOCK();
    bVar3 = DAT_1800072d8 == 0;
    iVar1 = DAT_1800072d8;
    if (bVar3) {
      DAT_1800072d8 = 1;
      iVar1 = 0;
    }
    UNLOCK();
    if (bVar3) break;
    Sleep((uint)(0x1f < uVar2));
    uVar2 = uVar2 + 1;
  }
  return iVar1;
}


/* Address 180001cdc; FUN_180001cdc */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 FUN_180001cdc(longlong *param_1,uint param_2,int param_3)

{
  bool bVar1;
  bool bVar2;
  HANDLE hHeap;
  int iVar3;
  DWORD DVar4;
  HANDLE pvVar5;
  LPVOID lpMem;
  ulonglong uVar6;
  uint uVar7;
  undefined1 auStack_88 [32];
  uint local_68 [2];
  DWORD local_60;
  DWORD local_5c;
  ulonglong local_48;
  
  local_48 = DAT_180007240 ^ (ulonglong)auStack_88;
  uVar6 = 0;
  *param_1 = 0;
  param_1[1] = 0;
  bVar2 = false;
  pvVar5 = (HANDLE)CreateToolhelp32Snapshot(4,0);
  if (pvVar5 != (HANDLE)0xffffffffffffffff) {
    local_68[0] = 0x1c;
    iVar3 = Thread32First(pvVar5,local_68);
    bVar1 = bVar2;
    if (iVar3 != 0) {
      do {
        if (((0xf < local_68[0]) && (DVar4 = GetCurrentProcessId(), local_5c == DVar4)) &&
           (DVar4 = GetCurrentThreadId(), hHeap = DAT_1800072e0, local_60 != DVar4)) {
          lpMem = (LPVOID)*param_1;
          if (lpMem == (LPVOID)0x0) {
            *(undefined4 *)(param_1 + 1) = 0x80;
            lpMem = HeapAlloc(hHeap,0,0x200);
            *param_1 = (longlong)lpMem;
            if (lpMem == (LPVOID)0x0) goto LAB_180001e00;
          }
          else if (*(uint *)(param_1 + 1) <= *(uint *)((longlong)param_1 + 0xc)) {
            uVar7 = *(uint *)(param_1 + 1) * 2;
            *(uint *)(param_1 + 1) = uVar7;
            lpMem = HeapReAlloc(hHeap,0,lpMem,(ulonglong)uVar7 << 2);
            if (lpMem == (LPVOID)0x0) goto LAB_180001e00;
            *param_1 = (longlong)lpMem;
          }
          *(DWORD *)((longlong)lpMem + (ulonglong)*(uint *)((longlong)param_1 + 0xc) * 4) = local_60
          ;
          *(int *)((longlong)param_1 + 0xc) = *(int *)((longlong)param_1 + 0xc) + 1;
        }
        local_68[0] = 0x1c;
        iVar3 = Thread32Next(pvVar5,local_68);
      } while (iVar3 != 0);
      DVar4 = GetLastError();
      bVar1 = true;
      if (DVar4 != 0x12) {
LAB_180001e00:
        bVar1 = bVar2;
        if (*param_1 != 0) {
          HeapFree(DAT_1800072e0,0,(LPVOID)*param_1);
          *param_1 = 0;
        }
      }
    }
    CloseHandle(pvVar5);
    if (bVar1) {
      if (*param_1 == 0) {
        return 0;
      }
      if (*(int *)((longlong)param_1 + 0xc) == 0) {
        return 0;
      }
      do {
        pvVar5 = OpenThread(0x5a,0,*(DWORD *)(*param_1 + uVar6 * 4));
        if (pvVar5 == (HANDLE)0x0) {
LAB_180001e85:
          *(undefined4 *)(*param_1 + uVar6 * 4) = 0;
        }
        else {
          DVar4 = SuspendThread(pvVar5);
          if (DVar4 != 0xffffffff) {
            FUN_1800020e4(pvVar5,param_2,param_3);
          }
          CloseHandle(pvVar5);
          if (DVar4 == 0xffffffff) goto LAB_180001e85;
        }
        uVar7 = (int)uVar6 + 1;
        uVar6 = (ulonglong)uVar7;
        if (*(uint *)((longlong)param_1 + 0xc) <= uVar7) {
          return 0;
        }
      } while( true );
    }
  }
  return 9;
}


/* Address 180001ec4; FUN_180001ec4 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

uint FUN_180001ec4(undefined8 *param_1,LPCVOID param_2,undefined8 *param_3)

{
  uint uVar1;
  undefined8 uVar2;
  undefined8 *puVar3;
  undefined8 *puVar4;
  size_t _Size;
  longlong *plVar5;
  undefined1 auStack_98 [32];
  undefined8 *local_78;
  LPCVOID local_70;
  undefined8 *local_68;
  undefined8 local_60;
  int local_58;
  uint local_54;
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  ulonglong local_40;
  
  local_40 = DAT_180007240 ^ (ulonglong)auStack_98;
  FUN_180001c9c();
  if (DAT_1800072e0 == 0) {
    uVar1 = 2;
  }
  else {
    uVar2 = FUN_18000259c(param_1);
    if (((int)uVar2 == 0) || (uVar2 = FUN_18000259c(param_2), (int)uVar2 == 0)) {
      uVar1 = 7;
    }
    else {
      plVar5 = DAT_1800072e8;
      uVar1 = 0;
      if (DAT_1800072f4 != 0) {
        do {
          if (param_1 == (undefined8 *)*plVar5) {
            if (uVar1 != 0xffffffff) {
              uVar1 = 3;
              goto LAB_180002058;
            }
            break;
          }
          uVar1 = uVar1 + 1;
          plVar5 = plVar5 + 7;
        } while (uVar1 < DAT_1800072f4);
      }
      puVar3 = (undefined8 *)FUN_1800022fc(param_1);
      if (puVar3 == (undefined8 *)0x0) {
        uVar1 = 9;
      }
      else {
        local_78 = param_1;
        local_70 = param_2;
        local_68 = puVar3;
        uVar2 = FUN_1800025d0((ulonglong *)&local_78);
        if ((int)uVar2 == 0) {
          uVar1 = 8;
        }
        else {
          puVar4 = FUN_180001930();
          if (puVar4 != (undefined8 *)0x0) {
            *puVar4 = local_78;
            puVar4[1] = local_60;
            puVar4[2] = local_68;
            *(byte *)(puVar4 + 4) = *(byte *)(puVar4 + 4) & 0xf8 | (byte)local_58 & 1;
            *(uint *)((longlong)puVar4 + 0x24) =
                 *(uint *)((longlong)puVar4 + 0x24) & 0xfffffff0 | local_54 & 0xf;
            memcpy(puVar4 + 5,local_50,8);
            memcpy(puVar4 + 6,local_48,8);
            if (local_58 == 0) {
              _Size = 5;
            }
            else {
              param_1 = (undefined8 *)((longlong)param_1 + -5);
              _Size = 7;
            }
            memcpy(puVar4 + 3,param_1,_Size);
            uVar1 = 0;
            if (param_3 != (undefined8 *)0x0) {
              *param_3 = puVar4[2];
              uVar1 = 0;
            }
            goto LAB_180002058;
          }
          uVar1 = 9;
        }
        FUN_180002320(puVar3);
      }
    }
  }
LAB_180002058:
  LOCK();
  DAT_1800072d8 = 0;
  UNLOCK();
  return uVar1;
}


/* Address 18000207c; FUN_18000207c */

void FUN_18000207c(longlong param_1)

{
  FUN_180001a98(param_1,1);
  return;
}


/* Address 180002088; FUN_180002088 */

undefined8 FUN_180002088(void)

{
  undefined8 uVar1;
  
  uVar1 = 0;
  FUN_180001c9c();
  if (DAT_1800072e0 == (HANDLE)0x0) {
    DAT_1800072e0 = HeapCreate(0,0,0);
    if (DAT_1800072e0 == (HANDLE)0x0) {
      uVar1 = 9;
    }
    else {
      FUN_180002598();
    }
  }
  else {
    uVar1 = 1;
  }
  LOCK();
  DAT_1800072d8 = 0;
  UNLOCK();
  return uVar1;
}


/* Address 1800020e4; FUN_1800020e4 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_1800020e4(HANDLE param_1,uint param_2,int param_3)

{
  BOOL BVar1;
  byte bVar2;
  uint uVar3;
  ulonglong uVar4;
  DWORD64 DVar5;
  DWORD64 *pDVar6;
  longlong lVar7;
  ulonglong uVar8;
  uint uVar9;
  undefined1 auStack_508 [32];
  _CONTEXT local_4e8;
  ulonglong local_18;
  
  local_18 = DAT_180007240 ^ (ulonglong)auStack_508;
  local_4e8.ContextFlags = 0x100001;
  BVar1 = GetThreadContext(param_1,&local_4e8);
  if (BVar1 != 0) {
    uVar9 = DAT_1800072f4;
    if (param_2 != 0xffffffff) {
      uVar9 = param_2 + 1;
    }
    uVar3 = 0;
    if (param_2 != 0xffffffff) {
      uVar3 = param_2;
    }
    if (uVar3 < uVar9) {
      lVar7 = (ulonglong)uVar3 * 0x38;
      uVar8 = (ulonglong)(uVar9 - uVar3);
      do {
        pDVar6 = (DWORD64 *)(DAT_1800072e8 + lVar7);
        if (param_3 == 0) {
          bVar2 = 0;
        }
        else if (param_3 == 1) {
          bVar2 = 1;
        }
        else {
          bVar2 = (byte)pDVar6[4] >> 2 & 1;
        }
        if (((byte)pDVar6[4] >> 1 & 1) != bVar2) {
          if (bVar2 == 0) {
            if ((((byte)pDVar6[4] & 1) == 0) || (DVar5 = *pDVar6, local_4e8.Rip != DVar5 - 5)) {
              uVar4 = 0;
              uVar9 = *(uint *)((longlong)pDVar6 + 0x24) & 0xf;
              if (uVar9 != 0) {
                do {
                  if (local_4e8.Rip ==
                      (ulonglong)*(byte *)(uVar4 + 0x30 + (longlong)pDVar6) + pDVar6[2]) {
                    DVar5 = (ulonglong)*(byte *)(uVar4 + 0x28 + (longlong)pDVar6) + *pDVar6;
                    goto LAB_1800021c6;
                  }
                  uVar3 = (int)uVar4 + 1;
                  uVar4 = (ulonglong)uVar3;
                } while (uVar3 < uVar9);
              }
              if (local_4e8.Rip != pDVar6[1]) goto LAB_1800021c4;
              DVar5 = *pDVar6;
            }
          }
          else {
            uVar4 = 0;
            uVar9 = *(uint *)((longlong)pDVar6 + 0x24) & 0xf;
            if (uVar9 != 0) {
              do {
                if (local_4e8.Rip == (ulonglong)*(byte *)(uVar4 + 0x28 + (longlong)pDVar6) + *pDVar6
                   ) {
                  DVar5 = (ulonglong)*(byte *)(uVar4 + 0x30 + (longlong)pDVar6) + pDVar6[2];
                  goto LAB_1800021c6;
                }
                uVar3 = (int)uVar4 + 1;
                uVar4 = (ulonglong)uVar3;
              } while (uVar3 < uVar9);
            }
LAB_1800021c4:
            DVar5 = 0;
          }
LAB_1800021c6:
          if (DVar5 != 0) {
            local_4e8.Rip = DVar5;
            SetThreadContext(param_1,&local_4e8);
          }
        }
        lVar7 = lVar7 + 0x38;
        uVar8 = uVar8 - 1;
      } while (uVar8 != 0);
    }
  }
  return;
}


/* Address 180002280; FUN_180002280 */

void FUN_180002280(longlong *param_1)

{
  DWORD dwThreadId;
  HANDLE hThread;
  uint uVar1;
  ulonglong uVar2;
  
  if (*param_1 != 0) {
    uVar2 = 0;
    if (*(int *)((longlong)param_1 + 0xc) != 0) {
      do {
        dwThreadId = *(DWORD *)(*param_1 + uVar2 * 4);
        if (dwThreadId != 0) {
          hThread = OpenThread(0x5a,0,dwThreadId);
          if (hThread != (HANDLE)0x0) {
            ResumeThread(hThread);
            CloseHandle(hThread);
          }
        }
        uVar1 = (int)uVar2 + 1;
        uVar2 = (ulonglong)uVar1;
      } while (uVar1 < *(uint *)((longlong)param_1 + 0xc));
    }
    HeapFree(DAT_1800072e0,0,(LPVOID)*param_1);
  }
  return;
}


/* Address 1800022fc; FUN_1800022fc */

void FUN_1800022fc(undefined8 *param_1)

{
  undefined8 uVar1;
  undefined8 *puVar2;
  
  puVar2 = FUN_180002388(param_1);
  if (puVar2 != (undefined8 *)0x0) {
    uVar1 = *(undefined8 *)puVar2[1];
    *(int *)(puVar2 + 2) = *(int *)(puVar2 + 2) + 1;
    puVar2[1] = uVar1;
  }
  return;
}


/* Address 180002320; FUN_180002320 */

void FUN_180002320(undefined8 *param_1)

{
  int *piVar1;
  undefined8 *puVar2;
  undefined8 *lpAddress;
  undefined8 *puVar3;
  
  puVar2 = DAT_1800072f8;
  puVar3 = (undefined8 *)0x0;
  while( true ) {
    lpAddress = puVar2;
    if (lpAddress == (undefined8 *)0x0) {
      return;
    }
    if (lpAddress == (undefined8 *)((ulonglong)param_1 & 0xfffffffffffff000)) break;
    puVar2 = (undefined8 *)*lpAddress;
    puVar3 = lpAddress;
  }
  *param_1 = lpAddress[1];
  piVar1 = (int *)(lpAddress + 2);
  *piVar1 = *piVar1 + -1;
  lpAddress[1] = param_1;
  if (*piVar1 == 0) {
    puVar2 = (undefined8 *)*lpAddress;
    if (puVar3 != (undefined8 *)0x0) {
      *puVar3 = (undefined8 *)*lpAddress;
      puVar2 = DAT_1800072f8;
    }
    DAT_1800072f8 = puVar2;
    VirtualFree(lpAddress,0,0x8000);
  }
  return;
}


/* Address 180002388; FUN_180002388 */

undefined8 * FUN_180002388(undefined8 *param_1)

{
  DWORD DVar1;
  SIZE_T SVar2;
  undefined8 *puVar3;
  ulonglong uVar4;
  ulonglong uVar5;
  undefined8 *puVar6;
  PVOID pvVar7;
  undefined8 *puVar8;
  undefined8 *lpAddress;
  _SYSTEM_INFO local_88;
  _MEMORY_BASIC_INFORMATION local_58;
  PVOID pvVar9;
  
  GetSystemInfo(&local_88);
  puVar8 = local_88.lpMinimumApplicationAddress;
  if (((undefined8 *)0x40000000 < param_1) &&
     (local_88.lpMinimumApplicationAddress < param_1 + -0x8000000)) {
    puVar8 = param_1 + -0x8000000;
  }
  puVar6 = param_1 + 0x8000000;
  if (local_88.lpMaximumApplicationAddress <= param_1 + 0x8000000) {
    puVar6 = local_88.lpMaximumApplicationAddress;
  }
  puVar6 = (undefined8 *)((longlong)puVar6 - 0xfff);
  for (puVar3 = DAT_1800072f8; puVar3 != (undefined8 *)0x0; puVar3 = (undefined8 *)*puVar3) {
    if (((puVar8 <= puVar3) && (puVar3 < puVar6)) && (puVar3[1] != 0)) {
      return puVar3;
    }
  }
  puVar3 = (undefined8 *)0x0;
  lpAddress = param_1;
  if (puVar8 <= param_1) {
    do {
      pvVar7 = (PVOID)(ulonglong)local_88.dwAllocationGranularity;
      pvVar9 = (PVOID)((longlong)lpAddress - (ulonglong)lpAddress % (ulonglong)pvVar7);
      while( true ) {
        lpAddress = (undefined8 *)((longlong)pvVar9 - (longlong)pvVar7);
        if ((lpAddress < puVar8) || (SVar2 = VirtualQuery(lpAddress,&local_58,0x30), SVar2 == 0))
        goto LAB_18000248d;
        if (local_58.State == 0x10000) break;
        pvVar9 = local_58.AllocationBase;
        if (local_58.AllocationBase < pvVar7) goto LAB_18000248d;
      }
      if (lpAddress == (undefined8 *)0x0) break;
      puVar3 = VirtualAlloc(lpAddress,0x1000,0x3000,0x40);
      if (puVar3 != (undefined8 *)0x0) goto LAB_180002532;
    } while (puVar8 <= lpAddress);
LAB_18000248d:
    if (puVar3 != (undefined8 *)0x0) goto LAB_180002537;
  }
  if (puVar6 < param_1) {
    return puVar3;
  }
  do {
    DVar1 = local_88.dwAllocationGranularity;
    uVar4 = (ulonglong)local_88.dwAllocationGranularity;
    param_1 = (undefined8 *)
              (((ulonglong)local_88.dwAllocationGranularity - (ulonglong)param_1 % uVar4) +
              (longlong)param_1);
    if (puVar6 < param_1) {
      return puVar3;
    }
    while( true ) {
      SVar2 = VirtualQuery(param_1,&local_58,0x30);
      if (SVar2 == 0) goto LAB_180002532;
      if (local_58.State == 0x10000) break;
      uVar5 = (DVar1 - 1) + local_58.RegionSize + (longlong)local_58.BaseAddress;
      param_1 = (undefined8 *)(uVar5 - uVar5 % uVar4);
      if (puVar6 < param_1) goto LAB_180002532;
    }
  } while (((param_1 != (undefined8 *)0x0) &&
           (puVar3 = VirtualAlloc(param_1,0x1000,0x3000,0x40), puVar3 == (undefined8 *)0x0)) &&
          (param_1 <= puVar6));
LAB_180002532:
  if (puVar3 == (undefined8 *)0x0) {
    return (undefined8 *)0x0;
  }
LAB_180002537:
  *(undefined4 *)(puVar3 + 2) = 0;
  uVar4 = 0x40;
  puVar6 = (undefined8 *)0x0;
  puVar8 = puVar3;
  do {
    puVar8 = puVar8 + 8;
    *puVar8 = puVar6;
    puVar3[1] = puVar8;
    uVar4 = uVar4 + 0x40;
    puVar6 = puVar8;
  } while (uVar4 < 0xfc1);
  *puVar3 = DAT_1800072f8;
  DAT_1800072f8 = puVar3;
  return puVar3;
}


/* Address 180002598; FUN_180002598 */

void FUN_180002598(void)

{
  return;
}


/* Address 18000259c; FUN_18000259c */

undefined8 FUN_18000259c(LPCVOID param_1)

{
  undefined8 uVar1;
  _MEMORY_BASIC_INFORMATION local_38;
  
  VirtualQuery(param_1,&local_38,0x30);
  if ((local_38.State == 0x1000) && (((byte)local_38.Protect & 0xf0) != 0)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}


/* Address 1800025d0; FUN_1800025d0 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 FUN_1800025d0(ulonglong *param_1)

{
  char cVar1;
  byte bVar2;
  undefined7 extraout_var;
  undefined8 uVar3;
  ulonglong uVar4;
  longlong lVar5;
  ulonglong *puVar6;
  ulonglong uVar7;
  uint uVar8;
  ulonglong *puVar9;
  ulonglong *puVar10;
  undefined1 *puVar11;
  uint uVar12;
  char *pcVar13;
  uint uVar14;
  ulonglong *puVar15;
  byte bVar16;
  undefined1 auStack_c8 [32];
  byte local_a8;
  uint local_a4;
  byte local_a0 [11];
  byte local_95;
  byte local_94;
  byte local_93;
  char local_91;
  int local_8b;
  int local_83;
  uint local_7f;
  undefined1 local_78 [6];
  ulonglong *local_72;
  ulonglong local_68;
  ulonglong *local_60;
  undefined2 local_58;
  undefined4 local_56;
  undefined2 local_52;
  undefined8 local_50;
  ulonglong local_48 [2];
  ulonglong local_38;
  
  local_38 = DAT_180007240 ^ (ulonglong)auStack_c8;
  local_58 = 0x15ff;
  local_50 = 0;
  uVar7 = 0;
  local_78._2_4_ = 0;
  uVar14 = 0;
  local_72 = (ulonglong *)0x0;
  local_60 = (ulonglong *)0x0;
  local_a8 = 0;
  local_a4 = 0;
  param_1[4] = 0;
  local_56 = 2;
  local_52 = 0x8eb;
  local_78._0_2_ = 0x25ff;
  local_68 = 0x25ff0e70;
  puVar9 = (ulonglong *)0x0;
  do {
    bVar16 = (byte)uVar7;
    puVar15 = (ulonglong *)(uVar7 + *param_1);
    uVar7 = param_1[2];
    bVar2 = FUN_180002974((byte *)puVar15,local_a0);
    uVar4 = CONCAT71(extraout_var,bVar2) & 0xffffffff;
    uVar12 = (uint)CONCAT71(extraout_var,bVar2);
    if ((local_7f & 0x1000) != 0) {
      return 0;
    }
    puVar6 = puVar9;
    if (bVar16 < 5) {
      if ((local_93 & 199) == 5) {
        puVar9 = puVar15;
        puVar10 = local_48;
        for (; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(char *)puVar10 = (char)*puVar9;
          puVar9 = (ulonglong *)((longlong)puVar9 + 1);
          puVar10 = (ulonglong *)((longlong)puVar10 + 1);
        }
        puVar10 = local_48;
        *(uint *)((longlong)local_48 +
                 ((ulonglong)local_a0[0] - (ulonglong)(local_7f >> 2 & 0xf)) + -4) =
             ((int)puVar15 - (uVar14 + (int)uVar7)) + local_83;
        if ((local_95 == 0xff) && (local_91 == '\x04')) {
          local_a4 = 1;
        }
      }
      else if (local_95 == 0xe8) {
        puVar10 = (ulonglong *)&local_58;
        local_50 = (longlong)puVar15 + (ulonglong)local_a0[0] + (longlong)local_8b;
LAB_1800027f3:
        uVar12 = 0x10;
        puVar6 = puVar9;
      }
      else {
        puVar10 = puVar15;
        if ((local_95 & 0xfd) == 0xe9) {
          lVar5 = (longlong)(char)local_8b;
          if (local_95 != 0xeb) {
            lVar5 = (longlong)local_8b;
          }
          puVar6 = (ulonglong *)((longlong)puVar15 + lVar5 + (ulonglong)local_a0[0]);
          if ((puVar6 < (ulonglong *)*param_1) || ((ulonglong *)((longlong)*param_1 + 5U) <= puVar6)
             ) {
            local_72 = puVar6;
            uVar12 = 0xe;
            puVar10 = (ulonglong *)local_78;
LAB_180002782:
            local_a4 = (uint)(puVar9 <= puVar15);
            puVar6 = puVar9;
          }
          else if (puVar6 <= puVar9) {
            puVar6 = puVar9;
          }
        }
        else if ((((local_95 & 0xf0) == 0x70) || ((local_95 & 0xfc) == 0xe0)) ||
                ((local_94 & 0xf0) == 0x80)) {
          if (((local_95 & 0xf0) == 0x70) || (lVar5 = (longlong)local_8b, (local_95 & 0xfc) == 0xe0)
             ) {
            lVar5 = (longlong)(char)local_8b;
          }
          puVar6 = (ulonglong *)((longlong)puVar15 + lVar5 + (ulonglong)local_a0[0]);
          if ((puVar6 < (ulonglong *)*param_1) || ((ulonglong *)((longlong)*param_1 + 5U) <= puVar6)
             ) {
            if ((local_95 & 0xfc) == 0xe0) {
              return 0;
            }
            puVar10 = &local_68;
            local_60 = puVar6;
            bVar2 = local_94;
            if (local_95 != 0xf) {
              bVar2 = local_95;
            }
            local_68 = CONCAT71(local_68._1_7_,bVar2) & 0xffffffffffffff0f ^ 0x71;
            goto LAB_1800027f3;
          }
          if (puVar6 <= puVar9) {
            puVar6 = puVar9;
          }
        }
        else if ((local_95 & 0xfe) == 0xc2) goto LAB_180002782;
      }
    }
    else {
      local_72 = puVar15;
      puVar10 = (ulonglong *)local_78;
      local_a4 = 1;
      uVar12 = 0xe;
    }
    if ((puVar15 < puVar6) && (uVar12 != local_a0[0])) {
      return 0;
    }
    if (0x32 < uVar12 + local_a8) {
      return 0;
    }
    if (7 < *(uint *)((longlong)param_1 + 0x24)) {
      return 0;
    }
    *(byte *)((ulonglong)*(uint *)((longlong)param_1 + 0x24) + 0x28 + (longlong)param_1) = bVar16;
    *(byte *)((ulonglong)*(uint *)((longlong)param_1 + 0x24) + 0x30 + (longlong)param_1) = local_a8;
    bVar2 = local_a8 + (char)uVar12;
    uVar14 = (uint)bVar2;
    uVar8 = 1;
    *(int *)((longlong)param_1 + 0x24) = *(int *)((longlong)param_1 + 0x24) + 1;
    puVar11 = (undefined1 *)((ulonglong)local_a8 + param_1[2]);
    for (uVar7 = (ulonglong)uVar12; uVar7 != 0; uVar7 = uVar7 - 1) {
      *puVar11 = (char)*puVar10;
      puVar10 = (ulonglong *)((longlong)puVar10 + 1);
      puVar11 = puVar11 + 1;
    }
    bVar16 = bVar16 + local_a0[0];
    uVar7 = (ulonglong)bVar16;
    puVar9 = puVar6;
    local_a8 = bVar2;
  } while (local_a4 == 0);
  if (bVar16 < 5) {
    uVar7 = *param_1;
    pcVar13 = (char *)(bVar16 + uVar7);
    uVar3 = FUN_180002938(pcVar13,5 - bVar16);
    if ((int)uVar3 == 0) {
      if (((1 < bVar16) || (uVar3 = FUN_180002938(pcVar13,2 - bVar16), (int)uVar3 != 0)) &&
         (uVar3 = FUN_18000259c((LPCVOID)(uVar7 - 5)), (int)uVar3 != 0)) {
        cVar1 = *(char *)(*param_1 - 5);
        if (((cVar1 == '\0') || (cVar1 == -0x70)) || (cVar1 == -0x34)) {
          pcVar13 = (char *)(*param_1 - 4);
          do {
            if (*pcVar13 != cVar1) {
              return 0;
            }
            uVar8 = uVar8 + 1;
            pcVar13 = pcVar13 + 1;
          } while (uVar8 < 5);
          *(undefined4 *)(param_1 + 4) = 1;
          goto LAB_1800028db;
        }
      }
      return 0;
    }
  }
LAB_1800028db:
  local_72 = (ulonglong *)param_1[1];
  param_1[3] = param_1[2] + (ulonglong)bVar2;
  memcpy((void *)(param_1[2] + (ulonglong)bVar2),local_78,0xe);
  return 1;
}


/* Address 180002938; FUN_180002938 */

undefined8 FUN_180002938(char *param_1,uint param_2)

{
  char cVar1;
  uint uVar2;
  ulonglong uVar3;
  
  cVar1 = *param_1;
  if (((cVar1 != '\0') && (cVar1 != -0x70)) && (cVar1 != -0x34)) {
    return 0;
  }
  uVar3 = 1;
  if (1 < param_2) {
    do {
      if (param_1[uVar3] != cVar1) {
        return 0;
      }
      uVar2 = (int)uVar3 + 1;
      uVar3 = (ulonglong)uVar2;
    } while (uVar2 < param_2);
  }
  return 1;
}


/* Address 180002974; FUN_180002974 */

byte FUN_180002974(byte *param_1,byte *param_2)

{
  undefined2 uVar1;
  undefined4 uVar2;
  undefined8 uVar3;
  byte bVar4;
  byte bVar5;
  byte *pbVar6;
  char cVar7;
  byte bVar8;
  uint uVar9;
  byte bVar10;
  byte *pbVar11;
  byte *pbVar12;
  byte bVar13;
  byte bVar14;
  int iVar15;
  ulonglong uVar16;
  uint uVar17;
  uint uVar18;
  byte bVar19;
  undefined *puVar20;
  char local_res8;
  byte local_res20;
  char local_58;
  
  puVar20 = &DAT_180007020;
  local_58 = false;
  uVar9 = 0;
  bVar10 = 0;
  memset(param_2,0,0x25);
  cVar7 = '\x10';
  pbVar11 = param_1;
  do {
    pbVar12 = pbVar11;
    bVar13 = *pbVar12;
    uVar16 = (ulonglong)bVar13;
    pbVar11 = pbVar12 + 1;
    bVar19 = (byte)uVar9;
    if (bVar13 < 0x66) {
      if ((0x3f < (byte)(bVar13 - 0x26)) ||
         ((0xc000000001010101U >> ((ulonglong)(bVar13 - 0x26) & 0x3f) & 1) == 0)) break;
      param_2[3] = bVar13;
      uVar9 = (uint)(bVar19 | 0x40);
    }
    else if (bVar13 == 0x66) {
      param_2[4] = 0x66;
      uVar9 = (uint)(bVar19 | 8);
    }
    else if (bVar13 == 0x67) {
      param_2[5] = 0x67;
      uVar9 = (uint)(bVar19 | 0x10);
    }
    else if (bVar13 == 0xf0) {
      param_2[2] = 0xf0;
      uVar9 = (uint)(bVar19 | 0x20);
    }
    else if (bVar13 == 0xf2) {
      param_2[1] = 0xf2;
      uVar9 = (uint)(bVar19 | 2);
    }
    else {
      if (bVar13 != 0xf3) break;
      param_2[1] = 0xf3;
      uVar9 = (uint)(bVar19 | 4);
    }
    cVar7 = cVar7 + -1;
  } while (cVar7 != '\0');
  uVar17 = uVar9 << 0x17;
  *(uint *)(param_2 + 0x21) = uVar17;
  uVar18 = 1;
  if ((char)uVar9 != '\0') {
    uVar18 = uVar9;
  }
  bVar19 = (byte)uVar18;
  if ((bVar13 & 0xf0) == 0x40) {
    uVar17 = uVar17 | 0x40000000;
    bVar4 = bVar13 >> 3 & 1;
    *(uint *)(param_2 + 0x21) = uVar17;
    param_2[7] = bVar4;
    if (bVar4 != 0) {
      local_58 = (*pbVar11 & 0xf8) == 0xb8;
    }
    param_2[8] = bVar13 >> 2 & 1;
    param_2[10] = bVar13 & 1;
    param_2[9] = bVar13 >> 1 & 1;
    local_res20 = *pbVar11;
    uVar16 = (ulonglong)local_res20;
    pbVar11 = pbVar12 + 2;
    if ((local_res20 & 0xf0) != 0x40) goto LAB_180002ac8;
LAB_180002b31:
    iVar15 = (int)uVar16;
    uVar17 = uVar17 | 0x3000;
    *(uint *)(param_2 + 0x21) = uVar17;
    uVar9 = 0;
    if (((byte)uVar16 & 0xfd) != 0x24) goto LAB_180002b55;
    uVar9 = 1;
    cVar7 = '\0';
  }
  else {
LAB_180002ac8:
    bVar13 = (byte)uVar16;
    param_2[0xb] = bVar13;
    if (bVar13 == 0xf) {
      uVar16 = (ulonglong)*pbVar11;
      puVar20 = &DAT_18000706a;
      param_2[0xc] = *pbVar11;
      pbVar11 = pbVar11 + 1;
    }
    else if ((0x9f < bVar13) && (bVar13 < 0xa4)) {
      local_58 = local_58 + '\x01';
      if ((uVar18 & 0x10) == 0) {
        bVar19 = bVar19 & 0xf7;
      }
      else {
        bVar19 = bVar19 | 8;
      }
    }
    local_res20 = (byte)uVar16;
    uVar9 = (uint)(byte)puVar20[(ulonglong)((uint)uVar16 & 3) +
                                (ulonglong)(byte)puVar20[uVar16 >> 2]];
    if (puVar20[(ulonglong)((uint)uVar16 & 3) + (ulonglong)(byte)puVar20[uVar16 >> 2]] == 0xff)
    goto LAB_180002b31;
LAB_180002b55:
    iVar15 = (int)uVar16;
    cVar7 = '\0';
    if ((char)uVar9 < '\0') {
      uVar18 = uVar9 & 0x7f;
      uVar9 = (uint)*(ushort *)(puVar20 + uVar18);
      cVar7 = (char)(*(ushort *)(puVar20 + uVar18) >> 8);
    }
  }
  bVar13 = param_2[0xc];
  if ((bVar13 != 0) &&
     ((*(byte *)((ulonglong)*(byte *)((uVar16 >> 2) + 0x18000715c) + (ulonglong)((uint)uVar16 & 3) +
                0x18000715c) & bVar19) != 0)) {
    uVar17 = uVar17 | 0x3000;
    *(uint *)(param_2 + 0x21) = uVar17;
  }
  if ((uVar9 & 1) == 0) {
    if ((bVar19 & 0x20) != 0) {
      *(uint *)(param_2 + 0x21) = uVar17 | 0x9000;
    }
  }
  else {
    uVar18 = uVar17 | 1;
    *(uint *)(param_2 + 0x21) = uVar18;
    bVar4 = *pbVar11;
    param_2[0xd] = bVar4;
    bVar8 = bVar4 >> 6;
    bVar5 = bVar4 & 7;
    bVar4 = bVar4 >> 3 & 7;
    param_2[0xe] = bVar8;
    param_2[0x10] = bVar5;
    param_2[0xf] = bVar4;
    if ((cVar7 != '\0') && ((char)(cVar7 << bVar4) < '\0')) {
      uVar18 = uVar17 | 0x3001;
      *(uint *)(param_2 + 0x21) = uVar18;
    }
    if (((bVar13 == 0) && (0xd8 < local_res20)) && (local_res20 < 0xe0)) {
      if (bVar8 == 3) {
        cVar7 = (&DAT_180007124)[(ulonglong)bVar4 + (ulonglong)(byte)(local_res20 + 0x27) * 8];
        bVar14 = bVar5;
      }
      else {
        cVar7 = (&DAT_18000711d)[(byte)(local_res20 + 0x27)];
        bVar14 = bVar4;
      }
      if ((char)(cVar7 << bVar14) < '\0') {
        uVar18 = uVar18 | 0x3000;
        *(uint *)(param_2 + 0x21) = uVar18;
      }
    }
    if ((bVar19 & 0x20) != 0) {
      if (bVar8 == 3) {
        *(uint *)(param_2 + 0x21) = uVar18 | 0x9000;
      }
      else {
        if (bVar13 == 0) {
          pbVar12 = &DAT_1800071ce;
          bVar14 = local_res20 & 0xfe;
          pbVar6 = &DAT_1800071e6;
        }
        else {
          pbVar12 = &DAT_1800071e6;
          pbVar6 = &DAT_1800071f8;
          bVar14 = local_res20;
        }
        do {
          if (*pbVar12 == bVar14) {
            if (-1 < (char)(pbVar12[1] << bVar4)) goto LAB_180002cb9;
            break;
          }
          pbVar12 = pbVar12 + 2;
        } while (pbVar12 != pbVar6);
        *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x9000;
      }
    }
LAB_180002cb9:
    if (bVar13 == 0) {
      if (iVar15 == 0x8c) {
LAB_180002e1d:
        if (5 < bVar4) goto LAB_180002e22;
      }
      else {
        if (iVar15 == 0x8e) {
          if (bVar4 != 1) goto LAB_180002e1d;
          goto LAB_180002e22;
        }
LAB_180002d88:
        if (bVar8 == 3) {
          pbVar6 = &DAT_180007207;
          pbVar12 = &DAT_180007231;
          if (bVar13 == 0) {
            pbVar12 = &DAT_180007207;
            pbVar6 = &DAT_1800071f8;
          }
          for (; pbVar6 != pbVar12; pbVar6 = pbVar6 + 3) {
            if (*pbVar6 == local_res20) {
              if (((pbVar6[1] & bVar19) != 0) && (-1 < (char)(pbVar6[2] << bVar4)))
              goto LAB_180002e22;
              break;
            }
          }
        }
        else if (bVar13 != 0) {
          if (iVar15 == 0x50) {
LAB_180002df8:
            bVar13 = bVar19 & 9;
          }
          else {
            if (iVar15 == 0xc5) goto LAB_180002e22;
            if (iVar15 != 0xd6) {
              if ((iVar15 != 0xd7) && (iVar15 != 0xf7)) goto LAB_180002e29;
              goto LAB_180002df8;
            }
            bVar13 = bVar19 & 6;
          }
          if (bVar13 != 0) goto LAB_180002e22;
        }
      }
    }
    else if (iVar15 == 0x20) {
LAB_180002d58:
      bVar8 = 3;
      if ((4 < bVar4) || (bVar4 == 1)) goto LAB_180002e22;
    }
    else {
      if (iVar15 != 0x21) {
        if (iVar15 == 0x22) goto LAB_180002d58;
        if (iVar15 != 0x23) goto LAB_180002d88;
      }
      bVar8 = 3;
      if (bVar4 == 4 || bVar4 == 5) {
LAB_180002e22:
        *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x11000;
      }
    }
LAB_180002e29:
    bVar13 = pbVar11[1];
    pbVar12 = pbVar11 + 2;
    if (bVar4 < 2) {
      if (local_res20 == 0xf6) {
        uVar9 = (uint)(byte)((byte)uVar9 | 2);
      }
      else if (local_res20 == 0xf7) {
        uVar9 = (uint)(byte)((byte)uVar9 | 0x10);
      }
    }
    if (bVar8 == 0) {
      if ((bVar19 & 0x10) == 0) {
        bVar10 = 0;
        if (bVar5 == 5) {
          bVar10 = 4;
        }
      }
      else {
        bVar10 = (bVar5 != 6) - 1U & 2;
      }
    }
    else if (bVar8 == 1) {
      bVar10 = 1;
    }
    else if ((bVar8 == 2) && (bVar10 = 2, (bVar19 & 0x10) == 0)) {
      bVar10 = 4;
    }
    if ((bVar8 != 3) && (bVar5 == 4)) {
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 2;
      pbVar12 = pbVar11 + 3;
      param_2[0x12] = bVar13 >> 6;
      param_2[0x11] = bVar13;
      param_2[0x13] = bVar13 >> 3 & 7;
      param_2[0x14] = bVar13 & 7;
      if (((bVar13 & 7) == 5) && ((bVar8 & 1) == 0)) {
        bVar10 = 4;
      }
    }
    pbVar12 = pbVar12 + -1;
    if (bVar10 == 1) {
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x40;
      param_2[0x1d] = *pbVar12;
    }
    else if (bVar10 == 2) {
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x80;
      *(undefined2 *)(param_2 + 0x1d) = *(undefined2 *)pbVar12;
    }
    else if (bVar10 == 4) {
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x100;
      *(undefined4 *)(param_2 + 0x1d) = *(undefined4 *)pbVar12;
    }
    pbVar11 = pbVar12 + bVar10;
  }
  if ((uVar9 & 0x10) == 0) goto LAB_180002f86;
  if ((uVar9 & 0x40) == 0) {
    if (local_58 == '\0') {
      if ((bVar19 & 8) == 0) {
        *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x10;
        uVar2 = *(undefined4 *)pbVar11;
        pbVar11 = pbVar11 + 4;
        *(undefined4 *)(param_2 + 0x15) = uVar2;
        goto LAB_180002f86;
      }
LAB_180002f8b:
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 8;
      uVar1 = *(undefined2 *)pbVar11;
      pbVar11 = pbVar11 + 2;
      *(undefined2 *)(param_2 + 0x15) = uVar1;
    }
    else {
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x20;
      uVar3 = *(undefined8 *)pbVar11;
      pbVar11 = pbVar11 + 8;
      *(undefined8 *)(param_2 + 0x15) = uVar3;
LAB_180002f86:
      if ((uVar9 & 4) != 0) goto LAB_180002f8b;
    }
    if ((uVar9 & 2) != 0) {
      *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 4;
      bVar10 = *pbVar11;
      pbVar11 = pbVar11 + 1;
      param_2[0x15] = bVar10;
    }
    if ((uVar9 & 0x40) == 0) {
      if ((uVar9 & 0x20) != 0) {
        *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x204;
        bVar10 = *pbVar11;
        pbVar11 = pbVar11 + 1;
        param_2[0x15] = bVar10;
      }
      goto LAB_180002ff0;
    }
  }
  else if ((bVar19 & 8) != 0) {
    *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x208;
    uVar1 = *(undefined2 *)pbVar11;
    pbVar11 = pbVar11 + 2;
    *(undefined2 *)(param_2 + 0x15) = uVar1;
    goto LAB_180002ff0;
  }
  *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x210;
  uVar2 = *(undefined4 *)pbVar11;
  pbVar11 = pbVar11 + 4;
  *(undefined4 *)(param_2 + 0x15) = uVar2;
LAB_180002ff0:
  local_res8 = (char)param_1;
  bVar10 = (char)pbVar11 - local_res8;
  *param_2 = bVar10;
  if (0xf < bVar10) {
    *(uint *)(param_2 + 0x21) = *(uint *)(param_2 + 0x21) | 0x5000;
    bVar10 = 0xf;
    *param_2 = 0xf;
  }
  return bVar10;
}


/* Address 180003024; FUN_180003024 */

ulonglong FUN_180003024(undefined8 param_1,int param_2,longlong param_3)

{
  byte bVar1;
  undefined1 uVar2;
  ulonglong uVar3;
  undefined7 extraout_var;
  
  if (param_2 == 0) {
    uVar2 = FUN_18000318c(CONCAT71((int7)((ulonglong)param_1 >> 8),param_3 != 0));
    return CONCAT71(extraout_var,uVar2);
  }
  if (param_2 != 1) {
    if (param_2 == 2) {
      bVar1 = FUN_1800035b0();
    }
    else {
      if (param_2 != 3) {
        return 1;
      }
      bVar1 = FUN_1800035d8();
    }
    return (ulonglong)bVar1;
  }
  uVar3 = FUN_180003074(param_1,param_3);
  return uVar3;
}


/* Address 180003074; FUN_180003074 */

undefined8 FUN_180003074(undefined8 param_1,undefined8 param_2)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  longlong *plVar6;
  ulonglong uVar7;
  
  uVar4 = FUN_180003694(0);
  if ((char)uVar4 != '\0') {
    uVar4 = __scrt_acquire_startup_lock();
    bVar1 = true;
    if (DAT_180007330 != 0) {
                    /* WARNING: Subroutine does not return */
      FUN_180003858(7);
    }
    DAT_180007330 = 1;
    bVar2 = FUN_180003598();
    if (bVar2) {
      FUN_1800039a4();
      FUN_1800034e0();
      FUN_18000350c();
      iVar3 = _initterm_e(&DAT_180005240,&DAT_180005248);
      if ((iVar3 == 0) && (uVar5 = __scrt_dllmain_after_initialize_c(), (char)uVar5 != '\0')) {
        _initterm(&DAT_180005230,&DAT_180005238);
        DAT_180007330 = 2;
        bVar1 = false;
      }
    }
    __scrt_release_startup_lock((char)uVar4);
    if (!bVar1) {
      plVar6 = (longlong *)FUN_180003844();
      if ((*plVar6 != 0) && (uVar7 = FUN_18000375c((longlong)plVar6), (char)uVar7 != '\0')) {
        (*(code *)PTR__guard_dispatch_icall_180005208)(param_1,2,param_2);
      }
      DAT_180007300 = DAT_180007300 + 1;
      return 1;
    }
  }
  return 0;
}


/* Address 18000318c; FUN_18000318c */

undefined1 FUN_18000318c(undefined8 param_1)

{
  undefined1 uVar1;
  undefined8 uVar2;
  undefined7 uVar3;
  
  uVar1 = (undefined1)param_1;
  if (DAT_180007300 < 1) {
    uVar1 = 0;
  }
  else {
    DAT_180007300 = DAT_180007300 + -1;
    uVar2 = __scrt_acquire_startup_lock();
    if (DAT_180007330 != 2) {
                    /* WARNING: Subroutine does not return */
      FUN_180003858(7);
    }
    __scrt_dllmain_uninitialize_c();
    FUN_1800034f0();
    FUN_1800039e0();
    DAT_180007330 = 0;
    uVar3 = (undefined7)((ulonglong)param_1 >> 8);
    __scrt_release_startup_lock((char)uVar2);
    uVar1 = __scrt_uninitialize_crt(CONCAT71(uVar3,uVar1),'\0');
    FUN_180003680();
  }
  return uVar1;
}


/* Address 180003210; FUN_180003210 */

int FUN_180003210(HMODULE param_1,int param_2,longlong param_3)

{
  ulonglong uVar1;
  undefined8 uVar2;
  HMODULE pHVar3;
  int iVar4;
  
  if ((param_2 == 0) && (DAT_180007300 < 1)) {
    return 0;
  }
  if (param_2 - 1U < 2) {
    if (DAT_180005350 == 0) {
      iVar4 = 1;
    }
    else {
      iVar4 = (*(code *)PTR__guard_dispatch_icall_180005208)();
    }
    if (iVar4 == 0) {
      return 0;
    }
    uVar1 = FUN_180003024(param_1,param_2,param_3);
    if ((int)uVar1 == 0) {
      return 0;
    }
  }
  uVar2 = DllMain(param_1,param_2);
  iVar4 = (int)uVar2;
  if ((param_2 == 1) && (iVar4 == 0)) {
    pHVar3 = param_1;
    DllMain(param_1,0);
    FUN_18000318c(CONCAT71((int7)((ulonglong)pHVar3 >> 8),param_3 != 0));
    if (DAT_180005350 != 0) {
      (*(code *)PTR__guard_dispatch_icall_180005208)(param_1,0,param_3);
    }
  }
  if ((param_2 == 0) || (param_2 == 3)) {
    uVar1 = FUN_180003024(param_1,param_2,param_3);
    iVar4 = 0;
    if ((int)uVar1 != 0) {
      if (DAT_180005350 == 0) {
        iVar4 = 1;
      }
      else {
        iVar4 = (*(code *)PTR__guard_dispatch_icall_180005208)(param_1,param_2,param_3);
      }
    }
  }
  return iVar4;
}


/* Address 180003338; entry */

void entry(HMODULE param_1,int param_2,longlong param_3)

{
  if (param_2 == 1) {
    FUN_180003430();
  }
  FUN_180003210(param_1,param_2,param_3);
  return;
}


/* Address 180003378; FUN_180003378 */

undefined8 FUN_180003378(undefined8 param_1,undefined8 param_2,undefined8 param_3,longlong param_4)

{
  FUN_180003398(param_2,param_4);
  return 1;
}


/* Address 180003398; FUN_180003398 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

ulonglong FUN_180003398(undefined8 param_1,longlong param_2)

{
  ulonglong uVar1;
  ulonglong uVar2;
  
  uVar2 = (ulonglong)*(uint *)(*(longlong *)(param_2 + 0x10) + 8);
  uVar1 = *(ulonglong *)(param_2 + 8);
  if ((*(byte *)(uVar2 + 3 + uVar1) & 0xf) != 0) {
    uVar1 = (ulonglong)*(byte *)(uVar2 + 3 + uVar1) & 0xfffffff0;
  }
  return uVar1;
}


/* Address 180003410; __security_check_cookie */

/* WARNING: This is an inlined function */

void __cdecl __security_check_cookie(uintptr_t _StackCookie)

{
  if ((_StackCookie == DAT_180007240) && ((short)(_StackCookie >> 0x30) == 0)) {
    return;
  }
  FUN_180003a54();
  return;
}


/* Address 180003430; FUN_180003430 */

void FUN_180003430(void)

{
  DWORD DVar1;
  _FILETIME local_res8;
  LARGE_INTEGER local_res10;
  _FILETIME local_18 [2];
  
  if (DAT_180007240 == 0x2b992ddfa232) {
    local_res8.dwLowDateTime = 0;
    local_res8.dwHighDateTime = 0;
    GetSystemTimeAsFileTime(&local_res8);
    local_18[0] = local_res8;
    DVar1 = GetCurrentThreadId();
    local_18[0] = (_FILETIME)((ulonglong)local_18[0] ^ (ulonglong)DVar1);
    DVar1 = GetCurrentProcessId();
    local_18[0] = (_FILETIME)((ulonglong)local_18[0] ^ (ulonglong)DVar1);
    QueryPerformanceCounter(&local_res10);
    DAT_180007240 =
         ((ulonglong)local_res10.s.LowPart << 0x20 ^
          CONCAT44(local_res10.s.HighPart,local_res10.s.LowPart) ^ (ulonglong)local_18[0] ^
         (ulonglong)local_18) & 0xffffffffffff;
    if (DAT_180007240 == 0x2b992ddfa232) {
      DAT_180007240 = 0x2b992ddfa233;
    }
  }
  DAT_180007280 = ~DAT_180007240;
  return;
}


/* Address 1800034e0; FUN_1800034e0 */

void FUN_1800034e0(void)

{
                    /* WARNING: Could not recover jumptable at 0x0001800034e7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  InitializeSListHead(&DAT_180007310);
  return;
}


/* Address 1800034f0; FUN_1800034f0 */

void FUN_1800034f0(void)

{
  __std_type_info_destroy_list(&DAT_180007310);
  return;
}


/* Address 1800034fc; FUN_1800034fc */

undefined * FUN_1800034fc(void)

{
  return &DAT_180007320;
}


/* Address 180003504; FUN_180003504 */

undefined * FUN_180003504(void)

{
  return &DAT_180007328;
}


/* Address 18000350c; FUN_18000350c */

void FUN_18000350c(void)

{
  ulonglong *puVar1;
  
  puVar1 = (ulonglong *)FUN_1800034fc();
  *puVar1 = *puVar1 | 0x24;
  puVar1 = (ulonglong *)FUN_180003504();
  *puVar1 = *puVar1 | 2;
  return;
}


/* Address 180003528; __scrt_acquire_startup_lock */

/* Library Function - Single Match
    __scrt_acquire_startup_lock
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

ulonglong __scrt_acquire_startup_lock(void)

{
  ulonglong uVar1;
  bool bVar2;
  undefined7 extraout_var;
  ulonglong uVar3;
  
  bVar2 = __scrt_is_ucrt_dll_in_use();
  uVar3 = CONCAT71(extraout_var,bVar2);
  if ((int)uVar3 == 0) {
LAB_180003556:
    uVar3 = uVar3 & 0xffffffffffffff00;
  }
  else {
    do {
      uVar3 = 0;
      LOCK();
      bVar2 = DAT_180007338 == 0;
      uVar1 = *(ulonglong *)((longlong)Self + 8);
      if (!bVar2) {
        uVar3 = DAT_180007338;
        uVar1 = DAT_180007338;
      }
      DAT_180007338 = uVar1;
      UNLOCK();
      if (bVar2) goto LAB_180003556;
    } while (*(ulonglong *)((longlong)Self + 8) != uVar3);
    uVar3 = CONCAT71((int7)(uVar3 >> 8),1);
  }
  return uVar3;
}


/* Address 180003564; __scrt_dllmain_after_initialize_c */

/* Library Function - Single Match
    __scrt_dllmain_after_initialize_c
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

undefined8 __scrt_dllmain_after_initialize_c(void)

{
  bool bVar1;
  undefined7 extraout_var;
  undefined8 uVar2;
  ulonglong uVar3;
  
  bVar1 = __scrt_is_ucrt_dll_in_use();
  if ((int)CONCAT71(extraout_var,bVar1) == 0) {
    uVar3 = FUN_180003e34();
    uVar3 = _configure_narrow_argv(uVar3 & 0xffffffff);
    if ((int)uVar3 != 0) {
      return uVar3 & 0xffffffffffffff00;
    }
    uVar2 = _initialize_narrow_environment();
  }
  else {
    uVar2 = FUN_180003b9c();
  }
  return CONCAT71((int7)((ulonglong)uVar2 >> 8),1);
}


/* Address 180003598; FUN_180003598 */

bool FUN_180003598(void)

{
  undefined8 uVar1;
  
  uVar1 = FUN_1800036d0(0);
  return (char)uVar1 != '\0';
}


/* Address 1800035b0; FUN_1800035b0 */

undefined1 FUN_1800035b0(void)

{
  char cVar1;
  
  cVar1 = FUN_180003eb0();
  if (cVar1 != '\0') {
    cVar1 = FUN_180003eb0();
    if (cVar1 != '\0') {
      return 1;
    }
    FUN_180003eb0();
  }
  return 0;
}


/* Address 1800035d8; FUN_1800035d8 */

undefined1 FUN_1800035d8(void)

{
  FUN_180003eb0();
  FUN_180003eb0();
  return 1;
}


/* Address 1800035f0; __scrt_dllmain_exception_filter */

/* Library Function - Single Match
    __scrt_dllmain_exception_filter
   
   Libraries: Visual Studio 2017 Release, Visual Studio 2019 Release */

void __scrt_dllmain_exception_filter
               (undefined8 param_1,int param_2,undefined8 param_3,undefined *param_4,
               undefined4 param_5,undefined8 param_6)

{
  bool bVar1;
  undefined7 extraout_var;
  
  bVar1 = __scrt_is_ucrt_dll_in_use();
  if (((int)CONCAT71(extraout_var,bVar1) == 0) && (param_2 == 1)) {
    (*(code *)PTR__guard_dispatch_icall_180005208)(param_1,0,param_3);
  }
  _seh_filter_dll(param_5,param_6);
  return;
}


/* Address 180003650; __scrt_dllmain_uninitialize_c */

/* Library Function - Single Match
    __scrt_dllmain_uninitialize_c
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void __scrt_dllmain_uninitialize_c(void)

{
  bool bVar1;
  undefined7 extraout_var;
  undefined8 uVar2;
  
  bVar1 = __scrt_is_ucrt_dll_in_use();
  if ((int)CONCAT71(extraout_var,bVar1) != 0) {
    _execute_onexit_table(&DAT_180007348);
    return;
  }
  uVar2 = FUN_180003eb4();
  if ((int)uVar2 == 0) {
    _cexit();
  }
  return;
}


/* Address 180003680; FUN_180003680 */

void FUN_180003680(void)

{
  FUN_180003eb0();
  FUN_180003eb0();
  return;
}


/* Address 180003694; FUN_180003694 */

longlong FUN_180003694(int param_1)

{
  char cVar1;
  uint7 extraout_var;
  uint7 uVar2;
  undefined7 extraout_var_00;
  uint7 extraout_var_01;
  
  if (param_1 == 0) {
    DAT_180007340 = 1;
  }
  FUN_180003b9c();
  cVar1 = FUN_180003eb0();
  uVar2 = extraout_var;
  if (cVar1 != '\0') {
    cVar1 = FUN_180003eb0();
    if (cVar1 != '\0') {
      return CONCAT71(extraout_var_00,1);
    }
    FUN_180003eb0();
    uVar2 = extraout_var_01;
  }
  return (ulonglong)uVar2 << 8;
}


/* Address 1800036d0; FUN_1800036d0 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_1800036d0(uint param_1)

{
  bool bVar1;
  ulonglong in_RAX;
  undefined7 extraout_var;
  
  if (DAT_180007341 == '\0') {
    if (1 < param_1) {
                    /* WARNING: Subroutine does not return */
      FUN_180003858(5);
    }
    bVar1 = __scrt_is_ucrt_dll_in_use();
    if (((int)CONCAT71(extraout_var,bVar1) == 0) || (param_1 != 0)) {
      in_RAX = 0xffffffffffffffff;
      _DAT_180007348 = _DAT_180005360;
      uRam0000000180007350 = _UNK_180005368;
      _DAT_180007358 = 0xffffffffffffffff;
      _DAT_180007360 = _DAT_180005360;
      uRam0000000180007368 = _UNK_180005368;
      _DAT_180007370 = 0xffffffffffffffff;
    }
    else {
      in_RAX = _initialize_onexit_table(&DAT_180007348);
      if (((int)in_RAX != 0) ||
         (in_RAX = _initialize_onexit_table(&DAT_180007360), (int)in_RAX != 0)) {
        return in_RAX & 0xffffffffffffff00;
      }
    }
    DAT_180007341 = '\x01';
  }
  return CONCAT71((int7)(in_RAX >> 8),1);
}


/* Address 18000375c; FUN_18000375c */

ulonglong FUN_18000375c(longlong param_1)

{
  ulonglong uVar1;
  uint7 uVar2;
  longlong lVar3;
  longlong lVar4;
  
  uVar1 = 0x5a4d;
  if (IMAGE_DOS_HEADER_180000000.e_magic == (char  [2])0x5a4d) {
    lVar3 = (longlong)(int)IMAGE_DOS_HEADER_180000000.e_lfanew;
    if ((*(int *)(lVar3 + 0x180000000) == 0x4550) &&
       (uVar1 = 0x20b,
       *(short *)((longlong)IMAGE_DOS_HEADER_180000000.e_res_4_ + lVar3 + -4) == 0x20b)) {
      lVar4 = (ulonglong)*(ushort *)((longlong)IMAGE_DOS_HEADER_180000000.e_res_4_ + lVar3 + -8) +
              0x18 + lVar3 + 0x180000000;
      uVar1 = (ulonglong)*(ushort *)(IMAGE_DOS_HEADER_180000000.e_magic + lVar3 + 6);
      lVar3 = lVar4 + uVar1 * 0x28;
      for (; lVar4 != lVar3; lVar4 = lVar4 + 0x28) {
        if (((ulonglong)*(uint *)(lVar4 + 0xc) <= param_1 - 0x180000000U) &&
           (uVar1 = (ulonglong)(*(int *)(lVar4 + 8) + *(uint *)(lVar4 + 0xc)),
           param_1 - 0x180000000U < uVar1)) goto LAB_1800037d2;
      }
      lVar4 = 0;
LAB_1800037d2:
      if (lVar4 == 0) {
        return uVar1 & 0xffffffffffffff00;
      }
      uVar2 = (uint7)(uVar1 >> 8);
      if (*(int *)(lVar4 + 0x24) < 0) {
        return (ulonglong)uVar2 << 8;
      }
      return CONCAT71(uVar2,1);
    }
  }
  return uVar1 & 0xffffffffffffff00;
}


/* Address 1800037f4; __scrt_release_startup_lock */

/* Library Function - Single Match
    __scrt_release_startup_lock
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void __scrt_release_startup_lock(char param_1)

{
  bool bVar1;
  undefined3 extraout_var;
  
  bVar1 = __scrt_is_ucrt_dll_in_use();
  if ((CONCAT31(extraout_var,bVar1) != 0) && (param_1 == '\0')) {
    LOCK();
    DAT_180007338 = 0;
    UNLOCK();
  }
  return;
}


/* Address 180003818; __scrt_uninitialize_crt */

/* Library Function - Single Match
    __scrt_uninitialize_crt
   
   Library: Visual Studio 2019 Release */

undefined1 __scrt_uninitialize_crt(undefined8 param_1,char param_2)

{
  if ((DAT_180007340 == '\0') || (param_2 == '\0')) {
    FUN_180003eb0();
    FUN_180003eb0();
  }
  return 1;
}


/* Address 180003844; FUN_180003844 */

undefined * FUN_180003844(void)

{
  return &DAT_1800078f8;
}


/* Address 18000384c; FUN_18000384c */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_18000384c(void)

{
  _DAT_180007378 = 0;
  return;
}


/* Address 180003858; FUN_180003858 */

void FUN_180003858(undefined4 param_1)

{
  code *pcVar1;
  BOOL BVar2;
  LONG LVar3;
  PRUNTIME_FUNCTION FunctionEntry;
  undefined1 *puVar4;
  undefined8 unaff_retaddr;
  DWORD64 local_res10;
  undefined1 local_res18 [8];
  undefined1 local_res20 [8];
  undefined1 auStack_5c8 [8];
  undefined1 auStack_5c0 [232];
  undefined1 local_4d8 [152];
  undefined1 *local_440;
  DWORD64 local_3e0;
  
  puVar4 = auStack_5c8;
  BVar2 = IsProcessorFeaturePresent(0x17);
  if (BVar2 != 0) {
    pcVar1 = (code *)swi(0x29);
    (*pcVar1)(param_1);
    puVar4 = auStack_5c0;
  }
  *(undefined8 *)(puVar4 + -8) = 0x18000388c;
  FUN_18000384c();
  *(undefined8 *)(puVar4 + -8) = 0x18000389d;
  memset(local_4d8,0,0x4d0);
  *(undefined8 *)(puVar4 + -8) = 0x1800038a7;
  RtlCaptureContext(local_4d8);
  *(undefined8 *)(puVar4 + -8) = 0x1800038c1;
  FunctionEntry = RtlLookupFunctionEntry(local_3e0,&local_res10,(PUNWIND_HISTORY_TABLE)0x0);
  if (FunctionEntry != (PRUNTIME_FUNCTION)0x0) {
    *(undefined8 *)(puVar4 + 0x38) = 0;
    *(undefined1 **)(puVar4 + 0x30) = local_res18;
    *(undefined1 **)(puVar4 + 0x28) = local_res20;
    *(undefined1 **)(puVar4 + 0x20) = local_4d8;
    *(undefined8 *)(puVar4 + -8) = 0x180003905;
    RtlVirtualUnwind(0,local_res10,local_3e0,FunctionEntry,*(PCONTEXT *)(puVar4 + 0x20),
                     *(PVOID **)(puVar4 + 0x28),*(PDWORD64 *)(puVar4 + 0x30),
                     *(PKNONVOLATILE_CONTEXT_POINTERS *)(puVar4 + 0x38));
  }
  local_440 = &stack0x00000008;
  *(undefined8 *)(puVar4 + -8) = 0x180003937;
  memset(puVar4 + 0x50,0,0x98);
  *(undefined8 *)(puVar4 + 0x60) = unaff_retaddr;
  *(undefined4 *)(puVar4 + 0x50) = 0x40000015;
  *(undefined4 *)(puVar4 + 0x54) = 1;
  *(undefined8 *)(puVar4 + -8) = 0x180003959;
  BVar2 = IsDebuggerPresent();
  *(undefined1 **)(puVar4 + 0x40) = puVar4 + 0x50;
  *(undefined1 **)(puVar4 + 0x48) = local_4d8;
  *(undefined8 *)(puVar4 + -8) = 0x180003976;
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)0x0);
  *(undefined8 *)(puVar4 + -8) = 0x180003981;
  LVar3 = UnhandledExceptionFilter((_EXCEPTION_POINTERS *)(puVar4 + 0x40));
  if ((LVar3 == 0) && (BVar2 != 1)) {
    *(undefined8 *)(puVar4 + -8) = 0x180003992;
    FUN_18000384c();
  }
  return;
}


/* Address 1800039a4; FUN_1800039a4 */

void FUN_1800039a4(void)

{
  longlong *plVar1;
  
  for (plVar1 = &DAT_180005898; plVar1 < &DAT_180005898; plVar1 = plVar1 + 1) {
    if (*plVar1 != 0) {
      (*(code *)PTR__guard_dispatch_icall_180005208)();
    }
  }
  return;
}


/* Address 1800039e0; FUN_1800039e0 */

void FUN_1800039e0(void)

{
  longlong *plVar1;
  
  for (plVar1 = &DAT_1800058a8; plVar1 < &DAT_1800058a8; plVar1 = plVar1 + 1) {
    if (*plVar1 != 0) {
      (*(code *)PTR__guard_dispatch_icall_180005208)();
    }
  }
  return;
}


/* Address 180003a1c; _guard_check_icall */

void _guard_check_icall(void)

{
  return;
}


/* Address 180003a20; __raise_securityfailure */

/* Library Function - Single Match
    __raise_securityfailure
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void __raise_securityfailure(_EXCEPTION_POINTERS *param_1)

{
  HANDLE pvVar1;
  
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)0x0);
  UnhandledExceptionFilter(param_1);
  pvVar1 = GetCurrentProcess();
                    /* WARNING: Could not recover jumptable at 0x000180003a4d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  TerminateProcess(pvVar1,0xc0000409);
  return;
}


/* Address 180003a54; FUN_180003a54 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180003a54(void)

{
  code *pcVar1;
  BOOL BVar2;
  undefined1 *puVar3;
  undefined1 auStack_38 [8];
  undefined1 auStack_30 [48];
  
  puVar3 = auStack_38;
  BVar2 = IsProcessorFeaturePresent(0x17);
  if (BVar2 != 0) {
    pcVar1 = (code *)swi(0x29);
    (*pcVar1)(2);
    puVar3 = auStack_30;
  }
  *(undefined8 *)(puVar3 + -8) = 0x180003a7f;
  FUN_180003b28((PCONTEXT)&DAT_180007420);
  _DAT_180007390 = *(undefined8 *)(puVar3 + 0x38);
  _DAT_1800074b8 = puVar3 + 0x40;
  _DAT_1800074a0 = *(undefined8 *)(puVar3 + 0x40);
  _DAT_180007380 = 0xc0000409;
  _DAT_180007384 = 1;
  _DAT_180007398 = 1;
  DAT_1800073a0 = 2;
  *(undefined8 *)(puVar3 + 0x20) = DAT_180007240;
  *(undefined8 *)(puVar3 + 0x28) = DAT_180007280;
  *(undefined8 *)(puVar3 + -8) = 0x180003b21;
  DAT_180007518 = _DAT_180007390;
  __raise_securityfailure((_EXCEPTION_POINTERS *)&PTR_DAT_180005370);
  return;
}


/* Address 180003b28; FUN_180003b28 */

void FUN_180003b28(PCONTEXT param_1)

{
  DWORD64 ControlPc;
  PRUNTIME_FUNCTION FunctionEntry;
  int iVar1;
  DWORD64 local_res8;
  ulonglong local_res10;
  PVOID local_res18 [2];
  
  RtlCaptureContext();
  ControlPc = param_1->Rip;
  iVar1 = 0;
  do {
    FunctionEntry = RtlLookupFunctionEntry(ControlPc,&local_res8,(PUNWIND_HISTORY_TABLE)0x0);
    if (FunctionEntry == (PRUNTIME_FUNCTION)0x0) {
      return;
    }
    RtlVirtualUnwind(0,local_res8,ControlPc,FunctionEntry,param_1,local_res18,&local_res10,
                     (PKNONVOLATILE_CONTEXT_POINTERS)0x0);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 2);
  return;
}


/* Address 180003b9c; FUN_180003b9c */

/* WARNING: Removing unreachable block (ram,0x000180003c8d) */
/* WARNING: Removing unreachable block (ram,0x000180003c7d) */
/* WARNING: Removing unreachable block (ram,0x000180003c58) */
/* WARNING: Removing unreachable block (ram,0x000180003bd6) */
/* WARNING: Removing unreachable block (ram,0x000180003bb4) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_180003b9c(void)

{
  int *piVar1;
  uint *puVar2;
  int *piVar3;
  longlong lVar4;
  uint uVar5;
  byte bVar6;
  ulonglong uVar7;
  uint uVar8;
  ulonglong uVar9;
  uint uVar10;
  uint uVar11;
  ulonglong uVar12;
  ulonglong in_XCR0;
  
  piVar1 = (int *)cpuid_basic_info(0);
  puVar2 = (uint *)cpuid_Version_info(1);
  uVar5 = puVar2[3];
  if ((piVar1[2] == 0x49656e69 && piVar1[3] == 0x6c65746e) && piVar1[1] == 0x756e6547) {
    uVar8 = *puVar2 & 0xfff3ff0;
    _DAT_1800072a0 = 0x8000;
    _DAT_1800072a8 = 0xffffffffffffffff;
    if ((((uVar8 == 0x106c0) || (uVar8 == 0x20660)) || (uVar8 == 0x20670)) ||
       ((uVar8 - 0x30650 < 0x21 &&
        ((0x100010001U >> ((ulonglong)(uVar8 - 0x30650) & 0x3f) & 1) != 0)))) {
      DAT_1800078f4 = DAT_1800078f4 | 1;
    }
  }
  uVar8 = 0;
  uVar10 = 0;
  uVar11 = 0;
  uVar12 = 0;
  if (6 < *piVar1) {
    piVar3 = (int *)cpuid_Extended_Feature_Enumeration_info(7);
    uVar8 = piVar3[1];
    uVar10 = piVar3[2];
    if ((uVar8 >> 9 & 1) != 0) {
      DAT_1800078f4 = DAT_1800078f4 | 2;
    }
    if (0 < *piVar3) {
      lVar4 = cpuid_Extended_Feature_Enumeration_info(7);
      uVar11 = *(uint *)(lVar4 + 8);
    }
    if (0x23 < *piVar1) {
      lVar4 = cpuid(0x24);
      uVar12 = (ulonglong)*(uint *)(lVar4 + 4);
    }
  }
  _DAT_180007298 = 1;
  DAT_18000729c = 2;
  uVar9 = DAT_180007290 & 0xfffffffffffffffe;
  if ((uVar5 >> 0x14 & 1) != 0) {
    _DAT_180007298 = 2;
    DAT_18000729c = 6;
    uVar9 = DAT_180007290 & 0xffffffffffffffee;
  }
  DAT_180007290 = uVar9;
  if ((uVar5 >> 0x1b & 1) != 0) {
    uVar9 = xinuse(0);
    uVar9 = in_XCR0 & uVar9 & 0xffffffff;
    if (((uVar5 >> 0x1c & 1) != 0) && (bVar6 = (byte)uVar9, (bVar6 & 6) == 6)) {
      _DAT_180007298 = 3;
      uVar7 = DAT_180007290;
      uVar5 = DAT_18000729c | 8;
      if ((uVar8 & 0x20) != 0) {
        _DAT_180007298 = 5;
        uVar7 = DAT_180007290 & 0xfffffffffffffffd;
        uVar5 = DAT_18000729c | 0x28;
        if (((uVar8 & 0xd0030000) == 0xd0030000) && ((bVar6 & 0xe0) == 0xe0)) {
          DAT_18000729c = DAT_18000729c | 0x68;
          _DAT_180007298 = 6;
          uVar7 = DAT_180007290 & 0xffffffffffffffd9;
          uVar5 = DAT_18000729c;
        }
      }
      DAT_18000729c = uVar5;
      DAT_180007290 = uVar7;
      if ((uVar10 >> 0x17 & 1) != 0) {
        DAT_180007290 = DAT_180007290 & 0xfffffffffeffffff;
      }
      if (((uVar11 >> 0x13 & 1) != 0) && ((bVar6 & 0xe0) == 0xe0)) {
        _DAT_1800078f0 = (uint)uVar12 & 0x400ff;
        DAT_180007290 = ~((ulonglong)((uint)(uVar12 >> 0x10) & 6) | 0x1000029) & DAT_180007290;
        if (1 < (byte)_DAT_1800078f0) {
          DAT_180007290 = DAT_180007290 & 0xffffffffffffffbf;
        }
      }
    }
    if (((uVar11 >> 0x15 & 1) != 0) && ((uVar9 >> 0x13 & 1) != 0)) {
      DAT_180007290 = DAT_180007290 & 0xffffffffffffff7f;
    }
  }
  return 0;
}


/* Address 180003e34; FUN_180003e34 */

undefined8 FUN_180003e34(void)

{
  return 1;
}


/* Address 180003e3c; __scrt_is_ucrt_dll_in_use */

/* Library Function - Single Match
    __scrt_is_ucrt_dll_in_use
   
   Libraries: Visual Studio 2017 Release, Visual Studio 2019 Release */

bool __scrt_is_ucrt_dll_in_use(void)

{
  return DAT_1800072b0 != 0;
}


/* Address 180003e50; memcpy */

void * __cdecl memcpy(void *_Dst,void *_Src,size_t _Size)

{
  void *pvVar1;
  
                    /* WARNING: Could not recover jumptable at 0x000180003e50. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  pvVar1 = memcpy(_Dst,_Src,_Size);
  return pvVar1;
}


/* Address 180003e56; memcmp */

int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x000180003e56. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = memcmp(_Buf1,_Buf2,_Size);
  return iVar1;
}


/* Address 180003e5c; memset */

void * __cdecl memset(void *_Dst,int _Val,size_t _Size)

{
  void *pvVar1;
  
                    /* WARNING: Could not recover jumptable at 0x000180003e5c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  pvVar1 = memset(_Dst,_Val,_Size);
  return pvVar1;
}


/* Address 180003e68; __std_type_info_destroy_list */

void __std_type_info_destroy_list(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e68. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  __std_type_info_destroy_list();
  return;
}


/* Address 180003e6e; _initterm */

void _initterm(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e6e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _initterm();
  return;
}


/* Address 180003e74; _initterm_e */

void _initterm_e(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e74. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _initterm_e();
  return;
}


/* Address 180003e7a; _seh_filter_dll */

void _seh_filter_dll(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e7a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _seh_filter_dll();
  return;
}


/* Address 180003e80; _configure_narrow_argv */

void _configure_narrow_argv(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e80. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _configure_narrow_argv();
  return;
}


/* Address 180003e86; _initialize_narrow_environment */

void _initialize_narrow_environment(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e86. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _initialize_narrow_environment();
  return;
}


/* Address 180003e8c; _initialize_onexit_table */

void _initialize_onexit_table(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e8c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _initialize_onexit_table();
  return;
}


/* Address 180003e92; _execute_onexit_table */

void _execute_onexit_table(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e92. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _execute_onexit_table();
  return;
}


/* Address 180003e98; _cexit */

void __cdecl _cexit(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e98. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  _cexit();
  return;
}


/* Address 180003e9e; CreateToolhelp32Snapshot */

void CreateToolhelp32Snapshot(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003e9e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  CreateToolhelp32Snapshot();
  return;
}


/* Address 180003ea4; Thread32First */

void Thread32First(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003ea4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  Thread32First();
  return;
}


/* Address 180003eaa; Thread32Next */

void Thread32Next(void)

{
                    /* WARNING: Could not recover jumptable at 0x000180003eaa. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  Thread32Next();
  return;
}


/* Address 180003eb0; FUN_180003eb0 */

undefined1 FUN_180003eb0(void)

{
  return 1;
}


/* Address 180003eb4; FUN_180003eb4 */

undefined8 FUN_180003eb4(void)

{
  return 0;
}


/* Address 180003ed0; _guard_dispatch_icall */

/* WARNING: This is an inlined function */

void _guard_dispatch_icall(void)

{
  code *UNRECOVERED_JUMPTABLE;
  
                    /* WARNING: Could not recover jumptable at 0x000180003ed0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* Address 180003ef0; _guard_dispatch_icall */

/* WARNING: This is an inlined function */
/* WARNING: Switch with 1 destination removed at 0x000180003ef0 */

void _guard_dispatch_icall(void)

{
  code *UNRECOVERED_JUMPTABLE;
  
                    /* WARNING: Could not recover jumptable at 0x000180003ed0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}


/* Address 180003ef6; FUN_180003ef6 */

void FUN_180003ef6(undefined8 *param_1,ulonglong param_2,LPVOID param_3,SIZE_T param_4)

{
  ulonglong *puVar1;
  LPVOID pvVar2;
  undefined8 uVar3;
  SIZE_T SVar4;
  ulonglong local_20;
  ulonglong local_18;
  
  if ((longlong)param_4 < 0) {
    param_1[1] = 0;
  }
  else {
    if (param_2 == 0) {
      puVar1 = &local_20;
      param_2 = 0;
    }
    else {
      local_20 = 1;
      puVar1 = &local_18;
    }
    *puVar1 = param_2;
    SVar4 = param_4;
    if ((local_20 == 0) || (local_18 == 0)) {
      pvVar2 = (LPVOID)FUN_1800015ff(1,param_4);
    }
    else {
      pvVar2 = FUN_1800017ff(param_3,1,local_18,param_4);
    }
    if (pvVar2 != (LPVOID)0x0) {
      param_1[1] = pvVar2;
      uVar3 = 0;
      param_4 = SVar4;
      goto LAB_180003f81;
    }
    param_1[1] = 1;
  }
  uVar3 = 1;
LAB_180003f81:
  param_1[2] = param_4;
  *param_1 = uVar3;
  return;
}


/* Address 180003f8f; FUN_180003f8f */

void FUN_180003f8f(uint *param_1,char param_2,undefined8 param_3,longlong param_4)

{
  code *pcVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  bool bVar6;
  uint local_30;
  undefined1 local_2c;
  
  uVar4 = *param_1;
  uVar3 = uVar4 & 3;
  uVar5 = uVar4 & 4;
  switch(uVar3) {
  case 0:
switchD_180003fc0_caseD_0:
    return;
  case 2:
    goto switchD_180003fc0_caseD_2;
  case 3:
    goto switchD_180003fc0_caseD_3;
  }
switchD_180003fc0_caseD_1:
  do {
    if (uVar5 == 0) {
      uVar4 = uVar4 | 4;
      LOCK();
      uVar2 = *param_1;
      bVar6 = uVar2 == 1;
      if (bVar6) {
        *param_1 = uVar4;
        uVar2 = 1;
      }
      UNLOCK();
      if (!bVar6) goto LAB_180003fd6;
    }
    FUN_1800017a2(param_1,uVar4);
    uVar4 = *param_1;
    uVar3 = uVar4 & 3;
    uVar5 = uVar4 & 4;
    switch(uVar3) {
    case 0:
      goto switchD_180003fc0_caseD_0;
    case 2:
switchD_180003fc0_caseD_2:
      if (param_2 == '\0') {
                    /* WARNING: Does not return */
        pcVar1 = (code *)invalidInstructionException();
        (*pcVar1)();
      }
    case 3:
      goto switchD_180003fc0_caseD_3;
    }
  } while( true );
LAB_180003fd6:
  do {
    uVar4 = uVar2;
    uVar3 = uVar4 & 3;
    uVar5 = uVar4 & 4;
    switch(uVar3) {
    case 0:
      goto switchD_180003fc0_caseD_0;
    case 1:
      goto switchD_180003fc0_caseD_1;
    case 2:
      goto switchD_180003fc0_caseD_2;
    case 3:
switchD_180003fc0_caseD_3:
      LOCK();
      uVar2 = *param_1;
      bVar6 = uVar4 == uVar2;
      if (bVar6) {
        *param_1 = uVar5 | 1;
        uVar2 = uVar4;
      }
      UNLOCK();
      if (bVar6) {
        local_30 = 0;
        local_2c = uVar3 == 2;
        (**(code **)(param_4 + 0x20))(param_3,&local_30);
        LOCK();
        uVar4 = *param_1;
        *param_1 = local_30;
        UNLOCK();
        if ((uVar4 & 4) == 0) {
          return;
        }
        WakeByAddressAll(param_1);
        return;
      }
    }
  } while( true );
}


/* Address 180004057; FUN_180004057 */

void FUN_180004057(void)

{
  code *pcVar1;
  
                    /* WARNING: Does not return */
  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}


/* Address 180004059; FUN_180004059 */

void FUN_180004059(undefined8 param_1,longlong param_2)

{
  __scrt_release_startup_lock(*(char *)(param_2 + 0x40));
  return;
}


/* Address 180004070; FUN_180004070 */

void FUN_180004070(undefined8 param_1,longlong param_2)

{
  __scrt_release_startup_lock(*(char *)(param_2 + 0x20));
  return;
}


/* Address 180004089; FUN_180004089 */

void FUN_180004089(void)

{
  FUN_180003680();
  return;
}


/* Address 18000409d; FUN_18000409d */

void FUN_18000409d(undefined8 *param_1,longlong param_2)

{
  __scrt_dllmain_exception_filter
            (*(undefined8 *)(param_2 + 0x60),*(int *)(param_2 + 0x68),
             *(undefined8 *)(param_2 + 0x70),FUN_180003024,*(undefined4 *)*param_1,param_1);
  return;
}


/* Address 1800040d3; FUN_1800040d3 */

bool FUN_1800040d3(undefined8 *param_1)

{
  return *(int *)*param_1 == -0x3ffffffb;
}


