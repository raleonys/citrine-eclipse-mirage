/* 0x180025480 FUN_180025480; Evidence ev_d3aaf411d3595e43ae2c3c1c8309ed42f85a4434602216330b5e2c58aa788a3b */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_180025480(longlong *param_1,int *param_2,ulonglong param_3)

{
  int iVar1;
  longlong lVar2;
  bool bVar3;
  byte bVar4;
  undefined7 extraout_var;
  undefined **ppuVar5;
  longlong *plVar6;
  longlong *plVar7;
  ulonglong uVar8;
  undefined8 ***pppuVar9;
  undefined8 ****ppppuVar10;
  undefined8 *puVar11;
  undefined1 auStackY_a8 [32];
  ulonglong uStack_70;
  undefined8 *local_68;
  ulonglong uStack_60;
  undefined8 ***local_58;
  undefined8 uStack_50;
  ulonglong local_48;
  ulonglong uStack_40;
  ulonglong local_38;
  
  local_38 = DAT_1800a1580 ^ (ulonglong)auStackY_a8;
  iVar1 = *param_2;
  if (iVar1 != 0) {
    ppuVar5 = FUN_1800285b0();
    plVar7 = *(longlong **)(param_2 + 2);
    if (((undefined *)plVar7[1] == ppuVar5[1]) && (iVar1 == 2)) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    if (bVar3) {
      plVar6 = FUN_180006880(&DAT_1800a4130,0x1800845c8);
      (**(code **)(*plVar7 + 0x10))(plVar7,&local_58,iVar1);
      ppppuVar10 = &local_58;
      if (0xf < uStack_40) {
        ppppuVar10 = (undefined8 ****)local_58;
      }
      plVar7 = FUN_180007990(plVar6,ppppuVar10,local_48);
      uVar8 = FUN_18001e510((longlong)*(int *)(*plVar7 + 4) + (longlong)plVar7,10);
      FUN_18000fe90(plVar7,(byte)uVar8);
      FUN_18000e1e0(plVar7);
      if (0xf < uStack_40) {
        ppppuVar10 = (undefined8 ****)local_58;
        if ((0xfff < uStack_40 + 1) &&
           (ppppuVar10 = (undefined8 ****)local_58[-1],
           0x1f < (ulonglong)((longlong)local_58 + (-8 - (longlong)ppppuVar10)))) {
                    /* WARNING: Subroutine does not return */
          FUN_18005c974();
        }
        thunk_FUN_18005caec(ppppuVar10);
      }
    }
    return;
  }
  lVar2 = *param_1;
  puVar11 = (undefined8 *)(lVar2 + 0xa8);
  local_58 = (undefined8 ***)0x0;
  uStack_50 = 0;
  local_48 = 0;
  uStack_40 = 0;
  uVar8 = param_3 - 1;
  if (*(ulonglong *)(lVar2 + 0xb8) < param_3 - 1) {
    uVar8 = *(ulonglong *)(lVar2 + 0xb8);
  }
  if (0xf < *(ulonglong *)(lVar2 + 0xc0)) {
    puVar11 = (undefined8 *)*puVar11;
  }
  FUN_180006b30(&local_58,puVar11,uVar8);
  local_68 = (undefined8 *)FUN_18004b0bc(0x30);
  if (local_68 == (undefined8 *)0x0) {
    local_68 = (undefined8 *)0x0;
  }
  else {
    *local_68 = local_58;
    local_68[1] = uStack_50;
    local_68[2] = local_48;
    local_68[3] = uStack_40;
    local_48 = 0;
    uStack_40 = 0xf;
    local_58 = (undefined8 ***)((ulonglong)local_58 & 0xffffffffffffff00);
    local_68[4] = *param_1;
    local_68[5] = FUN_180028d30;
  }
  puVar11 = FUN_18005e8e0((LPSECURITY_ATTRIBUTES)0x0,0,(LPCWSTR)FUN_180021770,local_68,0,
                          (DWORD *)&uStack_70);
  if (puVar11 != (undefined8 *)0x0) {
    if ((int)uStack_70 != 0) {
      uStack_60 = uStack_70;
      local_68 = puVar11;
      bVar4 = FUN_18004a458(&local_68);
      if ((int)CONCAT71(extraout_var,bVar4) == 0) {
        uStack_70 = 0;
        if (0xf < uStack_40) {
          pppuVar9 = local_58;
          if ((0xfff < uStack_40 + 1) &&
             (pppuVar9 = (undefined8 ***)local_58[-1],
             0x1f < (ulonglong)((longlong)local_58 + (-8 - (longlong)pppuVar9)))) {
                    /* WARNING: Subroutine does not return */
            FUN_18005c974();
          }
          thunk_FUN_18005caec(pppuVar9);
        }
        lVar2 = *param_1;
        puVar11 = (undefined8 *)(lVar2 + 0xa8);
        uVar8 = *(ulonglong *)(lVar2 + 0xb8);
        if (uVar8 < param_3) {
          param_3 = uVar8;
        }
        if (0xf < *(ulonglong *)(lVar2 + 0xc0)) {
          puVar11 = (undefined8 *)*puVar11;
        }
        FUN_180078d30(puVar11,(undefined8 *)((longlong)puVar11 + param_3),(uVar8 - param_3) + 1);
        *(ulonglong *)(lVar2 + 0xb8) = uVar8 - param_3;
        FUN_1800280d0((undefined8 *)*param_1);
        return;
      }
    }
                    /* WARNING: Subroutine does not return */
    FUN_18004a648(1);
  }
  uStack_70 = uStack_70 & 0xffffffff00000000;
                    /* WARNING: Subroutine does not return */
  FUN_18004a648(6);
}



