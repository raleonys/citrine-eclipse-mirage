/* 0x18002b300 FUN_18002b300; Evidence ev_035e10f60a4facdf90b41d0d45af7f9a50b80c7a8ef00061d18d0bf367821008 */

void FUN_18002b300(undefined8 *param_1,ulonglong param_2)

{
  ulonglong uVar1;
  ulonglong uVar2;
  undefined8 *puVar3;
  undefined8 *puVar4;
  
  uVar1 = param_1[2];
  if (uVar1 <= param_2) {
    uVar2 = param_1[3];
    if (uVar2 != param_2) {
      if (uVar2 < param_2) {
        FUN_1800124c0(param_1,param_2 - uVar1);
        param_1[2] = uVar1;
        return;
      }
      if ((param_2 < 0x10) && (0xf < uVar2)) {
        puVar3 = (undefined8 *)*param_1;
        FUN_180078d30(param_1,puVar3,uVar1 + 1);
        puVar4 = puVar3;
        if ((0xfff < param_1[3] + 1) &&
           (puVar4 = (undefined8 *)puVar3[-1],
           0x1f < (ulonglong)((longlong)puVar3 + (-8 - (longlong)puVar4)))) {
                    /* WARNING: Subroutine does not return */
          FUN_18005c974();
        }
        thunk_FUN_18005caec(puVar4);
        param_1[3] = 0xf;
      }
    }
  }
  return;
}



