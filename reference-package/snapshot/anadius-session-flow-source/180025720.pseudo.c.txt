/* 0x180025720 FUN_180025720; Evidence ev_3e78124546b5e0a51f195c1b0be03ae62d45ef4336e5cbc3f29792ddf071ce39 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_180025720(longlong *param_1,int *param_2)

{
  int *piVar1;
  int iVar2;
  ulonglong uVar3;
  undefined8 uVar4;
  LPVOID pvVar5;
  undefined8 *puVar6;
  undefined8 *puVar7;
  longlong lVar8;
  longlong *plVar9;
  ulonglong uVar10;
  LPVOID pvVar11;
  undefined4 *puVar12;
  ulonglong uVar13;
  undefined1 auStackY_1b8 [32];
  uint in_stack_fffffffffffffe6c;
  undefined8 local_178;
  undefined **ppuStack_170;
  undefined8 local_160;
  longlong *local_158;
  undefined4 local_150;
  undefined4 uStack_14c;
  longlong local_148;
  ulonglong local_140;
  ulonglong local_138;
  LPVOID local_130 [3];
  ulonglong local_118;
  undefined4 local_108 [52];
  ulonglong local_38;
  
  local_38 = DAT_1800a1580 ^ (ulonglong)auStackY_1b8;
  uVar13 = 0;
  iVar2 = *param_2;
  if (iVar2 == 0) {
    puVar6 = FUN_180023000(&local_160,(longlong *)(*param_1 + 0x98));
    puVar6 = (undefined8 *)*puVar6;
    puVar7 = (undefined8 *)FUN_18002bc70(local_108);
    uVar4 = puVar7[1];
    puVar6[0x1e] = *puVar7;
    puVar6[0x1f] = uVar4;
    uVar4 = puVar7[3];
    puVar6[0x20] = puVar7[2];
    puVar6[0x21] = uVar4;
    uVar4 = puVar7[5];
    puVar6[0x22] = puVar7[4];
    puVar6[0x23] = uVar4;
    uVar4 = puVar7[7];
    puVar6[0x24] = puVar7[6];
    puVar6[0x25] = uVar4;
    uVar4 = puVar7[9];
    puVar6[0x26] = puVar7[8];
    puVar6[0x27] = uVar4;
    uVar4 = puVar7[0xb];
    puVar6[0x28] = puVar7[10];
    puVar6[0x29] = uVar4;
    uVar4 = puVar7[0xd];
    puVar6[0x2a] = puVar7[0xc];
    puVar6[0x2b] = uVar4;
    uVar4 = puVar7[0xf];
    puVar6[0x2c] = puVar7[0xe];
    puVar6[0x2d] = uVar4;
    uVar4 = puVar7[0x11];
    puVar6[0x2e] = puVar7[0x10];
    puVar6[0x2f] = uVar4;
    uVar4 = puVar7[0x13];
    puVar6[0x30] = puVar7[0x12];
    puVar6[0x31] = uVar4;
    uVar4 = puVar7[0x15];
    puVar6[0x32] = puVar7[0x14];
    puVar6[0x33] = uVar4;
    uVar4 = puVar7[0x17];
    puVar6[0x34] = puVar7[0x16];
    puVar6[0x35] = uVar4;
    puVar6[0x36] = puVar7[0x18];
    puVar7 = FUN_18002d540(puVar6 + 0x1e,local_130);
    uVar3 = puVar7[2];
    if (0xf < (ulonglong)puVar7[3]) {
      puVar7 = (undefined8 *)*puVar7;
    }
    local_178 = (ulonglong)local_178._4_4_ << 0x20;
    ppuStack_170 = &PTR_vftable_1800a10f8;
    if (uVar3 != 0) {
      do {
        if ((int)local_178 != 0) goto LAB_180025a50;
        uVar10 = uVar13;
        if (uVar3 <= uVar13) {
          uVar10 = uVar3;
        }
        local_148 = uVar10 + (longlong)puVar7;
        local_140 = 0x10000;
        if (uVar3 - uVar10 < 0x10000) {
          local_140 = uVar3 - uVar10;
        }
        local_150 = (undefined4)local_140;
        lVar8 = FUN_18002a200(puVar6[3],*(byte *)(puVar6 + 4),&local_150,1,
                              (ulonglong)in_stack_fffffffffffffe6c << 0x20,local_140 == 0,
                              (int *)&local_178);
        uVar13 = uVar13 + lVar8;
      } while (uVar13 < uVar3);
    }
    if ((int)local_178 != 0) {
LAB_180025a50:
                    /* WARNING: Subroutine does not return */
      FUN_180028240(&local_178,(undefined8 *)"write");
    }
    if (0xf < local_118) {
      pvVar11 = local_130[0];
      if ((0xfff < local_118 + 1) &&
         (pvVar11 = *(LPVOID *)((longlong)local_130[0] + -8),
         0x1f < (ulonglong)((longlong)local_130[0] + (-8 - (longlong)pvVar11)))) {
                    /* WARNING: Subroutine does not return */
        FUN_18005c974();
      }
      thunk_FUN_18005caec(pvVar11);
    }
    FUN_1800280d0(puVar6);
    if (local_158 != (longlong *)0x0) {
      LOCK();
      plVar9 = local_158 + 1;
      lVar8 = *plVar9;
      *(int *)plVar9 = (int)*plVar9 + -1;
      UNLOCK();
      if ((int)lVar8 == 1) {
        (**(code **)*local_158)(local_158);
        LOCK();
        piVar1 = (int *)((longlong)local_158 + 0xc);
        iVar2 = *piVar1;
        *piVar1 = *piVar1 + -1;
        UNLOCK();
        if (iVar2 == 1) {
          (**(code **)(*local_158 + 8))(local_158);
        }
      }
    }
  }
  else {
    plVar9 = FUN_180006880(&DAT_1800a4130,0x1800845f8);
    (**(code **)(**(longlong **)(param_2 + 2) + 0x10))(*(longlong **)(param_2 + 2),&local_150,iVar2)
    ;
    puVar12 = &local_150;
    if (0xf < local_138) {
      puVar12 = (undefined4 *)CONCAT44(uStack_14c,local_150);
    }
    plVar9 = FUN_180007990(plVar9,puVar12,local_140);
    uVar13 = FUN_18001e510((longlong)*(int *)(*plVar9 + 4) + (longlong)plVar9,10);
    FUN_18000fe90(plVar9,(byte)uVar13);
    FUN_18000e1e0(plVar9);
    if (0xf < local_138) {
      pvVar5 = (LPVOID)CONCAT44(uStack_14c,local_150);
      pvVar11 = pvVar5;
      if ((0xfff < local_138 + 1) &&
         (pvVar11 = *(LPVOID *)((longlong)pvVar5 + -8),
         0x1f < (ulonglong)((longlong)pvVar5 + (-8 - (longlong)pvVar11)))) {
                    /* WARNING: Subroutine does not return */
        FUN_18005c974();
      }
      thunk_FUN_18005caec(pvVar11);
    }
  }
  FUN_1800270e0((longlong *)*param_1);
  return;
}



