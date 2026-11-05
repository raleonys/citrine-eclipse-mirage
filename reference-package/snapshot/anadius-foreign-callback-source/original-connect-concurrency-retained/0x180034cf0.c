/* 0x180034cf0 FUN_180034cf0; Evidence ev_4fab18fcac92110149151ce89486bab8893108a4acf7a067b91e53c13866d114 */

longlong * FUN_180034cf0(longlong *param_1)

{
  ulonglong uVar1;
  uint uVar2;
  longlong lVar3;
  undefined1 *puVar4;
  longlong *plVar5;
  int iVar6;
  uint local_res10 [2];
  
  lVar3 = FUN_18005eeb8((longlong *)0x0);
  FUN_180045050(local_res10,(int)lVar3);
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0xf;
  *(undefined1 *)param_1 = 0;
  puVar4 = (undefined1 *)FUN_18004b0bc(0x30);
  param_1[2] = 0x20;
  param_1[3] = 0x2f;
  *puVar4 = (char)*param_1;
  *param_1 = (longlong)puVar4;
  param_1[2] = 0;
  iVar6 = 0x10;
  FUN_18002b300(param_1,0x10);
  do {
    iVar6 = iVar6 + -1;
    uVar2 = FUN_180045070(local_res10);
    uVar1 = param_1[2];
    if (uVar1 < (ulonglong)param_1[3]) {
      param_1[2] = uVar1 + 1;
      plVar5 = param_1;
      if (0xf < (ulonglong)param_1[3]) {
        plVar5 = (longlong *)*param_1;
      }
      *(char *)((longlong)plVar5 + uVar1) = (char)uVar2;
      *(undefined1 *)((longlong)plVar5 + uVar1 + 1) = 0;
    }
    else {
      FUN_180012350(param_1,1,0,(char)uVar2);
    }
  } while (iVar6 != 0);
  FUN_18003d190(param_1);
  return param_1;
}



