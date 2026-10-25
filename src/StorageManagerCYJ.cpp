#include "StorageManagerCYJ.h"
#include "PlatformControllerCYJ.h"
#include "WeChatAccountCYJ.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

namespace {
void completeLine(std::istringstream& in) {
    requireCYJ(!in.fail(),"Missing or invalid fields"); in >> std::ws; requireCYJ(in.eof(),"Unexpected extra fields");
}
}
StorageManagerCYJ::StorageManagerCYJ(std::filesystem::path p):directory_(std::move(p)) {}
const std::filesystem::path& StorageManagerCYJ::getDirectory() const { return directory_; }
void StorageManagerCYJ::saveAll(const PlatformControllerCYJ& p) const {
    p.validateAll(); std::filesystem::create_directories(directory_);
    const auto file=directory_/"platform.txt", tmp=directory_/"platform.txt.tmp", backup=directory_/"platform.txt.bak";
    // One snapshot keeps user/account/friend/group records in the same committed version.
    std::ofstream out(tmp,std::ios::binary|std::ios::trunc); requireCYJ(out.is_open(),"Cannot open temporary snapshot");
    out << "IMS_CYJ 1\n";
    for(const auto& [id,u]:p.users_) {
        out << "USER " << std::quoted(id) << ' ' << std::quoted(u.getNickname()) << ' ' << std::quoted(u.getBirthDate()) << ' ' << std::quoted(u.getRegisterTime()) << ' ' << std::quoted(u.getLocation()) << '\n';
        for(const auto& [type,a]:u.getAccounts()) {
            const auto* wx=dynamic_cast<const WeChatAccountCYJ*>(a.get());
            out << "ACCOUNT " << std::quoted(id) << ' ' << toStringCYJ(type) << ' ' << std::quoted(a->getServiceId()) << ' ' << std::quoted(wx?wx->getBoundQQId():std::string{}) << '\n';
        }
    }
    for(const auto& [key,r]:p.friendManager_.getRelations()) out << "FRIEND " << std::quoted(key.first) << ' ' << std::quoted(key.second) << ' ' << std::quoted(r.getRemark()) << '\n';
    for(const auto& [id,g]:p.groupManager_.getGroups()) {
        out << "GROUP " << std::quoted(id) << ' ' << std::quoted(g.getName()) << ' ' << std::quoted(g.getOwnerAccountId()) << ' ' << toStringCYJ(g.getServiceType()) << ' ' << g.getPolicy().getName() << ' ' << std::quoted(g.getParentGroupId()) << '\n';
        for(const auto& [a,m]:g.getMembers()) out << "MEMBER " << std::quoted(id) << ' ' << std::quoted(a) << ' ' << toStringCYJ(m.getRole()) << '\n';
        for(const auto& a:g.getPendingRequests()) out << "PENDING " << std::quoted(id) << ' ' << std::quoted(a) << '\n';
    }
    out << "END\n"; out.flush(); requireCYJ(out.good(),"Snapshot write failed"); out.close(); requireCYJ(!out.fail(),"Snapshot close failed");
    bool rotated=false;
    if(std::filesystem::exists(file)) {
        std::filesystem::remove(backup); std::filesystem::rename(file,backup); rotated=true;
    }
    try { std::filesystem::rename(tmp,file); }
    catch(...) {
        if(rotated) { std::error_code ec; std::filesystem::rename(backup,file,ec); }
        throw;
    }
    std::error_code ec; std::filesystem::remove(backup,ec);
}
bool StorageManagerCYJ::loadAll(PlatformControllerCYJ& p) const {
    const auto file=directory_/"platform.txt", backup=directory_/"platform.txt.bak";
    auto source=file;
    if(!std::filesystem::exists(source)) {
        if(std::filesystem::exists(backup)) source=backup;
        else { requireCYJ(!std::filesystem::exists(directory_/"platform.txt.tmp"),"Incomplete snapshot found; inspect platform.txt.tmp before starting a new database"); return false; }
    }
    std::ifstream in(source,std::ios::binary); requireCYJ(in.is_open(),"Cannot open snapshot");
    std::string line; std::getline(in,line); if(!line.empty() && line.back()=='\r') line.pop_back(); requireCYJ(line=="IMS_CYJ 1","Unsupported snapshot header/version");
    PlatformControllerCYJ staged(directory_);
    std::vector<std::pair<std::string,std::string>> bindings;
    std::size_t number=1; bool ended=false;
    while(std::getline(in,line)) {
        ++number; if(!line.empty() && line.back()=='\r') line.pop_back();
        try {
            requireCYJ(line.size()<=16384,"Record exceeds size limit");
            requireCYJ(!ended,"Data found after END marker");
            std::istringstream row(line); std::string tag; row>>tag;
            if(tag=="END") { completeLine(row); ended=true; continue; }
            if(tag=="USER") {
                std::string id,n,b,r,l; row>>std::quoted(id)>>std::quoted(n)>>std::quoted(b)>>std::quoted(r)>>std::quoted(l); completeLine(row);
                requireCYJ(staged.users_.emplace(id,UserCYJ(id,n,b,l,r)).second,"Duplicate user");
            } else if(tag=="ACCOUNT") {
                std::string u,t,id,b; row>>std::quoted(u)>>t>>std::quoted(id)>>std::quoted(b); completeLine(row);
                auto type=serviceTypeCYJ(t); staged.openService(u,type,id);
                requireCYJ(type==ServiceTypeCYJ::WeChat || b.empty(),"Unexpected binding on non-WeChat account");
                if(!b.empty()) bindings.emplace_back(u,b);
            } else if(tag=="FRIEND") {
                std::string a,b,r; row>>std::quoted(a)>>std::quoted(b)>>std::quoted(r); completeLine(row);
                requireCYJ(staged.friendManager_.relations_.emplace(std::make_pair(a,b),FriendRelationCYJ(a,b,r)).second,"Duplicate friend record");
            } else if(tag=="GROUP") {
                std::string id,n,o,t,policy,parent; row>>std::quoted(id)>>std::quoted(n)>>std::quoted(o)>>t>>policy>>std::quoted(parent); completeLine(row);
                GroupCYJ g(id,n,o,serviceTypeCYJ(t),GroupPolicyCYJ::create(policy),parent); g.members_.clear();
                requireCYJ(staged.groupManager_.groups_.emplace(id,std::move(g)).second,"Duplicate group");
            } else if(tag=="MEMBER") {
                std::string id,a,r; row>>std::quoted(id)>>std::quoted(a)>>r; completeLine(row);
                auto& g=staged.groupManager_.mutableGroup(id); requireCYJ(g.members_.emplace(a,GroupMemberCYJ(a,groupRoleCYJ(r))).second,"Duplicate member");
            } else if(tag=="PENDING") {
                std::string id,a; row>>std::quoted(id)>>std::quoted(a); completeLine(row);
                requireCYJ(staged.groupManager_.mutableGroup(id).pending_.insert(a).second,"Duplicate pending request");
            } else throw std::runtime_error("Unknown record type");
        } catch(const std::exception& e) { throw std::runtime_error("Snapshot line "+std::to_string(number)+": "+e.what()); }
    }
    requireCYJ(!in.bad() && ended,"Snapshot incomplete: missing END or read error");
    for(const auto& [uid,qqid]:bindings) {
        auto& u=staged.users_.at(uid); const auto* qq=u.getAccount(ServiceTypeCYJ::QQ);
        requireCYJ(qq && qq->getServiceId()==qqid,"Invalid persisted QQ binding");
        dynamic_cast<WeChatAccountCYJ&>(*u.accounts_.at(ServiceTypeCYJ::WeChat)).bindQQ(*qq);
    }
    staged.validateAll();
    // Commit only after every record and cross-reference passes validation.
    p.users_=std::move(staged.users_); p.friendManager_=std::move(staged.friendManager_); p.groupManager_=std::move(staged.groupManager_);
    p.currentUser_.clear(); p.currentService_.reset();
    return true;
}
