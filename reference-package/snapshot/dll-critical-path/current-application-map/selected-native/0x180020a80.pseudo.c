/* 0x180020a80 FUN_180020a80; Evidence ev_f52c22ab6d0482e58f7ca56d36cbfec5a1c0913b03e9f41689c3a3276162843d */

undefined8 * FUN_180020a80(undefined8 param_1,undefined8 *param_2,int param_3)

{
  char *pcVar1;
  
  *param_2 = 0;
  param_2[1] = 0;
  if (param_3 == 1) {
    param_2[2] = 0xc;
    param_2[3] = 0xf;
    *param_2 = 0x2079646165726c41;
    *(undefined4 *)(param_2 + 1) = 0x6e65706f;
    *(undefined1 *)((longlong)param_2 + 0xc) = 0;
    return param_2;
  }
  if (param_3 == 2) {
    param_2[2] = 0xb;
    param_2[3] = 0xf;
    *param_2 = 0x6620666f20646e45;
    *(undefined2 *)(param_2 + 1) = 0x6c69;
    *(undefined1 *)((longlong)param_2 + 10) = 0x65;
    *(undefined1 *)((longlong)param_2 + 0xb) = 0;
    return param_2;
  }
  if (param_3 == 3) {
    param_2[2] = 0;
    param_2[3] = 0;
    pcVar1 = (char *)FUN_18004b0bc(0x20);
    *param_2 = pcVar1;
    param_2[2] = 0x11;
    param_2[3] = 0x1f;
    builtin_strncpy(pcVar1,"Element not found",0x12);
    return param_2;
  }
  if (param_3 == 4) {
    param_2[2] = 0;
    param_2[3] = 0;
    pcVar1 = (char *)FUN_18004b0bc(0x40);
    *param_2 = pcVar1;
    param_2[2] = 0x39;
    param_2[3] = 0x3f;
    builtin_strncpy(pcVar1,"The descriptor does not fit into the select call\'s fd_set",0x3a);
    return param_2;
  }
  param_2[2] = 0xf;
  param_2[3] = 0xf;
  *param_2 = 0x73696d2e6f697361;
  *(undefined4 *)(param_2 + 1) = 0x72652063;
  *(undefined2 *)((longlong)param_2 + 0xc) = 0x6f72;
  *(undefined1 *)((longlong)param_2 + 0xe) = 0x72;
  *(undefined1 *)((longlong)param_2 + 0xf) = 0;
  return param_2;
}



