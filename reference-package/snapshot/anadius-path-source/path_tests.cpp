#include "paths.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace anadius::path_recovered;
namespace {
int checks;
void check(bool condition, int line) {
    ++checks;
    if (!condition) throw std::runtime_error("path check failed at line " + std::to_string(line));
}
#define CHECK(value) check((value), __LINE__)
struct Case { Path input; std::size_t root; bool absolute; Path filename; Path parent; };
void decomposition() {
    const std::vector<Case> cases{
        {u"",0,false,u"",u""},
        {u"a",0,false,u"a",u""},
        {u".",0,false,u".",u""},
        {u"..",0,false,u"..",u""},
        {u"a/b",0,false,u"b",u"a"},
        {u"a//b",0,false,u"b",u"a"},
        {u"a/",0,false,u"",u"a"},
        {u"a///",0,false,u"",u"a"},
        {u"a/../b",0,false,u"b",u"a/.."},
        {u"/",0,false,u"",u"/"},
        {u"//",0,false,u"",u"//"},
        {u"///",0,false,u"",u"///"},
        {u"/a",0,false,u"a",u"/"},
        {u"/a/b",0,false,u"b",u"/a"},
        {u"///a",0,false,u"a",u"///"},
        {u"C:",2,false,u"",u"C:"},
        {u"c:a",2,false,u"a",u"c:"},
        {u"C:foo/bar",2,false,u"bar",u"C:foo"},
        {u"C:/",2,true,u"",u"C:/"},
        {u"C:///",2,true,u"",u"C:///"},
        {u"C:/foo",2,true,u"foo",u"C:/"},
        {u"C:/foo/",2,true,u"",u"C:/foo"},
        {u"1:foo",0,false,u"1:foo",u""},
        {u"@:x",0,false,u"@:x",u""},
        {u"[:x",0,false,u"[:x",u""},
        {u"//server",8,true,u"",u"//server"},
        {u"//server/",8,true,u"",u"//server/"},
        {u"//server/share",8,true,u"share",u"//server/"},
        {u"//server/share/file",8,true,u"file",u"//server/share"},
        {u"//server//share",8,true,u"share",u"//server//"},
        {uR"(\a)",0,false,u"a",uR"(\)"},
        {uR"(a\b)",0,false,u"b",u"a"},
        {uR"(\\s\share)",3,true,u"share",uR"(\\s\)"},
        {uR"(\\?\C:\a)",3,true,u"a",uR"(\\?\C:)"},
        {uR"(\\.\pipe\name)",3,true,u"name",uR"(\\.\pipe)"},
        {uR"(\??\C:\a)",3,true,u"a",uR"(\??\C:)"},
        {uR"(\??\)",3,true,u"",uR"(\??\)"},
        {uR"(\??\\a)",0,false,u"a",uR"(\??)"},
        {uR"(/\?\a)",3,true,u"a",uR"(/\?\)"},
        {uR"(\\?)",3,true,u"",uR"(\\?)"},
        {uR"(\\?\\a)",3,true,u"a",uR"(\\?\\)"},
        {Path{u'a',0,u'b',u'/',u'c'},0,false,u"c",Path{u'a',0,u'b'}},
        {Path{u'C',u':',0,u'x'},2,false,Path{0,u'x'},u"C:"},
        {Path{u'/',u'/',0,u'x'},4,true,u"",Path{u'/',u'/',0,u'x'}},
        {Path{char16_t(0xd800),u'/',char16_t(0xdc00)},0,false,
            Path{char16_t(0xdc00)},Path{char16_t(0xd800)}},
    };
    for (const auto& item : cases) {
        CHECK(root_name_length_00bb30(item.input) == item.root);
        CHECK(is_absolute_0195b0(item.input) == item.absolute);
        CHECK(filename_00e120(item.input) == item.filename);
        CHECK(parent_path_019820(item.input) == item.parent);
    }
}
void joining() {
    struct Join { Path left, right, expected; };
    const std::vector<Join> cases{
        {u"",u"",u""}, {u"",u"a",u"a"}, {u"a",u"",uR"(a\)"},
        {u"a",u"b",uR"(a\b)"}, {u"a/",u"b",u"a/b"},
        {u"a",u"/b",u"/b"}, {u"a",u"//b",u"//b"},
        {u"a/b",u"../c",uR"(a/b\../c)"},
        {u"C:",u"a",u"C:a"}, {u"C:a",u"b",uR"(C:a\b)"},
        {u"C:a",u"C:b",uR"(C:a\b)"},
        {u"C:a",u"c:b",u"c:b"}, {u"C:a",u"D:b",u"D:b"},
        {u"C:a",u"/b",u"C:/b"}, {u"C:/a",u"/b",u"C:/b"},
        {u"C:/a",uR"(\b)",uR"(C:\b)"},
        {u"C:/a",u"D:/b",u"D:/b"},
        {u"C:/a",u"C:/b",u"C:/b"},
        {u"C:/a",u"C:",uR"(C:/a\)"},
        {u"//s",u"a",uR"(//s\a)"},
        {u"//s/a",u"/b",u"//s/b"},
        {u"//s/a",u"//s",u"//s"},
        {uR"(\\?\C:\a)",u"b",uR"(\\?\C:\a\b)"},
        {uR"(\\?\C:\a)",u"/b",uR"(\\?/b)"},
        {u"/",u"a",u"/a"}, {u"C:/",u"a",u"C:/a"},
        {u"C:/a",u"a/b",uR"(C:/a\a/b)"},
        {Path{u'a',0,u'b'},u"x",Path{u'a',0,u'b',u'\\',u'x'}},
        {u"C:/a",Path{0,u'x'},Path{u'C',u':',u'/',u'a',u'\\',0,u'x'}},
        {Path(80,u'a'),Path(80,u'b'),Path(80,u'a') + u"\\" + Path(80,u'b')},
    };
    for (const auto& item : cases) {
        CHECK(join_016b00(item.left,item.right) == item.expected);
        auto left = item.left;
        const auto right = item.right;
        CHECK(&append_016b50(left,right) == &left);
        CHECK(left == item.expected);
        CHECK(right == item.right);
    }
    for (const Path& input : {Path{},Path(u"C:"),Path(u"a/"),Path(u"C:/a"),Path(u"//s")}) {
        Path self(input);
        const Path expected = input == u"a/" ? Path(u"a/a/") : input;
        CHECK(&append_016b50(self,self) == &self);
        CHECK(self == expected);
    }
    for (const Path& input : {Path(u"a"),Path(80,u'a')}) {
        auto self = input;
        bool rejected = false;
        try { append_016b50(self,self); } catch (const OutOfEvidenceAliasing&) { rejected = true; }
        CHECK(rejected);
        CHECK(self == input);
        // The non-aliasing value-returning operator remains defined.
        CHECK(join_016b00(input,input) == input + u"\\" + input);
    }
    for (const Path& input : {Path(u"/a"),Path(u"/"),Path(u"///x"),Path(u"\\x")}) {
        auto self = input;
        auto expected = input;
        expected[0] = 0;
        append_016b50(self,self);
        CHECK(self == expected);
        CHECK(self.size() == input.size());
        CHECK(join_016b00(input,input) == input);
    }
}
}
int main() {
    try { decomposition(); joining(); std::cout << checks << " deterministic path checks PASS\n"; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
