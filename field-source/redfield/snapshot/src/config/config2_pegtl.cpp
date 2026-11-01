// SPDX-License-Identifier: GPL-3.0-only
#include "config2_pegtl.hpp"
#include <tao/pegtl.hpp>

namespace preloader_l { namespace config2_pegtl {
namespace {
namespace p = tao::pegtl;
struct Whitespace : p::one<' ', '\t', '\n', '\r'> {};
struct LineComment : p::seq<p::string<'/', '/'>,
    p::star<p::seq<p::not_at<p::one<'\n'>>,p::any>>> {};
struct BlockClose : p::string<'*', '/'> {};
struct BlockComment : p::seq<p::string<'/', '*'>,
    p::star<p::seq<p::not_at<BlockClose>,p::any>>,BlockClose> {};
struct Skip : p::star<p::sor<Whitespace, LineComment, BlockComment>> {};
struct EscapedPair : p::seq<p::one<'\\'>,p::any> {};
struct Quoted : p::seq<p::one<'"'>,
    p::star<p::sor<EscapedPair,p::seq<p::not_at<p::one<'"'>>,p::any>>>,
    p::one<'"'>> {};
struct Open : p::one<'{'> {};
struct Close : p::one<'}'> {};
struct Item;
struct Object : p::seq<Open,Skip,p::star<Item>,Close> {};
struct Item : p::seq<Quoted,Skip,p::sor<Quoted,Object>,Skip> {};
struct BOM : p::string<'\xEF','\xBB','\xBF'> {};
struct UnclosedBlockTail : p::seq<p::string<'/','*'>,p::star<p::any>> {};
struct Document : p::seq<p::opt<BOM>,Skip,Quoted,Skip,Object,Skip,p::eof> {};
// The legacy behavioral reader treated an unterminated trailing /*... as EOF;
// unlike runtime config, preserve that historically observable acceptance.
struct BehavioralDocument : p::seq<p::opt<BOM>,Skip,Quoted,Skip,Object,Skip,
                                    p::opt<UnclosedBlockTail>,p::eof> {};
struct Tracker {
    const char* base;
    const Visitor& visitor;
    Span path[33]{};
    Span pending{};
    size_t depth=0;
    bool has_pending=false;
    Result failure=Result::invalid;
    bool equals(Span s,const char* literal,size_t n) const noexcept {
        if(s.len!=n) return false;
        for(size_t i=0;i<n;++i) if(base[s.off+i]!=literal[i])return false;
        return true;
    }
    bool quote(const char* first,size_t n) noexcept {
        if(n<2)return false;
        const Span span{static_cast<size_t>(first-base)+1u,n-2u};
        if(!has_pending){pending=span;has_pending=true;return true;}
        if(depth==0)return false;
        if(visitor.on_leaf && !visitor.on_leaf(visitor.user,path+1,depth-1,pending,span)){
            failure=Result::resource;
            return false;
        }
        has_pending=false;
        return true;
    }
    bool open() noexcept {
        if(!has_pending||depth>=33u)return false;
        if(depth==0&&!equals(pending,"Config2",7u))return false;
        if(depth==1u && visitor.on_section && !visitor.on_section(visitor.user,pending)){
            failure=Result::resource;
            return false;
        }
        path[depth++]=pending;
        has_pending=false;
        return true;
    }
    bool close() noexcept {
        if(has_pending||!depth)return false;
        --depth;
        return true;
    }
};
template<typename Rule> struct Action : p::nothing<Rule> {};
template<> struct Action<Quoted> {
    template<typename Input> static bool apply(const Input& in,Tracker& st) noexcept {
        return st.quote(in.begin(),in.size());
    }
};
template<> struct Action<UnclosedBlockTail> {
    template<typename Input> static bool apply(const Input& in,Tracker&) noexcept {
        // Legacy behavioral scanner stops at the final byte when /* has no
        // terminator. A following token scan can reach EOF only if that byte
        // is itself whitespace (or the comment has exactly two bytes).
        const size_t n=in.size();
        if(n==2u)return true;
        if(n<2u)return false;
        const char last=in.begin()[n-1];
        return last==' '||last=='\t'||last=='\n'||last=='\r';
    }
};
template<> struct Action<Open> {
    template<typename Input> static bool apply(const Input&,Tracker& st) noexcept {return st.open();}
};
template<> struct Action<Close> {
    template<typename Input> static bool apply(const Input&,Tracker& st) noexcept {return st.close();}
};
} // namespace
Result parse(const char* bytes,size_t length,const Visitor& visitor,
             bool service_legacy_trailing_unclosed_comment) noexcept {
    if(!bytes||!length)return Result::invalid;
    Tracker tracker{bytes,visitor};
#if defined(__cpp_exceptions)
    try {
#endif
        p::view_input input(bytes,length);
        const bool matched=service_legacy_trailing_unclosed_comment
            ? p::parse<BehavioralDocument,Action>(input,tracker)
            : p::parse<Document,Action>(input,tracker);
        if(!matched) return tracker.failure;
        return tracker.depth==0u && !tracker.has_pending?Result::ok:Result::invalid;
#if defined(__cpp_exceptions)
    } catch (...) {return Result::resource;}
#endif
}
} }
