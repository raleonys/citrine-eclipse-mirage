#include "node_parse.hpp"
#include <bit>

namespace anadius::socket_recovered {
namespace {
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width=8){
    const auto bytes=memory.view(address,width);std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[i]}<<(8*i);
    return value;
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width=8){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(8*i));
}
std::int32_t signed_count(ReceiveBufferMemory& memory,std::uintptr_t document){
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(load(memory,document+0x104,4)));
}
std::uintptr_t target(NativeNodeParseContext& c,std::uintptr_t node,unsigned slot){
    return static_cast<std::uintptr_t>(load(c.memory,load(c.memory,node)+slot));
}
std::uintptr_t probe(NativeNodeParseContext& c,std::uintptr_t node,unsigned slot){
    return static_cast<std::uintptr_t>(c.nodes.node_probe(target(c,node,slot),node));
}
void release_pair(std::uintptr_t pair,NativeNodeParseContext& c){
    if(load(c.memory,pair,4)&0x200)c.lower.free_04b0f8(load(c.memory,pair+8));
}
void cleanup(std::uintptr_t node,bool live_tail,NativeNodeParseContext& c){
    auto& m=c.memory;
    if(!probe(c,node,0x38)){
        const auto document=load(m,node+8);const auto initial=signed_count(m,document);
        if(initial>0){
            const auto bound=static_cast<std::uint64_t>(initial),entries=load(m,document+0xa8);
            std::uint32_t index=0;
            for(std::uint64_t cursor=0;cursor<bound;++cursor,++index){
                if(load(m,entries+cursor*8)!=node)continue;
                const auto tail=live_tail?static_cast<std::uint64_t>(static_cast<std::int64_t>(signed_count(m,document))):bound;
                const auto replacement=load(m,entries+tail*8-8);
                const auto slot=static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(index)));
                store(m,entries+slot*8,replacement);
                store(m,document+0x104,static_cast<std::uint32_t>(load(m,document+0x104,4))-std::uint32_t{1},4);
                break;
            }
        }
    }
    const auto pool=load(m,node+0x60),table=load(m,node);
    c.nodes.node_destroy(load(m,table+0x78),node,0);
    c.nodes.pool_release(load(m,load(m,pool)+0x18),pool,node);
}
std::uintptr_t name(std::uintptr_t node,NativeNodeParseContext& c){
    if(probe(c,node,0x30))return 0;
    return c.lower.materialize_0035e0(node+0x18);
}
void move_pair(std::uintptr_t to,std::uintptr_t from,NativeNodeParseContext& c){
    if(!to||to==from)return;
    release_pair(to,c);auto& m=c.memory;
    store(m,to,0,4);store(m,to+8,0);store(m,to+0x10,0);
    store(m,to,load(m,from,4),4);
    store(m,to+8,load(m,from+8));store(m,to+0x10,load(m,from+0x10));
    store(m,from,0,4);store(m,from+8,0);store(m,from+0x10,0);
}
}
std::uintptr_t parse_node_004970(std::uintptr_t owner,std::uintptr_t cursor,
    std::uintptr_t closing_pair,std::uintptr_t line_counter,NativeNodeParseScratch scratch,
    NativeNodeParseContext& context){
    auto& m=context.memory;auto& lower=context.lower;const auto ids=context.identities;
    const auto original_document=load(m,owner+8);
    store(m,original_document+0xa4,static_cast<std::uint32_t>(load(m,original_document+0xa4,4))+std::uint32_t{1},4);
    if(load(m,original_document+0xa4,4)==100)
        lower.report_error({original_document,18,static_cast<std::uint32_t>(load(m,original_document+0xa0,4)),ids.depth_format,false,0});
    std::uintptr_t result=0;
    if(!load(m,load(m,owner+8)+0x6c,4)&&cursor){
        while(m.view(cursor,1)[0]){
            store(m,scratch.out_node,0);
            const auto identified=lower.identify_0038b0(load(m,owner+8),cursor,scratch.out_node);
            const auto child=load(m,scratch.out_node);
            if(!child)break;
            const auto child_line=static_cast<std::uint32_t>(load(m,child+0x30,4));
            store(m,scratch.out_node,child_line,4);
            store(m,scratch.local_pair,0,4);store(m,scratch.local_pair+8,0);store(m,scratch.local_pair+0x10,0);
            cursor=lower.parse_child_virtual(target(context,child,0x80),child,identified,scratch.local_pair,line_counter);
            if(!cursor){
                cleanup(child,false,context);
                const auto document=load(m,owner+8);
                if(!load(m,document+0x6c,4))lower.report_error({document,15,child_line,0,false,0});
                release_pair(scratch.local_pair,context);break;
            }
            const auto declaration=probe(context,child,0x48);
            if(declaration){
                bool legal=probe(context,owner,0x38)!=0;
                if(legal){
                    const auto first=load(m,owner+0x38);
                    if(first){
                        legal=probe(context,first,0x48)!=0;
                        if(legal){const auto last=load(m,owner+0x40);legal=last&&probe(context,last,0x48);}
                    }
                }
                if(!legal){
                    const auto document=load(m,owner+8),value=name(declaration,context);
                    lower.report_error({document,11,child_line,ids.declaration_format,true,value});
                    cleanup(child,false,context);release_pair(scratch.local_pair,context);break;
                }
            }
            const auto element=probe(context,child,8);
            if(element){
                const auto mode=static_cast<std::uint32_t>(load(m,element+0x68,4));
                if(mode==2){
                    move_pair(closing_pair,element+0x18,context);
                    const auto pool=load(m,child+0x60);
                    context.nodes.pool_track(load(m,load(m,pool)+0x20),pool);
                    cleanup(child,true,context);release_pair(scratch.local_pair,context);
                    result=cursor;break;
                }
                bool mismatch=false;auto error_line=child_line;
                const auto closing_end=load(m,scratch.local_pair+0x10);
                if(load(m,scratch.local_pair+8)==closing_end)mismatch=mode==0;
                else if(mode)mismatch=true;
                else{
                    const auto element_name=name(element,context);
                    const auto closing_name=lower.materialize_0035e0(scratch.local_pair);
                    if(closing_name!=element_name&&lower.compare_05c550(closing_name,element_name,0x7fffffff)){
                        mismatch=true;error_line=static_cast<std::uint32_t>(load(m,scratch.out_node,4));
                    }
                }
                if(mismatch){
                    const auto document=load(m,owner+8),value=name(element,context);
                    lower.report_error({document,14,error_line,ids.element_format,true,value});
                    cleanup(child,true,context);release_pair(scratch.local_pair,context);break;
                }
            }
            const auto document=load(m,owner+8);
            if(load(m,child+8)==document){
                lower.prepare_003aa0(owner,child);
                const auto last=load(m,owner+0x40);
                if(last){store(m,last+0x50,child);store(m,child+0x48,load(m,owner+0x40));store(m,owner+0x40,child);}
                else{store(m,owner+0x40,child);store(m,owner+0x38,child);store(m,child+0x48,0);}
                store(m,child+0x50,0);store(m,child+0x10,owner);
            }
            release_pair(scratch.local_pair,context);
        }
    }
    store(m,original_document+0xa4,static_cast<std::uint32_t>(load(m,original_document+0xa4,4))-std::uint32_t{1},4);
    return result;
}
void RecoveredNodeParseEffects::parse_virtual(std::uintptr_t target,std::uintptr_t node,
    std::uintptr_t cursor,std::uintptr_t closing_pair,std::uintptr_t line_counter){
    (void)parse_child_virtual(target,node,cursor,closing_pair,line_counter);
}
std::uintptr_t RecoveredNodeParseEffects::parse_child_virtual(std::uintptr_t target,std::uintptr_t node,
    std::uintptr_t cursor,std::uintptr_t closing_pair,std::uintptr_t line_counter){
    if(target!=parse_.identities.node_parse_target)
        return parse_other_virtual(target,node,cursor,closing_pair,line_counter);
    const auto scratch=acquire_parse_scratch(node,cursor,closing_pair,line_counter);
    return parse_node_004970(node,cursor,closing_pair,line_counter,scratch,parse_);
}
} // namespace anadius::socket_recovered
