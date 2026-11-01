/* 0x18006e800 FUN_18006e800; Evidence ev_d8c082ef9cc46a88c8ca7108849cd9ca5994c23f3db429686f83a36658a6d7ff */

undefined8 FUN_18006e800(longlong param_1,longlong param_2,ulonglong param_3,longlong *param_4)

{
  ulong uVar1;
  char *pcVar2;
  LPVOID pvVar3;
  longlong lVar4;
  ulonglong uVar6;
  undefined8 uVar7;
  ulonglong uVar8;
  longlong lVar5;
  
  uVar7 = 0;
  lVar4 = -1;
  do {
    lVar5 = lVar4;
    lVar4 = lVar5 + 1;
  } while (*(char *)(param_1 + lVar4) != '\0');
  uVar6 = lVar5 + 2;
  if (~param_3 < uVar6) {
    return 0xc;
  }
  uVar8 = param_3 + 1 + uVar6;
  pcVar2 = _calloc_base(uVar8,1);
  if (((param_3 != 0) && (uVar1 = FUN_1800767fc(pcVar2,uVar8,param_2,param_3), uVar1 != 0)) ||
     (uVar1 = FUN_1800767fc(pcVar2 + param_3,uVar8 - param_3,param_1,uVar6), uVar1 != 0)) {
                    /* WARNING: Subroutine does not return */
    _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
  }
  if (param_4[1] == param_4[2]) {
    if (*param_4 == 0) {
      pvVar3 = _calloc_base(4,8);
      *param_4 = (longlong)pvVar3;
      FUN_1800668c0((LPVOID)0x0);
      lVar4 = *param_4;
      if (lVar4 != 0) {
        param_4[1] = lVar4;
        param_4[2] = lVar4 + 0x20;
        goto LAB_18006e956;
      }
    }
    else {
      uVar6 = param_4[2] - *param_4 >> 3;
      if (uVar6 < 0x8000000000000000) {
        pvVar3 = _recalloc_base((LPVOID)*param_4,uVar6 * 2,8);
        if (pvVar3 != (LPVOID)0x0) {
          *param_4 = (longlong)pvVar3;
          param_4[1] = (longlong)((longlong)pvVar3 + uVar6 * 8);
          param_4[2] = (longlong)((longlong)pvVar3 + uVar6 * 0x10);
          FUN_1800668c0((LPVOID)0x0);
          goto LAB_18006e956;
        }
        FUN_1800668c0((LPVOID)0x0);
      }
    }
    uVar7 = 0xc;
    FUN_1800668c0(pcVar2);
  }
  else {
LAB_18006e956:
    *(char **)param_4[1] = pcVar2;
    param_4[1] = param_4[1] + 8;
  }
  FUN_1800668c0((LPVOID)0x0);
  return uVar7;
}



