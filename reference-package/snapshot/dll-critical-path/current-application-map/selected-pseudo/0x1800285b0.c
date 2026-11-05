/* 0x1800285b0 FUN_1800285b0; Evidence ev_0d416758bef1cb136be764bc42d61b6806ad330bc40b244b4ca7798197b53f2e */

undefined ** FUN_1800285b0(void)

{
  if (*(int *)(*(longlong *)((longlong)ThreadLocalStoragePointer + (ulonglong)_tls_index * 8) + 4) <
      DAT_1800a38ec) {
    FUN_18004b838(&DAT_1800a38ec);
    if (DAT_1800a38ec == -1) {
      atexit((_func_5014 *)&DAT_18007dfe0);
      _Init_thread_footer(&DAT_1800a38ec);
    }
  }
  return &PTR_vftable_1800a1120;
}



