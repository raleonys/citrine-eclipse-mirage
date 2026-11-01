/* 0x180067890 __acrt_initialize_lowio; Evidence ev_53596e1106f3183bdc899f798d5c824609c85d32af2b256dbfff07a0f6d395dd */

/* Library Function - Single Match
    __acrt_initialize_lowio
   
   Libraries: Visual Studio 2017 Release, Visual Studio 2019 Release */

bool __acrt_initialize_lowio(void)

{
  longlong lVar1;
  bool bVar2;
  
  __acrt_lock(7);
  lVar1 = __acrt_lowio_ensure_fh_exists(0);
  bVar2 = (int)lVar1 == 0;
  if (bVar2) {
    initialize_inherited_file_handles_nolock();
    FUN_180067788();
  }
  __acrt_unlock(7);
  return bVar2;
}




/* 0x1800678cc __acrt_uninitialize_lowio; Evidence ev_ee05528565d8f723446d2a4a90a5fdd8a2d6afeef615c9f1adf559539e7a1576 */

/* Library Function - Single Match
    __acrt_uninitialize_lowio
   
   Library: Visual Studio 2019 Release */

undefined1 __acrt_uninitialize_lowio(void)

{
  ulonglong uVar1;
  
  uVar1 = 0;
  do {
    if (*(LPCRITICAL_SECTION *)((longlong)&DAT_1800a54a0 + uVar1) != (LPCRITICAL_SECTION)0x0) {
      __acrt_lowio_destroy_handle_array(*(LPCRITICAL_SECTION *)((longlong)&DAT_1800a54a0 + uVar1));
      *(undefined8 *)((longlong)&DAT_1800a54a0 + uVar1) = 0;
    }
    uVar1 = uVar1 + 8;
  } while (uVar1 < 0x400);
  return 1;
}




/* 0x180073258 __acrt_lowio_destroy_handle_array; Evidence ev_a7cb4d754d283c1b2fff6be4e638fc62084f29cfd9a493afa9daed336a4a03ab */

/* Library Function - Single Match
    __acrt_lowio_destroy_handle_array
   
   Library: Visual Studio 2019 Release */

void __acrt_lowio_destroy_handle_array(LPCRITICAL_SECTION param_1)

{
  LPCRITICAL_SECTION lpCriticalSection;
  
  if (param_1 != (LPCRITICAL_SECTION)0x0) {
    for (lpCriticalSection = param_1;
        lpCriticalSection != (LPCRITICAL_SECTION)&param_1[0x73].LockCount;
        lpCriticalSection = (LPCRITICAL_SECTION)&lpCriticalSection[1].SpinCount) {
      DeleteCriticalSection(lpCriticalSection);
    }
    FUN_1800668c0(param_1);
  }
  return;
}



