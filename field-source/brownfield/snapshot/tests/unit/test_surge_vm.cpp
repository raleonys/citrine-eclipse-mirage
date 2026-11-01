#include "research/surge_vm.hpp"
#include <cassert>
#include <cstdint>

using preloader_l::research::X64State;

static void check_state_common(const X64State& a,const X64State& b){
    assert(a.rax==b.rax); assert(a.rbx==b.rbx); assert(a.rcx==b.rcx); assert(a.rdx==b.rdx);
    assert(a.rsi==b.rsi); assert(a.rdi==b.rdi); assert(a.rbp==b.rbp); assert(a.rsp==b.rsp);
    assert(a.r8==b.r8); assert(a.r9==b.r9); assert(a.r10==b.r10); assert(a.r11==b.r11);
    assert(a.r12==b.r12); assert(a.r13==b.r13); assert(a.r14==b.r14); assert(a.r15==b.r15);
    assert(a.rip==b.rip); assert(a.rflags==b.rflags);
}

int main(){
    {
        X64State in{12528665354467742746ULL,7083492513391013384ULL,0ULL,750380475323197417ULL,
                    13336974333500393104ULL,7259680864942807035ULL,4437521758130530800ULL,1879048448ULL,
                    232933988100270743ULL,13111917543432708956ULL,7657892293891953396ULL,17329454066028374694ULL,
                    3580974194509012851ULL,17779561442224477718ULL,14092863029711258048ULL,6529178661029856936ULL,
                    6446958019ULL,2130ULL};
        auto out=preloader_l::research::model_vm_entry_44c5c3(in,6442450944ULL);
        X64State want=in; want.rsp=1879048416ULL; want.rip=6446817871ULL; want.rflags=70ULL;
        check_state_common(out.state,want);
        const std::uint64_t words[8]={4344725ULL,7083492513391013384ULL,7083492513391013384ULL,2145339489ULL,27441ULL,2130ULL,1134ULL,4344725ULL};
        assert(out.stack_begin==1879048384ULL); for(int i=0;i<8;++i) assert(out.stack_words[i]==words[i]);
        assert(out.undefined_rflags_mask==16ULL);
    }
    {
        X64State in{372623510356580201ULL,3485550510676883412ULL,0ULL,1144979188717748360ULL,
                    5514255099923520876ULL,17194486777306945816ULL,11305314104168286396ULL,1879048448ULL,
                    15531242640910957319ULL,8327751155006871249ULL,17305013671176870875ULL,9737543851805920513ULL,
                    4882465204301097541ULL,271100826362168069ULL,12165501739086370482ULL,5922626200998548138ULL,
                    6447239667ULL,1558ULL};
        auto out=preloader_l::research::model_vm_entry_4911f3(in,6442450944ULL);
        X64State want=in; want.rsp=1879048416ULL; want.rip=6446948943ULL; want.rflags=1542ULL;
        check_state_common(out.state,want);
        const std::uint64_t words[8]={4720402ULL,1879048408ULL,1879048416ULL,15531242640910957319ULL,44670ULL,1558ULL,426ULL,4720402ULL};
        assert(out.stack_begin==1879048384ULL); for(int i=0;i<8;++i) assert(out.stack_words[i]==words[i]);
        assert(out.undefined_rflags_mask==0ULL);
    }
    {
        X64State in{9788616465260889674ULL,9233319821169443036ULL,2052062887314110326ULL,16437138199565519691ULL,
                    7561317648058399271ULL,15450648569731596794ULL,3360138268392103923ULL,1879052288ULL,
                    13333230364166641688ULL,15273017209045850191ULL,2080129504202257807ULL,5518096743572043069ULL,
                    2585700463936593760ULL,8435439147853393245ULL,5571587621829214066ULL,13076417783169228546ULL,
                    6446817871ULL,3270ULL};
        auto out=preloader_l::research::model_vm_bootstrap_42a24f(in,8337494348688082045ULL,6442450944ULL);
        assert(out.state.rax==0); assert(out.state.rbx==79); assert(out.state.rcx==1); assert(out.state.rdx==384);
        assert(out.state.rsi==6443959625ULL); assert(out.state.rdi==32); assert(out.state.rbp==6443959625ULL);
        assert(out.state.rsp==1879052152ULL); assert(out.state.r8==2052062887314110326ULL); assert(out.state.r9==8337494348688082045ULL);
        assert(out.state.rip==6446818918ULL); assert(out.state.rflags==1094ULL); assert(out.stack_begin==1879052128ULL);
        assert(out.stack_words[0]==1879052144ULL); assert(out.stack_words[19]==6446817876ULL); assert(out.stack_words[20]==9788616465260889674ULL);
    }
    {
        X64State in{14665803558805072500ULL,5532412281477646854ULL,4217966505638387078ULL,10815601608441371586ULL,
                    13836549589751853511ULL,11475613913903654709ULL,18419750408420338166ULL,1879052288ULL,
                    7159523198233562105ULL,9368855342587942247ULL,18026355448698979764ULL,2839728133903888915ULL,
                    14273173946396596922ULL,13341784104101393797ULL,18345547533688198479ULL,7782116271808992464ULL,
                    6446948943ULL,3287ULL};
        auto out=preloader_l::research::model_vm_bootstrap_44a24f(in,15070420177883846814ULL,6442450944ULL);
        assert(out.state.rax==0); assert(out.state.rbx==79); assert(out.state.rcx==1); assert(out.state.rdx==384);
        assert(out.state.rsi==6443959625ULL); assert(out.state.rdi==32); assert(out.state.rbp==6443959625ULL);
        assert(out.state.rsp==1879052152ULL); assert(out.state.r8==4217966505638387078ULL); assert(out.state.r9==15070420177883846814ULL);
        assert(out.state.rip==6446950001ULL); assert(out.state.rflags==1094ULL); assert(out.stack_begin==1879052128ULL);
        assert(out.stack_words[0]==15070420177883846814ULL); assert(out.stack_words[1]==2290124429ULL); assert(out.stack_words[2]==4217966505638387078ULL);
    }
    {
        preloader_l::research::VmContextOccupancy occ{};
        for(auto& x:occ) x.store(7);
        occ[2].store(0);
        const auto scan=preloader_l::research::model_try_claim_vm_context(occ,6442450944ULL);
        assert(scan.acquired); assert(scan.slot==2); assert(scan.attempted_slots==3);
        assert(scan.context_address==6443960393ULL); assert(scan.lock_address==6443960472ULL);
        assert(occ[2].load()==1);
    }
    return 0;
}
