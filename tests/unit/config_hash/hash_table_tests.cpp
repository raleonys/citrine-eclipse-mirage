// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/hash_table.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <list>
#include <optional>

using namespace preloader_l;
namespace {
std::size_t checks=0;
void require(bool value,const char* message) {++checks;if(!value)throw std::runtime_error(message);}
template<class E,class F> void rejects(F f,const char* label) {
    bool hit=false;try {f();}catch(const E&){hit=true;}require(hit,label);
}
// Independent pointer/list-splice oracle for the admitted unique-key branch.
// It preserves the next unprocessed iterator before each splice, like017570.
struct Oracle {
    std::uint64_t capacity=8;
    std::list<std::string> rows;
    std::uint64_t bucket(std::string_view key)const {
        std::uint64_t h=14695981039346656037ULL;
        for(const unsigned char c:key){h^=c;h*=1099511628211ULL;}
        return h&(capacity-1);
    }
    void rehash(std::uint64_t n) {
        capacity=n;
        std::map<std::uint64_t,std::list<std::string>::iterator> first;
        for(auto current=rows.begin();current!=rows.end();) {
            auto next=std::next(current);
            const auto index=bucket(*current);
            const auto existing=first.find(index);
            if(existing==first.end())first.emplace(index,current);
            else {rows.splice(existing->second,rows,current);existing->second=current;}
            current=next;
        }
    }
    bool insert(std::string key) {
        if(std::find(rows.begin(),rows.end(),key)!=rows.end())return false;
        if(rows.size()+1>capacity)rehash(capacity<512?capacity*8:capacity*2);
        const auto index=bucket(key);
        auto before=std::find_if(rows.begin(),rows.end(),[&](const auto& old){return bucket(old)==index;});
        rows.insert(before,std::move(key));return true;
    }
    void assign(const Oracle& source) {
        if(this==&source)return;
        auto requested=std::max<std::uint64_t>(8,source.rows.size());
        if(requested<=capacity)requested=capacity;
        else if(capacity<512 && requested<=capacity*8)requested=capacity*8;
        std::uint64_t rounded=1;while(rounded<requested)rounded*=2;
        rows=source.rows;rehash(rounded);
    }
    std::vector<std::string> order()const {return {rows.begin(),rows.end()};}
};
void growth_and_oracle() {
    using namespace config_hash;
    require(fnv1a64("")==0xcbf29ce484222325ULL,"FNV seed");
    require(fnv1a64("foobar")==0x85944171f73967e8ULL,"FNV anchor");
    require(fnv1a64(std::string(1,static_cast<char>(0xff)))==((0xcbf29ce484222325ULL^255ULL)*0x100000001b3ULL),"unsigned high byte");
    require(fnv1a64(std::string("a\0i",3))!=fnv1a64("a"),"embedded NUL retained");
    for(const auto& [capacity,count,expected]:std::vector<std::array<std::uint64_t,3>>{
        {8,0,8},{8,8,8},{8,9,64},{8,64,64},{8,65,65},{64,65,512},
        {512,513,513},{1024,1,1024},{1ULL<<24,(1ULL<<24)+1,1ULL<<24}})
        require(growth_target(capacity,count)==expected,"growth target binary32/current/eightfold");
    require(rounded_buckets(65)==128 && rounded_buckets(513)==1024,"rehash round-up");
    require(rounded_buckets(max_native_buckets)==max_native_buckets,"maximum admitted bucket target");
    rejects<std::length_error>([]{(void)rounded_buckets(max_native_buckets+1);},"bucket overflow rejected");
    rejects<std::length_error>([]{(void)growth_target(8,max_native_size+1);},"native max_size rejected");
    rejects<std::invalid_argument>([]{(void)growth_target(9,1);},"nonpower capacity rejected");
    rejects<std::invalid_argument>([]{(void)rounded_buckets(0);},"outside observed minimum8 request rejected by model");
    Table actual;Oracle oracle;
    for(unsigned i=0;i<1100;++i) {
        const auto key="synthetic-key-"+std::to_string(i);
        require(actual.insert_unique(key)==oracle.insert(key),"unique insert boolean");
        require(actual.bucket_count==oracle.capacity,"capacity matches independent oracle");
        require(actual.traversal==oracle.order(),"list ordering matches independent splice oracle");
        if(i==7)require(actual.bucket_count==8,"eight rows capacity8");
        if(i==8)require(actual.bucket_count==64,"ninth row capacity64");
        if(i==64)require(actual.bucket_count==512,"sixty-fifth row capacity512");
        if(i==512)require(actual.bucket_count==1024,"513th row capacity1024");
        if(i==1024)require(actual.bucket_count==2048,"1025th row capacity2048");
        if(i%23==0) {
            const auto previous=actual.traversal;const auto capacity=actual.bucket_count;
            require(!actual.insert_unique(key),"duplicate returns existing");
            require(actual.bucket_count==capacity && actual.traversal==previous,"duplicate no growth/no move");
        }
    }
    for(unsigned n:std::array<unsigned,9>{0,1,8,9,64,65,512,513,1100}) {
        Table source;Oracle source_oracle;
        for(unsigned i=0;i<n;++i){const auto key="copy-"+std::to_string(i);source.insert_unique(key);source_oracle.insert(key);}
        Table fresh;Oracle fresh_oracle;fresh.assign_from(source);fresh_oracle.assign(source_oracle);
        require(fresh.bucket_count==fresh_oracle.capacity && fresh.traversal==fresh_oracle.order(),"fresh assignment oracle");
        actual.assign_from(source);oracle.assign(source_oracle);
        require(actual.bucket_count==2048,"small assignment retains destination capacity");
        require(actual.traversal==oracle.order(),"reused assignment oracle");
        auto cloned=Table::clone(source);Oracle cloned_oracle;cloned_oracle.capacity=source_oracle.capacity;
        for(const auto& key:source_oracle.rows)cloned_oracle.insert(key);
        require(cloned.bucket_count==source.bucket_count && cloned.traversal==cloned_oracle.order(),"clone inherits source capacity and reinserts");
        const auto old=actual.traversal;actual.assign_from(actual);
        require(actual.traversal==old,"self assignment skips rehash");
    }
    Table duplicate;duplicate.traversal={"same","same"};
    rejects<std::invalid_argument>([&]{duplicate.validate();},"duplicate logical table rejected");
}
config_hash::Node parsed(std::string_view bytes) {
    parser::State state;require(parser::parse(bytes,state),"synthetic parser succeeds");
    return config_hash::from_parser(*state.root.find("Config2"));
}
void recursive_assignment() {
    using namespace config_hash;
    auto source=parsed(R"("Config2" {"a" {"a" "1" "b" "2" "i" "3" "j" "4"} "b" "value" "i" "other"})");
    Node selected;selected.assign_from(source);
    require(selected.table.traversal==std::vector<std::string>{"a","i","b"},"root copied order");
    require(selected.children.at("a").table.traversal==std::vector<std::string>{"a","i","b","j"},"nested copied order");
    const auto converted=to_configuration(selected);
    source=Node{};
    require(converted.child("a").text("i")=="3","owned configuration conversion");
    // Old first destination row has a large child table. A different source key
    // replaces that POSITION, keeping its capacity even when its name changes.
    Node reused;
    auto& old_first=reused.lookup("old-row");
    for(unsigned i=0;i<70;++i)old_first.lookup("old-child-"+std::to_string(i));
    require(old_first.table.bucket_count==512,"old first row owns grown child table");
    reused.lookup("tail-to-remove");
    const auto small=parsed(R"("Config2" {"replacement" {"a" "1" "i" "2" "b" "3" "j" "4"}})");
    reused.assign_from(small);
    require(reused.children.size()==1 && !reused.find("tail-to-remove"),"assignment removes excess destination tail");
    const auto* replacement=reused.find("replacement");
    require(replacement && replacement->table.bucket_count==512,"different key reuses destination slot child capacity");
    require(replacement->table.traversal==std::vector<std::string>{"i","a","j","b"},"retained large capacity changes child copy ordering");
    Node fresh;fresh.assign_from(small);
    require(fresh.children.at("replacement").table.bucket_count==8,"appended fresh row inherits source capacity");
    require(fresh.children.at("replacement").table.traversal==std::vector<std::string>{"a","i","b","j"},"fresh nested copy reverses small bucket collisions");
    auto append=parsed(R"("Config2" {"first" {"a" "1"} "second" {"b" "2" "j" "3"}})");
    reused.assign_from(append);
    require(reused.children.at("first").table.bucket_count==512,"first positional record reused again");
    require(reused.children.at("second").table.bucket_count==8,"new tail recursively cloned");
    require(reused.children.at("second").table.traversal==std::vector<std::string>{"b","j"},"new tail reinserts source traversal");
    const auto old_order=reused.table.traversal;const auto old_child_order=reused.children.at("first").table.traversal;
    reused.assign_from(reused);
    require(reused.table.traversal==old_order && reused.children.at("first").table.traversal==old_child_order,"recursive selfcopy leaves all lists");
    Node missing;auto& empty=missing.lookup("unknown");
    require(empty.name.empty() && empty.value.empty() && empty.table.bucket_count==8,"default inner node empty/fresh8");
    require(missing.table.traversal==std::vector<std::string>{"unknown"},"default outer pair key inserted");
    const auto before=missing.table.traversal;missing.lookup("unknown");
    require(missing.table.traversal==before,"repeated lookup preserves position");
    parser::Node duplicate;parser::Node child;child.name="same";duplicate.children={child,child};
    rejects<std::invalid_argument>([&]{(void)from_parser(duplicate);},"malformed logical duplicate slots rejected");
    parser::Node wide_root;wide_root.name="Config2";
    for(unsigned i=0;i<65;++i){parser::Node row;row.name="key-"+std::to_string(i);wide_root.children.push_back(std::move(row));}
    require(from_parser(wide_root).table.bucket_count==512,"65 parsed rows use incremental512 capacity");
    require(selected_copy(wide_root).table.bucket_count==128,"65 first-copy root rows use bulk128 capacity");
    parser::Node nested;nested.name="Config2";wide_root.name="nested-wide";nested.children.push_back(wide_root);
    const auto copied_nested=selected_copy(nested);
    require(copied_nested.children.at("nested-wide").table.bucket_count==512,"65 cloned descendant rows inherit512 notbulk128");
}
}
int main() {
    try {growth_and_oracle();recursive_assignment();std::cout<<"PASS: "<<checks<<" Config2 growth/copy/positional-reuse checks\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
