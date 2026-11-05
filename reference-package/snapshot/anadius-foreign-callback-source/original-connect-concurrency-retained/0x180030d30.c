/* 0x180030d30 FUN_180030d30; Evidence ev_d87af5d8151a7108b80f8af209dde572d444c5bc177dd0a57110f57cd31e8939 */

undefined8 FUN_180030d30(longlong param_1,longlong *param_2)

{
  char *pcVar1;
  char *pcVar2;
  char cVar3;
  longlong *plVar4;
  bool bVar5;
  longlong *plVar6;
  int iVar7;
  longlong lVar8;
  undefined8 uVar9;
  longlong lVar10;
  longlong lVar11;
  longlong *plVar12;
  
  iVar7 = FUN_18004a6ac((uint *)&DAT_1800a3e08);
  if (iVar7 != 0) {
                    /* WARNING: Subroutine does not return */
    FUN_18004a648(5);
  }
  if (DAT_1800a3e54 == 0x7fffffff) {
    DAT_1800a3e54 = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
    FUN_18004a648(6);
  }
  lVar8 = FUN_180002cc0(param_1,"All",(char *)0x0);
  bVar5 = false;
  if (lVar8 != 0) {
    lVar10 = 0;
    do {
      pcVar1 = (char *)(lVar8 + lVar10);
      lVar11 = lVar10 + 1;
      pcVar2 = (char *)((longlong)&DAT_18007f7cc + lVar10);
      if (*pcVar1 != *pcVar2) break;
      lVar10 = lVar11;
    } while (lVar11 != 5);
    bVar5 = false;
    if (*pcVar1 == *pcVar2) {
      bVar5 = true;
    }
  }
  FUN_18002bf60(param_2);
  plVar12 = (longlong *)*DAT_1800a3e58;
  cVar3 = *(char *)((longlong)plVar12 + 0x19);
  while (cVar3 == '\0') {
    if ((bVar5) || (*(int *)((longlong)plVar12 + 0x44) != 0)) {
      FUN_18002c080(param_2,(byte *)(plVar12 + 4),(uint *)(plVar12 + 8));
    }
    plVar4 = (longlong *)plVar12[2];
    if (*(char *)((longlong)plVar4 + 0x19) == '\0') {
      cVar3 = *(char *)(*plVar4 + 0x19);
      plVar12 = plVar4;
      plVar4 = (longlong *)*plVar4;
      while (cVar3 == '\0') {
        cVar3 = *(char *)(*plVar4 + 0x19);
        plVar12 = plVar4;
        plVar4 = (longlong *)*plVar4;
      }
    }
    else {
      cVar3 = *(char *)(plVar12[1] + 0x19);
      plVar6 = (longlong *)plVar12[1];
      plVar4 = plVar12;
      while ((plVar12 = plVar6, cVar3 == '\0' && (plVar4 == (longlong *)plVar12[2]))) {
        cVar3 = *(char *)(plVar12[1] + 0x19);
        plVar6 = (longlong *)plVar12[1];
        plVar4 = plVar12;
      }
    }
    cVar3 = *(char *)((longlong)plVar12 + 0x19);
  }
  iVar7 = 2;
  do {
    (**(code **)(*param_2 + 0x48))(param_2,0);
    iVar7 = iVar7 + -1;
  } while (0 < iVar7);
  uVar9 = FUN_18004a6b4(0x1800a3e08);
  return CONCAT71((int7)((ulonglong)uVar9 >> 8),1);
}



