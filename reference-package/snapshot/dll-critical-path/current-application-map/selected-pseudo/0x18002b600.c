/* 0x18002b600 FUN_18002b600; Evidence ev_cf0268b1ee029a828a84218b358c285ae6a5a695d61571ca82f3f2ef3de25174 */

longlong * FUN_18002b600(longlong *param_1,longlong *param_2,longlong *param_3,undefined8 *param_4)

{
  char cVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  uint uVar4;
  int iVar5;
  longlong *plVar6;
  undefined8 *puVar7;
  undefined8 *_Buf2;
  longlong *plVar8;
  longlong *plVar9;
  ulonglong _Size;
  undefined8 *puVar10;
  undefined8 *local_48;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  
  plVar9 = (longlong *)*param_1;
  if (*(char *)((longlong)param_3 + 0x19) == '\0') {
    if (param_3 == (longlong *)*plVar9) {
      uVar4 = FUN_18000b2b0(param_1,param_4,param_3 + 4);
      if ((char)uVar4 != '\0') {
        *param_2 = (longlong)param_3;
        *(undefined4 *)(param_2 + 1) = 1;
        *(undefined1 *)(param_2 + 2) = 0;
        return param_2;
      }
      goto LAB_18002b800;
    }
    uVar4 = FUN_18000b2b0(param_1,param_4,param_3 + 4);
    if ((char)uVar4 != '\0') {
      plVar9 = (longlong *)*param_3;
      if (*(char *)((longlong)plVar9 + 0x19) == '\0') {
        cVar1 = *(char *)(plVar9[2] + 0x19);
        plVar8 = (longlong *)plVar9[2];
        while (plVar6 = plVar8, cVar1 == '\0') {
          plVar8 = (longlong *)plVar6[2];
          cVar1 = *(char *)((longlong)plVar8 + 0x19);
          plVar9 = plVar6;
        }
      }
      else {
        cVar1 = *(char *)(param_3[1] + 0x19);
        plVar6 = (longlong *)param_3[1];
        plVar8 = param_3;
        while ((plVar9 = plVar6, cVar1 == '\0' && (plVar8 == (longlong *)*plVar9))) {
          cVar1 = *(char *)(plVar9[1] + 0x19);
          plVar6 = (longlong *)plVar9[1];
          plVar8 = plVar9;
        }
        if (*(char *)((longlong)plVar8 + 0x19) != '\0') {
          plVar9 = plVar8;
        }
      }
      uVar4 = FUN_18000b2b0(param_1,plVar9 + 4,param_4);
      if ((char)uVar4 != '\0') {
        cVar1 = *(char *)(plVar9[2] + 0x19);
        *(undefined1 *)(param_2 + 2) = 0;
        if (cVar1 != '\0') {
          *param_2 = (longlong)plVar9;
          *(undefined4 *)(param_2 + 1) = 0;
          return param_2;
        }
        *param_2 = (longlong)param_3;
        *(undefined4 *)(param_2 + 1) = 1;
        return param_2;
      }
      goto LAB_18002b800;
    }
    uVar4 = FUN_18000b2b0(param_1,param_3 + 4,param_4);
    if ((char)uVar4 == '\0') {
      *param_2 = (longlong)param_3;
      *(undefined1 *)(param_2 + 2) = 1;
      *(undefined4 *)(param_2 + 1) = 0;
      return param_2;
    }
    plVar9 = (longlong *)param_3[2];
    if (*(char *)((longlong)plVar9 + 0x19) == '\0') {
      cVar1 = *(char *)(*plVar9 + 0x19);
      plVar8 = (longlong *)*plVar9;
      while (cVar1 == '\0') {
        cVar1 = *(char *)(*plVar8 + 0x19);
        plVar9 = plVar8;
        plVar8 = (longlong *)*plVar8;
      }
    }
    else {
      plVar8 = (longlong *)param_3[1];
      plVar6 = param_3;
      plVar9 = plVar8;
      if (*(char *)((longlong)plVar8 + 0x19) != '\0') goto LAB_18002b8e2;
      do {
        plVar9 = plVar8;
        if (plVar6 != (longlong *)plVar8[2]) break;
        plVar9 = (longlong *)plVar8[1];
        plVar6 = plVar8;
        plVar8 = plVar9;
      } while (*(char *)((longlong)plVar9 + 0x19) == '\0');
    }
    if ((*(char *)((longlong)plVar9 + 0x19) != '\0') ||
       (uVar4 = FUN_18000b2b0(param_1,param_4,plVar9 + 4), (char)uVar4 != '\0')) {
LAB_18002b8e2:
      cVar1 = *(char *)(param_3[2] + 0x19);
      *(undefined1 *)(param_2 + 2) = 0;
      if (cVar1 != '\0') {
        *param_2 = (longlong)param_3;
        *(undefined4 *)(param_2 + 1) = 0;
        return param_2;
      }
      *param_2 = (longlong)plVar9;
      *(undefined4 *)(param_2 + 1) = 1;
      return param_2;
    }
  }
  else if ((*(char *)(plVar9[1] + 0x19) != '\0') ||
          (uVar4 = FUN_18000b2b0(param_1,(undefined8 *)(plVar9[2] + 0x20),param_4),
          (char)uVar4 != '\0')) {
    *param_2 = plVar9[2];
    *(undefined4 *)(param_2 + 1) = 0;
    *(undefined1 *)(param_2 + 2) = 0;
    return param_2;
  }
LAB_18002b800:
  puVar10 = (undefined8 *)*param_1;
  uStack_40 = 0;
  local_48 = (undefined8 *)puVar10[1];
  if (*(char *)((longlong)local_48 + 0x19) == '\0') {
    uVar2 = param_4[2];
    puVar7 = local_48;
    do {
      local_48 = puVar7;
      puVar7 = local_48 + 4;
      _Buf2 = param_4;
      if (0xf < (ulonglong)param_4[3]) {
        _Buf2 = (undefined8 *)*param_4;
      }
      uVar3 = local_48[6];
      if (0xf < (ulonglong)local_48[7]) {
        puVar7 = (undefined8 *)*puVar7;
      }
      _Size = uVar3;
      if (uVar2 < uVar3) {
        _Size = uVar2;
      }
      iVar5 = memcmp(puVar7,_Buf2,_Size);
      if (iVar5 == 0) {
        if (uVar3 < uVar2) goto LAB_18002b8ca;
LAB_18002b869:
        uStack_40 = 1;
        puVar7 = (undefined8 *)*local_48;
        puVar10 = local_48;
      }
      else {
        if (-1 < iVar5) goto LAB_18002b869;
LAB_18002b8ca:
        puVar7 = (undefined8 *)local_48[2];
        uStack_40 = 0;
      }
    } while (*(char *)((longlong)puVar7 + 0x19) == '\0');
  }
  if ((*(char *)((longlong)puVar10 + 0x19) == '\0') &&
     (uVar4 = FUN_18000b2b0(param_1,param_4,puVar10 + 4), (char)uVar4 == '\0')) {
    *param_2 = (longlong)puVar10;
    *(undefined4 *)(param_2 + 1) = 2;
    *(undefined1 *)(param_2 + 2) = 1;
  }
  else {
    *(undefined1 *)(param_2 + 2) = 0;
    *param_2 = (longlong)local_48;
    param_2[1] = CONCAT44(uStack_3c,uStack_40);
  }
  return param_2;
}



