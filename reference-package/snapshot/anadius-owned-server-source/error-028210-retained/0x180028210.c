/* 0x180028210 FUN_180028210; Evidence ev_02a670557ce1dceb8af766dab7f5d5d49a6c728c526a27da27fc58e0c4ce4604 */

void FUN_180028210(undefined8 *param_1)

{
  code *pcVar1;
  undefined8 local_48;
  undefined8 uStack_40;
  undefined8 **local_38 [7];
  
  local_48 = *param_1;
  uStack_40 = param_1[1];
  FUN_18000a8b0(local_38,&local_48);
  FUN_180023990((longlong)local_38);
  pcVar1 = (code *)swi(3);
  (*pcVar1)();
  return;
}



