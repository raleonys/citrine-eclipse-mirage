/* 0x180062a80 FUN_180062a80; Evidence ev_50614aa9857141802f47ef16f2118e4167da2b050bd76658ee8a7a4107a6b417 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

LPCSTR FUN_180062a80(undefined8 *param_1)

{
  UINT *pUVar1;
  byte bVar2;
  int *piVar3;
  undefined2 uVar4;
  uint uVar5;
  BOOL BVar6;
  int iVar7;
  LPWSTR pWVar8;
  LPCSTR pCVar9;
  LPCSTR pCVar10;
  LPCSTR pCVar11;
  LPCSTR pCVar12;
  longlong lVar13;
  LPCSTR pCVar14;
  BYTE *pBVar15;
  LPCSTR pCVar16;
  LPCSTR pCVar17;
  LPCSTR pCVar18;
  undefined1 auStackY_c8 [32];
  undefined4 *local_70;
  undefined8 *local_58;
  undefined8 local_50;
  _cpinfo local_48;
  ulonglong local_30;
  
  local_30 = DAT_1800a1580 ^ (ulonglong)auStackY_c8;
  pCVar16 = (LPCSTR)0x0;
  local_50 = 0;
  if ((ushort *)param_1[0x27] == (ushort *)0x0) {
    piVar3 = (int *)param_1[0x20];
    if (piVar3 != (int *)0x0) {
      LOCK();
      *piVar3 = *piVar3 + -1;
      UNLOCK();
    }
    param_1[0x20] = 0;
    *param_1 = &DAT_18008a220;
    param_1[0x21] = 0;
    param_1[0x22] = &DAT_18008a4a0;
    param_1[0x23] = &DAT_18008a620;
    *(undefined4 *)(param_1 + 1) = 1;
    return (LPCSTR)0x0;
  }
  pUVar1 = (UINT *)((longlong)param_1 + 0xc);
  local_70 = (undefined4 *)0x0;
  pCVar17 = (LPCSTR)0x1;
  local_58 = param_1;
  if ((*pUVar1 != 0) ||
     (pWVar8 = FUN_18006e048((__crt_locale_pointers *)&local_58,0,(ushort *)param_1[0x27],0x1004,
                             (longlong *)pUVar1), pCVar11 = pCVar16, pCVar12 = pCVar16,
     pCVar9 = pCVar16, pCVar10 = pCVar16, (int)pWVar8 == 0)) {
    local_70 = _calloc_base(1,4);
    FUN_1800668c0((LPVOID)0x0);
    pCVar9 = _calloc_base(0x180,2);
    FUN_1800668c0((LPVOID)0x0);
    pCVar10 = _calloc_base(0x180,1);
    FUN_1800668c0((LPVOID)0x0);
    pCVar11 = _calloc_base(0x180,1);
    FUN_1800668c0((LPVOID)0x0);
    pCVar12 = _calloc_base(0x101,1);
    FUN_1800668c0((LPVOID)0x0);
    if ((local_70 != (undefined4 *)0x0) &&
       ((((pCVar9 != (LPCSTR)0x0 && (pCVar12 != (LPCSTR)0x0)) && (pCVar10 != (LPCSTR)0x0)) &&
        (pCVar14 = pCVar16, pCVar18 = pCVar12, pCVar11 != (LPCSTR)0x0)))) {
      do {
        *pCVar18 = (CHAR)pCVar14;
        uVar5 = (int)pCVar14 + 1;
        pCVar14 = (LPCSTR)(ulonglong)uVar5;
        pCVar18 = pCVar18 + 1;
      } while ((int)uVar5 < 0x100);
      BVar6 = GetCPInfo(*pUVar1,&local_48);
      if ((BVar6 != 0) && (local_48.MaxCharSize < 6)) {
        if (1 < (ushort)local_48.MaxCharSize) {
          if (*pUVar1 == 0xfde9) {
            FUN_1800793d0((undefined1 (*) [32])(pCVar12 + 0x80),0x20,0x80);
          }
          else {
            pBVar15 = local_48.LeadByte;
            bVar2 = local_48.LeadByte[0];
            while ((bVar2 != 0 && (pBVar15[1] != 0))) {
              uVar5 = (uint)*pBVar15;
              if (*pBVar15 <= pBVar15[1]) {
                do {
                  lVar13 = (longlong)(int)uVar5;
                  uVar5 = uVar5 + 1;
                  pCVar12[lVar13] = ' ';
                } while ((int)uVar5 <= (int)(uint)pBVar15[1]);
              }
              pBVar15 = pBVar15 + 2;
              bVar2 = *pBVar15;
            }
          }
        }
        iVar7 = __acrt_LCMapStringA((__crt_locale_pointers *)0x0,(ushort *)param_1[0x27],0x100,
                                    pCVar12 + 1,0xff,pCVar10 + 0x81,0xff,*pUVar1,0);
        if ((iVar7 != 0) &&
           (iVar7 = __acrt_LCMapStringA((__crt_locale_pointers *)0x0,(ushort *)param_1[0x27],0x200,
                                        pCVar12 + 1,0xff,pCVar11 + 0x81,0xff,*pUVar1,0), iVar7 != 0)
           ) {
          BVar6 = FUN_18006e210((__crt_locale_pointers *)0x0,1,pCVar12,0x100,
                                (LPWORD)(pCVar9 + 0x100),*pUVar1,0);
          if (BVar6 != 0) {
            pCVar17 = pCVar9 + 0xfe;
            pCVar17[0] = '\0';
            pCVar17[1] = '\0';
            pCVar10[0x7f] = '\0';
            pCVar11[0x7f] = '\0';
            pCVar10[0x80] = '\0';
            pCVar11[0x80] = '\0';
            if (1 < (local_48.MaxCharSize & 0xffff)) {
              if (*pUVar1 == 0xfde9) {
                pCVar14 = pCVar11 + 0x100;
                pCVar18 = pCVar9 + 0x200;
                iVar7 = 0x80;
                do {
                  uVar4 = 0;
                  if (iVar7 - 0xc2U < 0x33) {
                    uVar4 = 0x8000;
                  }
                  *(undefined2 *)pCVar18 = uVar4;
                  pCVar18 = pCVar18 + 2;
                  pCVar14[(longlong)pCVar10 - (longlong)pCVar11] = (CHAR)iVar7;
                  *pCVar14 = (CHAR)iVar7;
                  iVar7 = iVar7 + 1;
                  pCVar14 = pCVar14 + 1;
                } while (iVar7 < 0x100);
              }
              else {
                pBVar15 = local_48.LeadByte;
                while ((local_48.LeadByte[0] != 0 && (pBVar15[1] != 0))) {
                  uVar5 = (uint)*pBVar15;
                  if (*pBVar15 <= pBVar15[1]) {
                    do {
                      lVar13 = (longlong)(int)uVar5;
                      (pCVar9 + lVar13 * 2 + 0x100)[0] = '\0';
                      (pCVar9 + lVar13 * 2 + 0x100)[1] = -0x80;
                      pCVar10[lVar13 + 0x80] = (CHAR)uVar5;
                      pCVar11[lVar13 + 0x80] = (CHAR)uVar5;
                      uVar5 = uVar5 + 1;
                    } while ((int)uVar5 <= (int)(uint)pBVar15[1]);
                  }
                  pBVar15 = pBVar15 + 2;
                  local_48.LeadByte[0] = *pBVar15;
                }
              }
            }
            *(undefined8 *)pCVar9 = *(undefined8 *)(pCVar9 + 0x200);
            *(undefined8 *)(pCVar9 + 8) = *(undefined8 *)(pCVar9 + 0x208);
            *(undefined8 *)(pCVar9 + 0x10) = *(undefined8 *)(pCVar9 + 0x210);
            *(undefined8 *)(pCVar9 + 0x18) = *(undefined8 *)(pCVar9 + 0x218);
            *(undefined8 *)(pCVar9 + 0x20) = *(undefined8 *)(pCVar9 + 0x220);
            *(undefined8 *)(pCVar9 + 0x28) = *(undefined8 *)(pCVar9 + 0x228);
            *(undefined8 *)(pCVar9 + 0x30) = *(undefined8 *)(pCVar9 + 0x230);
            *(undefined8 *)(pCVar9 + 0x38) = *(undefined8 *)(pCVar9 + 0x238);
            *(undefined8 *)(pCVar9 + 0x40) = *(undefined8 *)(pCVar9 + 0x240);
            *(undefined8 *)(pCVar9 + 0x48) = *(undefined8 *)(pCVar9 + 0x248);
            *(undefined8 *)(pCVar9 + 0x50) = *(undefined8 *)(pCVar9 + 0x250);
            *(undefined8 *)(pCVar9 + 0x58) = *(undefined8 *)(pCVar9 + 600);
            *(undefined8 *)(pCVar9 + 0x60) = *(undefined8 *)(pCVar9 + 0x260);
            *(undefined8 *)(pCVar9 + 0x68) = *(undefined8 *)(pCVar9 + 0x268);
            *(undefined8 *)(pCVar9 + 0x70) = *(undefined8 *)(pCVar9 + 0x270);
            *(undefined8 *)(pCVar9 + 0x78) = *(undefined8 *)(pCVar9 + 0x278);
            *(undefined8 *)(pCVar9 + 0x80) = *(undefined8 *)(pCVar9 + 0x280);
            *(undefined8 *)(pCVar9 + 0x88) = *(undefined8 *)(pCVar9 + 0x288);
            *(undefined8 *)(pCVar9 + 0x90) = *(undefined8 *)(pCVar9 + 0x290);
            *(undefined8 *)(pCVar9 + 0x98) = *(undefined8 *)(pCVar9 + 0x298);
            *(undefined8 *)(pCVar9 + 0xa0) = *(undefined8 *)(pCVar9 + 0x2a0);
            *(undefined8 *)(pCVar9 + 0xa8) = *(undefined8 *)(pCVar9 + 0x2a8);
            *(undefined8 *)(pCVar9 + 0xb0) = *(undefined8 *)(pCVar9 + 0x2b0);
            *(undefined8 *)(pCVar9 + 0xb8) = *(undefined8 *)(pCVar9 + 0x2b8);
            *(undefined8 *)(pCVar9 + 0xc0) = *(undefined8 *)(pCVar9 + 0x2c0);
            *(undefined8 *)(pCVar9 + 200) = *(undefined8 *)(pCVar9 + 0x2c8);
            *(undefined8 *)(pCVar9 + 0xd0) = *(undefined8 *)(pCVar9 + 0x2d0);
            *(undefined8 *)(pCVar9 + 0xd8) = *(undefined8 *)(pCVar9 + 0x2d8);
            *(undefined8 *)(pCVar9 + 0xe0) = *(undefined8 *)(pCVar9 + 0x2e0);
            *(undefined8 *)(pCVar9 + 0xe8) = *(undefined8 *)(pCVar9 + 0x2e8);
            *(undefined8 *)(pCVar9 + 0xf0) = *(undefined8 *)(pCVar9 + 0x2f0);
            *(undefined4 *)(pCVar9 + 0xf8) = *(undefined4 *)(pCVar9 + 0x2f8);
            *(undefined2 *)(pCVar9 + 0xfc) = *(undefined2 *)(pCVar9 + 0x2fc);
            *(undefined8 *)pCVar10 = *(undefined8 *)(pCVar10 + 0x100);
            *(undefined8 *)(pCVar10 + 8) = *(undefined8 *)(pCVar10 + 0x108);
            *(undefined8 *)(pCVar10 + 0x10) = *(undefined8 *)(pCVar10 + 0x110);
            *(undefined8 *)(pCVar10 + 0x18) = *(undefined8 *)(pCVar10 + 0x118);
            *(undefined8 *)(pCVar10 + 0x20) = *(undefined8 *)(pCVar10 + 0x120);
            *(undefined8 *)(pCVar10 + 0x28) = *(undefined8 *)(pCVar10 + 0x128);
            *(undefined8 *)(pCVar10 + 0x30) = *(undefined8 *)(pCVar10 + 0x130);
            *(undefined8 *)(pCVar10 + 0x38) = *(undefined8 *)(pCVar10 + 0x138);
            *(undefined8 *)(pCVar10 + 0x40) = *(undefined8 *)(pCVar10 + 0x140);
            *(undefined8 *)(pCVar10 + 0x48) = *(undefined8 *)(pCVar10 + 0x148);
            *(undefined8 *)(pCVar10 + 0x50) = *(undefined8 *)(pCVar10 + 0x150);
            *(undefined8 *)(pCVar10 + 0x58) = *(undefined8 *)(pCVar10 + 0x158);
            *(undefined8 *)(pCVar10 + 0x60) = *(undefined8 *)(pCVar10 + 0x160);
            *(undefined8 *)(pCVar10 + 0x68) = *(undefined8 *)(pCVar10 + 0x168);
            *(undefined8 *)(pCVar10 + 0x70) = *(undefined8 *)(pCVar10 + 0x170);
            *(undefined4 *)(pCVar10 + 0x78) = *(undefined4 *)(pCVar10 + 0x178);
            *(undefined2 *)(pCVar10 + 0x7c) = *(undefined2 *)(pCVar10 + 0x17c);
            pCVar10[0x7e] = pCVar10[0x17e];
            *(undefined8 *)pCVar11 = *(undefined8 *)(pCVar11 + 0x100);
            *(undefined8 *)(pCVar11 + 8) = *(undefined8 *)(pCVar11 + 0x108);
            *(undefined8 *)(pCVar11 + 0x10) = *(undefined8 *)(pCVar11 + 0x110);
            *(undefined8 *)(pCVar11 + 0x18) = *(undefined8 *)(pCVar11 + 0x118);
            *(undefined8 *)(pCVar11 + 0x20) = *(undefined8 *)(pCVar11 + 0x120);
            *(undefined8 *)(pCVar11 + 0x28) = *(undefined8 *)(pCVar11 + 0x128);
            *(undefined8 *)(pCVar11 + 0x30) = *(undefined8 *)(pCVar11 + 0x130);
            *(undefined8 *)(pCVar11 + 0x38) = *(undefined8 *)(pCVar11 + 0x138);
            *(undefined8 *)(pCVar11 + 0x40) = *(undefined8 *)(pCVar11 + 0x140);
            *(undefined8 *)(pCVar11 + 0x48) = *(undefined8 *)(pCVar11 + 0x148);
            *(undefined8 *)(pCVar11 + 0x50) = *(undefined8 *)(pCVar11 + 0x150);
            *(undefined8 *)(pCVar11 + 0x58) = *(undefined8 *)(pCVar11 + 0x158);
            *(undefined8 *)(pCVar11 + 0x60) = *(undefined8 *)(pCVar11 + 0x160);
            *(undefined8 *)(pCVar11 + 0x68) = *(undefined8 *)(pCVar11 + 0x168);
            *(undefined8 *)(pCVar11 + 0x70) = *(undefined8 *)(pCVar11 + 0x170);
            *(undefined4 *)(pCVar11 + 0x78) = *(undefined4 *)(pCVar11 + 0x178);
            *(undefined2 *)(pCVar11 + 0x7c) = *(undefined2 *)(pCVar11 + 0x17c);
            pCVar11[0x7e] = pCVar11[0x17e];
            piVar3 = (int *)param_1[0x20];
            if (piVar3 != (int *)0x0) {
              LOCK();
              iVar7 = *piVar3;
              *piVar3 = *piVar3 + -1;
              UNLOCK();
              if (iVar7 == 1) {
                FUN_1800668c0((LPVOID)(param_1[0x21] + -0xfe));
                FUN_1800668c0((LPVOID)(param_1[0x22] + -0x80));
                FUN_1800668c0((LPVOID)(param_1[0x23] + -0x80));
                FUN_1800668c0((LPVOID)param_1[0x20]);
              }
            }
            *local_70 = 1;
            param_1[0x20] = local_70;
            *param_1 = pCVar9 + 0x100;
            param_1[0x21] = pCVar17;
            param_1[0x22] = pCVar10 + 0x80;
            param_1[0x23] = pCVar11 + 0x80;
            *(UINT *)(param_1 + 1) = local_48.MaxCharSize & 0xffff;
            pCVar17 = pCVar16;
            goto LAB_180063059;
          }
        }
      }
    }
  }
  FUN_1800668c0(local_70);
  FUN_1800668c0(pCVar9);
  FUN_1800668c0(pCVar10);
  FUN_1800668c0(pCVar11);
LAB_180063059:
  FUN_1800668c0(pCVar12);
  return pCVar17;
}



