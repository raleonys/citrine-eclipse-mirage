/* 0x1800192d0 FUN_1800192d0; Evidence ev_f97924b6178fd1c17ef606a5e5153562cafc5f04e36eecb08e5cdb93f2a31286 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

uint * FUN_1800192d0(uint *param_1,LPCSTR param_2,char param_3)

{
  uint ****ppppuVar1;
  undefined8 *puVar2;
  longlong lVar3;
  uint ****ppppuVar4;
  undefined1 auStack_88 [32];
  undefined4 local_68;
  uint *local_60;
  uint ***local_58;
  undefined8 uStack_50;
  longlong local_48;
  ulonglong uStack_40;
  longlong local_38 [4];
  ulonglong local_18;
  
  local_18 = DAT_1800a1580 ^ (ulonglong)auStack_88;
  local_68 = 0;
  local_60 = param_1;
  FUN_1800145d0((longlong *)&local_58,param_2);
  if (param_3 != '\0') {
    ppppuVar4 = &local_58;
    if (7 < uStack_40) {
      ppppuVar4 = (uint ****)local_58;
    }
    lVar3 = local_48 * 2 >> 1;
    if ((lVar3 < 2) || (0x19 < (*(uint *)ppppuVar4 & 0xffffffdf) - 0x3a0041)) {
      ppppuVar1 = (uint ****)
                  FUN_18000bb30((uint *)ppppuVar4,(uint *)(local_48 * 2 + (longlong)ppppuVar4));
      if (ppppuVar4 == ppppuVar1) goto LAB_1800193aa;
    }
    else if ((lVar3 < 3) ||
            ((*(short *)((longlong)ppppuVar4 + 4) != 0x5c &&
             (*(short *)((longlong)ppppuVar4 + 4) != 0x2f)))) {
LAB_1800193aa:
      puVar2 = FUN_180019820((uint *)&DAT_1800a39e0,local_38);
      FUN_18000a140((undefined8 *)param_1,puVar2);
      local_68 = 2;
      FUN_180016b50(param_1,(uint *)&local_58);
      FUN_18000ce10(local_38);
      if (uStack_40 < 8) {
        return param_1;
      }
      ppppuVar4 = (uint ****)local_58;
      if ((0xfff < uStack_40 * 2 + 2) &&
         (ppppuVar4 = (uint ****)local_58[-1],
         0x1f < (ulonglong)((longlong)local_58 + (-8 - (longlong)ppppuVar4)))) {
                    /* WARNING: Subroutine does not return */
        FUN_18005c974();
      }
      thunk_FUN_18005caec(ppppuVar4);
      return param_1;
    }
  }
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  *(uint ****)param_1 = local_58;
  *(undefined8 *)(param_1 + 2) = uStack_50;
  *(longlong *)(param_1 + 4) = local_48;
  *(ulonglong *)(param_1 + 6) = uStack_40;
  return param_1;
}



