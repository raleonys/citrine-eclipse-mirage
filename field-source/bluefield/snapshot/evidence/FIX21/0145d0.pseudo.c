/* 0x1800145d0 FUN_1800145d0; Evidence ev_b7669ec63c588e3e0c3c3ea478118b765778ea8ceeaac18635081dec14844c4d */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

longlong * FUN_1800145d0(longlong *param_1,LPCSTR param_2)

{
  ulonglong uVar1;
  ulonglong uVar2;
  int iVar3;
  undefined8 uVar4;
  LPWSTR ***ppppWVar5;
  ulonglong uVar6;
  ulonglong uVar7;
  ulonglong uVar8;
  undefined2 *puVar9;
  undefined1 auStackY_88 [32];
  LPWSTR **local_48;
  longlong lStack_40;
  ulonglong local_38;
  ulonglong uStack_30;
  ulonglong local_28;
  
  local_28 = DAT_1800a1580 ^ (ulonglong)auStackY_88;
  uVar1 = *(ulonglong *)(param_2 + 0x10);
  if (0xf < *(ulonglong *)(param_2 + 0x18)) {
    param_2 = *(LPCSTR *)param_2;
  }
  lStack_40 = 0;
  local_38 = 0;
  uStack_30 = 7;
  local_48 = (LPWSTR **)0x0;
  if (uVar1 != 0) {
    if (0x7fffffff < uVar1) {
                    /* WARNING: Subroutine does not return */
      FUN_18000cd70(0x16);
    }
    uVar4 = __std_fs_convert_narrow_to_wide(0xfde9,param_2,(int)uVar1,(LPWSTR)0x0,0);
    uVar2 = local_38;
    iVar3 = (int)((ulonglong)uVar4 >> 0x20);
    if (iVar3 != 0) {
                    /* WARNING: Subroutine does not return */
      FUN_18000cdb0(iVar3);
    }
    uVar6 = (ulonglong)(int)uVar4;
    if (local_38 < uVar6) {
      uVar8 = uVar6 - local_38;
      if (uStack_30 - local_38 < uVar8) {
        FUN_180007fc0(&local_48,uVar8,0,uVar8,0);
      }
      else {
        ppppWVar5 = &local_48;
        if (7 < uStack_30) {
          ppppWVar5 = (LPWSTR ***)local_48;
        }
        puVar9 = (undefined2 *)((longlong)ppppWVar5 + local_38 * 2);
        uVar7 = uVar8;
        local_38 = uVar6;
        if (uVar8 != 0) {
          for (; uVar7 != 0; uVar7 = uVar7 - 1) {
            *puVar9 = 0;
            puVar9 = puVar9 + 1;
          }
        }
        *(undefined2 *)((longlong)ppppWVar5 + (uVar2 + uVar8) * 2) = 0;
      }
    }
    else {
      ppppWVar5 = &local_48;
      if (7 < uStack_30) {
        ppppWVar5 = (LPWSTR ***)local_48;
      }
      local_38 = uVar6;
      *(undefined2 *)((longlong)ppppWVar5 + uVar6 * 2) = 0;
    }
    ppppWVar5 = &local_48;
    if (7 < uStack_30) {
      ppppWVar5 = (LPWSTR ***)local_48;
    }
    uVar4 = __std_fs_convert_narrow_to_wide(0xfde9,param_2,(int)uVar1,(LPWSTR)ppppWVar5,(int)uVar4);
    iVar3 = (int)((ulonglong)uVar4 >> 0x20);
    if (iVar3 != 0) {
                    /* WARNING: Subroutine does not return */
      FUN_18000cdb0(iVar3);
    }
  }
  *param_1 = (longlong)local_48;
  param_1[1] = lStack_40;
  param_1[2] = local_38;
  param_1[3] = uStack_30;
  return param_1;
}



