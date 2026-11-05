/* 0x180028240 FUN_180028240; Evidence ev_8812039f80975bf30fe10c8e8c563cbf587e3d91e37da97a8180438643f24c06 */

void FUN_180028240(undefined8 *param_1,undefined8 *param_2)

{
  code *pcVar1;
  undefined8 local_48;
  undefined8 uStack_40;
  undefined8 ****local_38 [7];
  
  local_48 = *param_1;
  uStack_40 = param_1[1];
  FUN_1800248b0(local_38,&local_48,param_2);
  FUN_180023990((longlong)local_38);
  pcVar1 = (code *)swi(3);
  (*pcVar1)();
  return;
}



