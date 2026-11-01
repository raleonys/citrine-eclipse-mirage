#include "preloader_l/platform/addon_hook_policy.hpp"
#include <cassert>
#include <iostream>
using namespace preloader_l::platform::addon_hook_policy;
int main() {
 int n=0;
 auto check=[&](bool ok){++n;assert(ok);};
 check(ascii_fold_equal("C:\\Games\\Client.EXE","c:\\games\\client.exe",19));
 check(!ascii_fold_equal("other.exe","client.exe",10));
 check(!ascii_fold_equal(nullptr,"client.exe",10));
 check(!ascii_fold_equal("client.exe","client.exe",0));
 check(!ascii_fold_equal("client.exe!","client.exe",10));
 check(!ascii_fold_equal("client.exe","client.exe!",11));
 check(ascii_fold_equal_wide(L"C:\\CLIENT.EXE","c:\\client.exe",13));
 check(!ascii_fold_equal_wide(L"C:\\CLIENT.EXE!","c:\\client.exe",13));
 check(!ascii_fold_equal_wide(L"\u011eame.exe","game.exe",8));
 check(!ascii_fold_equal("A\xff","a\xff",2));
 check(exact_true("true",4));
 check(exact_true("1",1));
 check(!exact_true("True",4));
 check(!exact_true("0",1));
 check(!exact_true(nullptr,4));
 check(!exact_true("true",0));
 check(selected_documents(5,"D",1));
 check(!selected_documents(0x8005,"D",1));
 check(!selected_documents(5,nullptr,1));
 check(!selected_documents(5,"",0));
 std::cout<<"HF17_OPTIONAL_POLICY_ASSERTIONS "<<n<<" PASS\n";
}
