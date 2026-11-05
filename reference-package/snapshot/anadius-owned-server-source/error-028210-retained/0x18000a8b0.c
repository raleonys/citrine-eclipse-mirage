/* 0x18000a8b0 FUN_18000a8b0; Evidence ev_fc947a3901241283810696e19eb65c0c82e180bf30a63575d0813340371d5d06 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 *** FUN_18000a8b0(undefined8 ***param_1,undefined8 *param_2)

{
  undefined8 **ppuVar1;
  undefined8 **ppuVar2;
  undefined8 *****pppppuVar3;
  undefined1 auStack_68 [32];
  undefined8 ****local_48;
  undefined1 local_40;
  undefined8 ****local_38 [3];
  ulonglong local_20;
  ulonglong local_18;
  
  local_18 = DAT_1800a1580 ^ (ulonglong)auStack_68;
  local_48 = (undefined8 ****)param_1;
  (**(code **)(*(longlong *)param_2[1] + 0x10))
            ((longlong *)param_2[1],local_38,*(undefined4 *)param_2);
  *param_1 = (undefined8 **)std::exception::vftable;
  local_40 = 1;
  local_48 = local_38;
  if (0xf < local_20) {
    local_48 = local_38[0];
  }
  param_1[1] = (undefined8 **)0x0;
  param_1[2] = (undefined8 **)0x0;
  __std_exception_copy((longlong *)&local_48,(longlong *)(param_1 + 1));
  *param_1 = (undefined8 **)std::runtime_error::vftable;
  if (0xf < local_20) {
    pppppuVar3 = (undefined8 *****)local_38[0];
    if ((0xfff < local_20 + 1) &&
       (pppppuVar3 = (undefined8 *****)local_38[0][-1],
       0x1f < (ulonglong)((longlong)local_38[0] + (-8 - (longlong)pppppuVar3)))) {
                    /* WARNING: Subroutine does not return */
      FUN_18005c974();
    }
    thunk_FUN_18005caec(pppppuVar3);
  }
  ppuVar1 = (undefined8 **)*param_2;
  ppuVar2 = (undefined8 **)param_2[1];
  *param_1 = (undefined8 **)std::system_error::vftable;
  param_1[3] = ppuVar1;
  param_1[4] = ppuVar2;
  return param_1;
}



