// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <tao/pegtl.hpp>

// Rules reconstructed from the documented owner-supplied Config2/VDF evidence.
// This is the recovered Config2/VDF byte grammar, not a generic Valve VDF spec.
// Actions and the owned ParseError mapping live in src/config/parser.cpp.
namespace preloader_l::parser::vdf {
namespace peg = tao::pegtl;

// 013640/0141a0: unknown escapes fall through as literal bytes. No UTF-8 check.
struct EscapedByte : peg::seq<peg::one<'\\'>,peg::one<'"','\\'>> {};
struct RegularByte : peg::not_one<'\r','\n'> {};
struct Character : peg::sor<EscapedByte,RegularByte> {};
// 013810: the closing quote is tested before Character; EOF/CR/LF must throw.
struct Quoted : peg::seq<peg::one<'"'>,
    peg::until<peg::one<'"'>,peg::must<Character>>> {};
struct Key : Quoted {};
struct Value : Quoted {};
struct ObjectName : Quoted {};

struct LineEnding : peg::sor<peg::one<'\n'>,peg::string<'\r','\n'>> {};
// 013460: consume the line ending, or finish successfully at EOF.
struct LineComment : peg::seq<peg::string<'/','/'>,
    peg::until<peg::sor<LineEnding,peg::eof>>> {};
// 013ed0: separate opener bytes preserve the consumed '/' on a direct failure.
// A surrounding required rewind guard restores input, but never action State.
struct BlockComment : peg::seq<peg::one<'/'>,peg::one<'*'>,
    peg::until<peg::string<'*','/'>>> {};
struct Separator : peg::sor<peg::ascii::space,LineComment,BlockComment> {};
struct Separators : peg::plus<Separator> {};

struct Object;
struct KeyValue : peg::seq<Key,Separators,Value> {};
// 0138e0: scalar alternative precedes object, including its action side effects.
struct Member : peg::sor<KeyValue,Object> {};
// 013cc0: the outer until permits adjacent members. The scalar's own separator
// is still mandatory. A body mismatch raises the owned object_members error.
struct ObjectMembers : peg::until<peg::one<'}'>,
    peg::sor<peg::list<Member,Separators>,Separators>> {};
struct Object : peg::seq<ObjectName,peg::star<Separator>,peg::one<'{'>,
    peg::must<ObjectMembers>> {};
// 014310: root accepts objects and separators; invalid input throws, not false.
struct Document : peg::until<peg::eof,
    peg::sor<LineEnding,Separator,Object,peg::raise<Document>>> {};
} // namespace preloader_l::parser::vdf
