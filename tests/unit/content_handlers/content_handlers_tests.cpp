#include "preloader_l/handlers/content_handlers.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <limits>
#include <stdexcept>
#include <tuple>

using namespace preloader_l::content_handlers_recovered;
static unsigned checks;
static void require(bool value) {++checks;if(!value)throw std::runtime_error("check "+std::to_string(checks));}
struct ThreadError {int code;};
struct WriterError {};
struct WriteError {};
struct Element {
    std::string name;
    std::vector<std::pair<std::string,std::string>> attributes;
    std::string get(std::string_view key)const {
        for(const auto& [k,v]:attributes)if(k==key)return v;
        throw std::runtime_error("missing attribute "+std::string(key));
    }
};
struct Record {
    std::vector<Element> elements;
    std::vector<std::size_t> stack;
    std::vector<std::string> trace;
    int closed=0;
};
struct MockXml final:Xml {
    std::shared_ptr<Record> record=std::make_shared<Record>();
    int fail_attribute=-1;
    int writes=0;
    std::function<void()> after_open;
    const char* attribute_002cc0(preloader_l::handlers_recovered::Request,const char*)override {
        throw std::runtime_error("request snapshot should be used");
    }
    void open_004080(const char* name,bool compact)override {
        require(!compact);record->elements.push_back({name,{}});record->stack.push_back(record->elements.size()-1);
        record->trace.push_back("open:"+std::string(name));if(after_open)after_open();
    }
    void attribute_005560(const char* key,const char* value)override {
        require(!record->stack.empty()&&key&&value);
        if(writes++==fail_attribute)throw WriterError{};
        record->elements[record->stack.back()].attributes.emplace_back(key,value);
        record->trace.push_back("attr:"+std::string(key));
    }
    void close_virtual_48(bool compact)override {
        require(!compact&&!record->stack.empty());record->stack.pop_back();++record->closed;
        record->trace.push_back("close");
    }
    std::vector<Element> named(std::string_view name)const {
        std::vector<Element> result;for(const auto& e:record->elements)if(e.name==name)result.push_back(e);return result;
    }
};
struct MockMutex final:Mutex {
    std::int32_t count=1;
    int result=0,locks=0,unlocks=0;
    std::vector<int> errors;
    bool held=false;
    int lock_04a6ac()override {++locks;if(result==0)held=true;return result;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override {require(held);held=false;++unlocks;}
    [[noreturn]] void thread_error_04a648(int code)override{errors.push_back(code);throw ThreadError{code};}
};
struct MockQueue final:EventQueue {
    MockMutex mutex;
    bool initialized=false;
    std::vector<std::string> entries;
    int initializations=0;
    Mutex& mutex_02c620()override{return mutex;}
    bool initialized_locked()const override{require(mutex.held);return initialized;}
    void initialize_locked(std::string initial)override {
        require(mutex.held&&!initialized);initialized=true;++initializations;entries.push_back(std::move(initial));
    }
    void push_persisted_locked(std::string value,const std::function<void()>& persist)override {
        push_locked_034660(std::move(value));try{persist();}catch(...){entries.pop_back();throw;}
    }
    void push_locked_034660(std::string payload)override {require(mutex.held&&initialized);entries.push_back(std::move(payload));}
};
struct MockDocument final:DocumentWriter {
    MockXml writer;
    std::string payload;
    Xml& xml()override{return writer;}
    std::string c_string_without_nul()const override {require(writer.record->stack.empty());return payload;}
};
struct MockEffects final:Effects {
    std::vector<std::string> diagnostics,atoi_inputs,paths_widened;
    std::vector<Path> online_paths,utf8_paths;
    std::vector<std::pair<Path,Path>> copies;
    std::vector<std::pair<Path,std::string>> files;
    std::vector<std::shared_ptr<Record>> documents;
    std::vector<AchievementMap> steam_snapshots;
    std::vector<int> saved_logs;
    std::int32_t atoi_value=0;
    bool avatar_success=false,copy_success=false,write_failure=false;
    unsigned times=0;
    MockMutex* achievement_mutex=nullptr;
    std::function<void()> time_hook;
    std::int32_t atoi_05ee08(std::string_view value)override {atoi_inputs.emplace_back(value);return atoi_value;}
    void unknown_entitlement_field(std::string_view value)override {diagnostics.emplace_back(value);}
    std::string utc_time_041b60(std::uint64_t& tick)override {
        require(achievement_mutex&&achievement_mutex->held);++times;tick=12000+times;
        if(time_hook)time_hook();return "2040-01-02T03:04:"+std::to_string(times);
    }
    std::unique_ptr<DocumentWriter> printer_0019b0()override {
        require(achievement_mutex&&achievement_mutex->held);
        auto p=std::make_unique<MockDocument>();p->payload="synthetic-printer-"+std::to_string(documents.size());
        documents.push_back(p->writer.record);return p;
    }
    void write_xml_03c1d0(const Path& path,std::string_view bytes)override {
        require(achievement_mutex&&achievement_mutex->held);if(write_failure)throw WriteError{};
        files.emplace_back(path,bytes);
    }
    void save_steam_01f9a0(const AchievementMap& values)override {
        require(achievement_mutex&&achievement_mutex->held);steam_snapshots.push_back(values);
    }
    Path filesystem_widen_006d70(std::string_view text)override {paths_widened.emplace_back(text);return Path(text.begin(),text.end());}
    Path path_append_016b50(const Path& a,const Path& b)override{return a+u"\\"+b;}
    bool online_avatar_virtual_28(const Path& path)override{online_paths.push_back(path);return avatar_success;}
    bool copy_overwrite_048d20(const Path& a,const Path& b)override{copies.emplace_back(a,b);return copy_success;}
    std::string path_to_utf8_031ca0(const Path& path)override{utf8_paths.push_back(path);return std::string(path.begin(),path.end());}
    void log_saved_size_085530(std::int32_t size)override{saved_logs.push_back(size);}
};
struct Fixture {
    std::vector<preloader_l::configuration::Content> contents{{"C2","Second","v2","TEST_STATE"},{"C1","First","v1","INSTALLED"}};
    std::vector<preloader_l::configuration::Entitlement> entitlements{{"itemA","group1","verA","typeA","tagA"},{"itemB","group2","verB","typeB","tagB"},{"itemC","group1","verC","typeC","tagC"}};
    preloader_l::handlers_recovered::Attributes game{{"DISPLAY_NAME","Synthetic Game"}};
    AchievementMap achievements{{"z",{3,0,0,"old","Name Z","Steam Z",33}},{"a",{5,1,5,"won","Name A","Steam A",44}}};
    std::string set="TEST_SET",content_id="TEST_CONTENT",user_id="user-test";
    Path achievement_file=u"synthetic-achievements.xml",avatar=u"synthetic-avatar.png",default_avatar=u"synthetic-default.png",user_directory=u"synthetic-user",last_avatar=u"synthetic-last.png";
    std::uint8_t online=0;
    std::int32_t saved_size=0;
    MockMutex mutex;
    MockQueue events;
    MockEffects effects;
    Fixture(){effects.achievement_mutex=&mutex;}
    StateReferences refs(){return{contents,entitlements,game,achievements,set,content_id,user_id,achievement_file,avatar,default_avatar,user_directory,last_avatar,online,saved_size,mutex,events};}
};
static void content_tests() {
    Fixture f;MockXml x;
    require(query_content_030ee0(x,f.refs()));
    require(x.record->elements.front().name=="QueryContentResponse"&&x.record->closed==3&&x.record->stack.empty());
    const auto games=x.named("Game");require(games.size()==2);
    require(games[0].attributes==std::vector<std::pair<std::string,std::string>>{{"progressValue","0"},{"contentID","C2"},{"availableVersion","v2"},{"displayName","Second"},{"state","TEST_STATE"},{"installedVersion","v2"}});
    require(games[1].get("contentID")=="C1"&&f.mutex.locks==0);
    f.contents.clear();MockXml empty;require(query_content_030ee0(empty,f.refs()));require(empty.record->elements.size()==1&&empty.record->closed==1);
}
static void entitlement_tests() {
    Fixture f;MockXml all;
    require(query_entitlements_031030({},all,f.refs(),f.effects));
    auto rows=all.named("Entitlement");require(rows.size()==3&&all.record->closed==4);
    require(rows[0].attributes==std::vector<std::pair<std::string,std::string>>{{"ItemId","itemA"},{"Type","typeA"},{"EntitlementId","1000000000001"},{"EntitlementTag","tagA"},{"Group","group1"},{"ResourceId",""},{"UseCount","0"},{"Expiration","0000-00-00T00:00:00"},{"GrantDate","0000-00-00T00:00:00"},{"LastModifiedDate","0000-00-00T00:00:00"},{"Version","verA"}});
    require(rows[2].get("EntitlementId")=="1000000000003");
    for(const auto& name:{"OfferId","ItemId"}) {
        RequestTree q{{{name,"itemB"}}, {}};MockXml x;
        require(query_entitlements_031030(q,x,f.refs(),f.effects));rows=x.named("Entitlement");
        require(rows.size()==1&&rows[0].get("EntitlementId")=="1000000000002");
    }
    MockXml group;require(query_entitlements_031030({{{"Group","group1"}},{}},group,f.refs(),f.effects));
    rows=group.named("Entitlement");require(rows.size()==2&&rows[1].get("EntitlementId")=="1000000000003");
    MockXml by_tag;
    require(query_entitlements_031030({{},{{"FilterItems","tagC"}}},by_tag,f.refs(),f.effects));
    require(by_tag.named("Entitlement").size()==1&&by_tag.named("Entitlement")[0].get("ItemId")=="itemC");
    MockXml wrong_items;
    require(query_entitlements_031030({{},{{"FilterItems","itemC"}}},wrong_items,f.refs(),f.effects));
    require(wrong_items.named("Entitlement").empty());
    RequestTree combined{{{"OfferId","itemB"},{"Group","group1"}},{{"FilterItems","not-present"}}};
    MockXml union_result;require(query_entitlements_031030(combined,union_result,f.refs(),f.effects));require(union_result.named("Entitlement").size()==3);
    RequestTree unusual{{{"UserId","ignored"},{"includeChildGroups","true"},{"includeExpiredTrialDLC","false"},{"version","99"},{"Unknown","value"},{"unknownEmpty",""},{"offerid","bad"}},{{"FilterOffers","itemA"},{"FilterOffers","itemA"},{"FilterGroups","group2"},{"UnknownChild","text"},{"EmptyChild",""},{std::nullopt,"text"},{"NoText",std::nullopt}}};
    const auto filters=parse_entitlement_filters_02c6a0(unusual,f.effects);
    require(filters.offers.size()==1&&filters.offers.contains("itemA")&&filters.groups.contains("group2")&&filters.items.empty());
    require(f.effects.diagnostics==std::vector<std::string>{"QueryEntitlements unknown attr: Unknown|value","QueryEntitlements unknown attr: offerid|bad","QueryEntitlements unknown child: UnknownChild|text"});
    f.entitlements.clear();MockXml none;require(query_entitlements_031030({},none,f.refs(),f.effects));require(none.record->elements.size()==1);
}
static void achievement_query_tests() {
    Fixture f;
    for(const auto& all_value:{"absent","true","TRUE","1","true ",""}) {
        RequestTree request;if(std::string_view(all_value)!="absent")request.attributes.emplace_back("All",all_value);
        MockXml x;require(query_achievements_030d30(request,x,f.refs()));
        auto a=x.named("Achievement");require(a.size()==(std::string_view(all_value)=="true"?2:1));
        require(a[0].get("Id")=="a"&&a[0].get("Total")=="5"&&a[0].get("Count")=="1"&&a[0].get("Progress")=="5");
        require(a[0].get("GrantDate")=="won"&&a[0].get("Expiration")=="0000-00-00T00:00:00"&&a[0].get("ImageId").empty());
        require(a[0].get("Description").empty()&&a[0].get("HowTo").empty());
        require(x.record->elements[0].name=="AchievementSets"&&x.record->elements[1].name=="AchievementSet");
        require(x.record->elements[1].get("GameName")=="Synthetic Game"&&x.record->elements[1].get("Name")=="TEST_SET");
        require(!f.mutex.held&&x.record->stack.empty());
    }
    f.achievements["negative"]={-10,-2,-8,"time","minus","",0};
    MockXml negative;require(query_achievements_030d30({},negative,f.refs()));require(negative.named("Achievement").size()==2);
    f.game.clear();MockXml inserted;require(query_achievements_030d30({},inserted,f.refs()));require(f.game.contains("DISPLAY_NAME")&&f.game["DISPLAY_NAME"].empty());
    MockXml throwing;throwing.fail_attribute=0;const auto unlocks=f.mutex.unlocks;
    try{query_achievements_030d30({},throwing,f.refs());require(false);}catch(const WriterError&){}
    require(f.mutex.unlocks==unlocks+1&&!f.mutex.held);
    Fixture failed;failed.mutex.result=4;MockXml untouched;
    try{query_achievements_030d30({},untouched,failed.refs());require(false);}catch(const ThreadError& e){require(e.code==5);}
    require(untouched.record->elements.empty()&&failed.mutex.unlocks==0);
    Fixture overflow;overflow.mutex.count=std::numeric_limits<std::int32_t>::max();
    try{query_achievements_030d30({},untouched,overflow.refs());require(false);}catch(const ThreadError& e){require(e.code==6);}
    require(overflow.mutex.count==std::numeric_limits<std::int32_t>::max()-1&&overflow.mutex.unlocks==0);
}
static void grant_tests() {
    Fixture f;MockXml absent;require(!grant_achievement_030010({},absent,f.refs(),f.effects));
    require(f.mutex.locks==0&&f.effects.atoi_inputs.empty()&&absent.record->elements.empty());
    RequestTree grant{{{"AchievementId","z"}}, {}};MockXml one;
    require(grant_achievement_030010(grant,one,f.refs(),f.effects));
    auto& z=f.achievements.at("z");require(z.field00==3&&z.field04==0&&z.field08==1&&z.field70==12001&&z.timestamp=="2040-01-02T03:04:1");
    require(f.events.entries.empty()&&f.effects.times==1&&f.effects.files.size()==1&&f.effects.steam_snapshots.size()==1);
    require(f.effects.documents.size()==1&&f.effects.documents[0]->elements.front().name=="Achievements");
    require(f.effects.documents[0]->elements[0].get("ContentId")=="TEST_CONTENT"&&f.effects.documents[0]->elements.size()==3);
    require(f.effects.documents[0]->elements[2].get("Id")=="z"&&f.effects.documents[0]->elements[2].get("Progress")=="1");
    require(one.named("Achievement")[0].get("Count")=="0"&&one.record->stack.empty());
    f.effects.atoi_value=5;grant.attributes.emplace_back("Progress","synthetic-five");MockXml complete;
    require(grant_achievement_030010(grant,complete,f.refs(),f.effects));
    require(z.field08==3&&z.field04==1&&f.effects.times==2);
    require(f.events.entries==std::vector<std::string>{"<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>","synthetic-printer-1"});
    require(f.events.initializations==1&&f.events.mutex.locks==1&&f.events.mutex.unlocks==1);
    const auto event=f.effects.documents[1];require(event->elements[0].name=="LSX"&&event->elements[1].name=="Event"&&event->elements[1].get("sender")=="EbisuSDK");
    require(event->elements[4].name=="Achievement"&&event->elements[4].get("Id")=="z"&&event->elements[4].get("Count")=="1");
    require(event->closed==5&&event->stack.empty());
    const auto snapshots=f.effects.steam_snapshots.size(),documents=f.effects.documents.size();MockXml repeated;
    require(grant_achievement_030010(grant,repeated,f.refs(),f.effects));
    require(f.effects.times==2&&f.effects.steam_snapshots.size()==snapshots&&f.effects.documents.size()==documents&&f.events.entries.size()==2);
    require(f.mutex.locks==3&&f.mutex.unlocks==3&&!f.mutex.held);
    // Newly inserted unknown ID uses requested increment as Total, not 1.
    RequestTree unknown{{{"AchievementId","new"},{"Progress","five"}}, {}};MockXml fresh;
    require(grant_achievement_030010(unknown,fresh,f.refs(),f.effects));
    const auto& n=f.achievements.at("new");require(n.field00==5&&n.field04==1&&n.field08==5&&n.display_name.empty()&&n.steam_id.empty());
    require(f.events.initializations==1&&f.events.entries.size()==3);
    Fixture empty_id;empty_id.achievement_file.clear();MockXml empty;
    require(grant_achievement_030010({{{"AchievementId",""}},{}},empty,empty_id.refs(),empty_id.effects));
    require(empty_id.achievements.at("").field00==1&&empty_id.achievements.at("").field04==1&&empty_id.effects.files.empty()&&empty_id.effects.steam_snapshots.size()==1);
}
static void grant_edges() {
    Fixture f;f.effects.atoi_value=-4;MockXml negative;
    require(grant_achievement_030010({{{"AchievementId","z"},{"Progress","minus"}},{}},negative,f.refs(),f.effects));
    require(f.achievements.at("z").field08==-4&&f.achievements.at("z").field04==0&&f.events.entries.empty());
    const auto persisted=f.effects.documents.front();require(persisted->elements.back().get("Progress")=="-4");
    MockXml new_negative;require(grant_achievement_030010({{{"AchievementId","negative-new"},{"Progress","minus"}},{}},new_negative,f.refs(),f.effects));
    require(f.achievements.at("negative-new").field00==-4&&f.achievements.at("negative-new").field08==0&&f.effects.times==1);
    f.effects.atoi_value=0;MockXml zero;require(grant_achievement_030010({{{"AchievementId","zero-new"},{"Progress",""}},{}},zero,f.refs(),f.effects));
    require(f.achievements.at("zero-new").field00==0&&f.effects.times==1);
    MockXml old_zero;require(grant_achievement_030010({{{"AchievementId","z"},{"Progress","nonnumeric"}},{}},old_zero,f.refs(),f.effects));
    require(f.achievements.at("z").field08==-4&&f.effects.times==2&&f.effects.steam_snapshots.size()==2);
    Fixture overflow;auto& a=overflow.achievements.at("z");a.field00=std::numeric_limits<std::int32_t>::max();a.field08=a.field00-1;overflow.effects.atoi_value=5;MockXml wrapped;
    require(grant_achievement_030010({{{"AchievementId","z"},{"Progress","overflow"}},{}},wrapped,overflow.refs(),overflow.effects));
    require(a.field08==std::numeric_limits<std::int32_t>::min()+3&&a.field04==0&&overflow.events.entries.empty());
    Fixture write_failed;write_failed.effects.write_failure=true;MockXml no_response;
    try{grant_achievement_030010({{{"AchievementId","z"}},{}},no_response,write_failed.refs(),write_failed.effects);require(false);}catch(const WriteError&){}
    require(write_failed.achievements.at("z").field08==0&&!write_failed.mutex.held&&write_failed.effects.steam_snapshots.empty()&&no_response.record->elements.empty());
    Fixture queue_failed;queue_failed.effects.atoi_value=3;queue_failed.events.mutex.result=4;
    try{grant_achievement_030010({{{"AchievementId","z"},{"Progress","three"}},{}},no_response,queue_failed.refs(),queue_failed.effects);require(false);}catch(const ThreadError& e){require(e.code==5);}
    require(queue_failed.achievements.at("z").field04==0&&!queue_failed.mutex.held&&queue_failed.effects.files.empty()&&queue_failed.effects.steam_snapshots.empty());
}
static RequestTree image(std::string width="416",std::string owner="user-test") {
    return{{{"Width",std::move(width)},{"Height","odd-height"},{"ImageId","avatar:"+owner}}, {}};
}
static void image_tests() {
    Fixture f;
    for(const auto& absent:{"Width","Height","ImageId"}) {
        auto q=image();std::erase_if(q.attributes,[&](const auto& a){return a.first==absent;});MockXml x;
        require(!query_image_0314f0(q,x,f.refs(),f.effects)&&x.record->elements.empty());
    }
    require(f.effects.atoi_inputs.empty()&&f.effects.utf8_paths.empty());
    MockXml own;require(query_image_0314f0(image(),own,f.refs(),f.effects));
    require(own.named("Image")[0].attributes==std::vector<std::pair<std::string,std::string>>{{"Width","416"},{"Height","odd-height"},{"ResourcePath","synthetic-avatar.png"},{"ImageId","avatar:user-test"}});
    MockXml other;require(query_image_0314f0(image("416","other"),other,f.refs(),f.effects));require(other.named("Image")[0].get("ResourcePath")=="synthetic-default.png");
    f.online=1;RequestTree no_colon{{{"Width","416"},{"Height","0"},{"ImageId","unqualified"}}, {}};MockXml plain;
    require(query_image_0314f0(no_colon,plain,f.refs(),f.effects));require(plain.named("Image")[0].get("ResourcePath")=="synthetic-avatar.png"&&f.effects.online_paths.empty());
    f.effects.atoi_value=416;MockXml failed;require(query_image_0314f0(image(),failed,f.refs(),f.effects));
    require(failed.named("Image")[0].get("ResourcePath")=="synthetic-avatar.png"&&f.effects.copies.empty());
    require(f.effects.paths_widened.back()=="_AVATAR_user-test_416"&&f.effects.online_paths.back()==u"synthetic-user\\_AVATAR_user-test_416");
    f.effects.avatar_success=true;f.effects.copy_success=false;MockXml copy_fail;
    require(query_image_0314f0(image(),copy_fail,f.refs(),f.effects));require(copy_fail.named("Image")[0].get("ResourcePath")=="synthetic-user\\_AVATAR_user-test_416"&&f.saved_size==0&&f.effects.saved_logs.empty());
    f.effects.copy_success=true;MockXml successful;
    require(query_image_0314f0(image(),successful,f.refs(),f.effects));require(f.saved_size==2&&f.effects.saved_logs==std::vector<int>{2});
    require(f.effects.copies.back()==std::pair<Path,Path>{u"synthetic-user\\_AVATAR_user-test_416",u"synthetic-last.png"});
    const auto copies=f.effects.copies.size();MockXml same_size;
    require(query_image_0314f0(image(),same_size,f.refs(),f.effects));require(f.effects.copies.size()==copies);
    f.effects.atoi_value=832;MockXml another;
    require(query_image_0314f0(image("832","other"),another,f.refs(),f.effects));require(f.effects.copies.size()==copies&&f.saved_size==2);
    require(another.named("Image")[0].get("ResourcePath")=="synthetic-user\\_AVATAR_other_832");
    f.effects.atoi_value=-417;MockXml negative;require(query_image_0314f0(image("-417"),negative,f.refs(),f.effects));require(f.saved_size==2&&f.effects.copies.size()==copies);
    // Exact original text is used in cache filename, no normalization of width.
    f.effects.atoi_value=416;MockXml leading;
    require(query_image_0314f0(image("00416"),leading,f.refs(),f.effects));require(f.effects.paths_widened.back()=="_AVATAR_user-test_00416");
    RequestTree multi{{{"Width",""},{"Height",""},{"ImageId","ignored:a:b"}}, {}};MockXml colon;
    require(query_image_0314f0(multi,colon,f.refs(),f.effects));require(f.effects.paths_widened.back()=="_AVATAR_a:b_"&&colon.named("Image")[0].get("Width").empty());
    require(f.mutex.locks==0&&f.events.mutex.locks==0);
}
int main() {
    try{content_tests();entitlement_tests();achievement_query_tests();grant_tests();grant_edges();image_tests();std::printf("PASS: %u synthetic content/entitlement/achievement/image checks; original binaries were not loaded or executed.\n",checks);return 0;}
    catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
}
