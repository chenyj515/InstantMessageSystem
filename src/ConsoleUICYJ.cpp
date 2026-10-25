#include "ConsoleUICYJ.h"
#include "WeChatAccountCYJ.h"
#include <sstream>
#include <vector>
namespace {
struct EndOfInputCYJ {};
}
ConsoleUICYJ::ConsoleUICYJ(PlatformControllerCYJ& c,std::istream& in,std::ostream& out):controller_(c),input_(in),output_(out) {}
std::string ConsoleUICYJ::read(const std::string& p) { output_<<p; std::string s; if(!std::getline(input_,s)) throw EndOfInputCYJ{}; if(!s.empty() && s.back()=='\r') s.pop_back(); return s; }
int ConsoleUICYJ::choice(const std::string& p) { const auto s=read(p); std::istringstream in(s); int n; in>>n; requireCYJ(!in.fail(),"请输入菜单数字"); in>>std::ws; requireCYJ(in.eof(),"请输入菜单数字"); return n; }
bool ConsoleUICYJ::askYes(const std::string& p) {
    const auto a=read(p); requireCYJ(a=="y" || a=="n","请输入 y 或 n"); return a=="y";
}
std::string ConsoleUICYJ::peerOnCurrentService(const std::string& prompt) {
    const auto raw=read(prompt);
    requireCYJ(!raw.empty(),"输入不能为空");
    if(raw.find(':')!=std::string::npos) return raw;
    return accountKeyCYJ(controller_.getCurrentService(),raw);
}
ServiceTypeCYJ ConsoleUICYJ::pickOtherOpenedService(const std::string& prompt) {
    const auto* u=controller_.getCurrentUser(); requireCYJ(u,"请先登录");
    const auto current=controller_.getCurrentService();
    std::vector<ServiceTypeCYJ> others;
    for(const auto& [t,a]:u->getAccounts()) {
        if(t==current) continue;
        others.push_back(t);
        output_<<"  "<<others.size()<<". "<<toStringCYJ(t)<<" 号码 "<<a->getServiceId()<<"（"<<a->getAccountId()<<"）\n";
    }
    requireCYJ(!others.empty(),"没有其它已开通服务可作为来源");
    const int n=choice(prompt);
    requireCYJ(n>=1 && n<=static_cast<int>(others.size()),"无效服务序号");
    return others[static_cast<std::size_t>(n-1)];
}
void ConsoleUICYJ::showUsers() {
    output_<<"统一用户ID识别同一个人；服务号码识别各微X账号。在线不等于当前操作服务。\n";
    bool first=true;
    for(const auto& [id,u]:controller_.getUsers()) {
        if(!first) output_<<"----------------------------------------\n";
        first=false;
        output_<<"统一用户ID: "<<id<<"\n";
        output_<<"昵称: "<<u.getNickname()<<"\n";
        output_<<"出生时间: "<<(u.getBirthDate().empty()?"（未填）":u.getBirthDate())<<"\n";
        output_<<"申请时间: "<<u.getRegisterTime()<<"\n";
        output_<<"T龄: "<<tAgeYearsCYJ(u.getRegisterTime())<<" 年\n";
        output_<<"所在地: "<<(u.getLocation().empty()?"（未填）":u.getLocation())<<"\n";
        if(u.getAccounts().empty()) output_<<"服务账号: （尚未开通）\n";
        else {
            output_<<"服务账号:\n";
            for(const auto& [t,a]:u.getAccounts()) {
                output_<<"  · "<<toStringCYJ(t)<<" 号码 "<<a->getServiceId()<<" | "<<a->getAccountId();
                output_<<(a->isLoggedIn()?" | 在线":" | 离线");
                if(controller_.hasCurrentService() && controller_.getCurrentUser() && controller_.getCurrentUser()->getUserId()==id
                   && controller_.getCurrentService()==t) output_<<" | 当前操作";
                output_<<'\n';
            }
        }
    }
    if(controller_.getUsers().empty()) output_<<"（暂无用户）\n";
}
void ConsoleUICYJ::showGroups() {
    for(const auto& [id,g]:controller_.getGroupManager().getGroups()) output_<<id<<" | "<<g.getName()<<" | 账号服务 "<<toStringCYJ(g.getServiceType())<<" | 当前规则 "<<g.getPolicy().getName()<<" | 群主 "<<g.getOwnerAccountId()<<" | 成员 "<<g.getMembers().size()<<" | 父群 "<<g.getParentGroupId()<<'\n';
}
void ConsoleUICYJ::showMembers() {
    const auto id=read("群号: "); const auto& g=controller_.getGroupManager().findGroup(id);
    for(const auto& [a,m]:g.getMembers()) output_<<a<<" | "<<toStringCYJ(m.getRole())<<'\n';
    for(const auto& a:g.getPendingRequests()) output_<<a<<" | 待审批\n";
}
void ConsoleUICYJ::showLoginStatus() {
    requireCYJ(controller_.hasCurrentService(),"请先登录");
    output_<<"当前操作服务: "<<controller_.getCurrentAccountId()<<"\n";
    output_<<"已开通服务在线状态（在线 ≠ 当前操作）:\n";
    for(const auto& s:controller_.getLoginStatus()) {
        const auto cur=controller_.getCurrentAccountId();
        const bool isCurrent=s.rfind(cur,0)==0;
        output_<<"  "<<s<<(isCurrent?"  ← 当前操作":"")<<'\n';
    }
}
void ConsoleUICYJ::registerWizard() {
    output_<<"\n=== 注册统一用户 ===\n";
    output_<<"两层ID：统一用户ID由系统自动生成；服务号码开通时自行输入，且不得与统一用户ID相同。\n";
    const auto name=read("昵称: ");
    const auto birth=read("出生日期 YYYY-MM-DD（可空）: ");
    const auto location=read("所在地（可空）: ");
    std::string userId;
    controller_.executeCommand([&] { userId=controller_.createUser(name,birth,location); });
    const auto& created=controller_.findUser(userId);
    output_<<"已创建：统一用户ID "<<userId<<" / 昵称 "<<name<<"\n";
    output_<<"系统记录申请时间: "<<created.getRegisterTime()<<"（T龄从此日起算）\n";
    openServiceWizard(userId,true);
    output_<<"\n注册完成。请用菜单「登录」输入 "<<userId<<"，再选择当前操作服务。\n";
}
void ConsoleUICYJ::openServiceWizard(const std::string& userId,bool duringRegister) {
    const auto& user=controller_.findUser(userId);
    output_<<(duringRegister?"\n=== 开通服务 ===\n":"\n=== 追加开通服务 ===\n");
    output_<<"统一用户ID: "<<userId<<" / "<<user.getNickname()<<"（此ID不是 QQ/微信号码）\n";
    output_<<"规则：请自行输入服务号码ID，且必须不同于统一用户ID；QQ 与微博共用号码，微信独立。\n";

    auto openOne=[&](ServiceTypeCYJ type,const std::string& label) {
        if(user.getAccount(type)) { output_<<label<<" 已开通: "<<user.getAccount(type)->getAccountId()<<'\n'; return; }
        if(!askYes("开通 "+label+"? y/n: ")) return;
        std::string sid;
        if(type==ServiceTypeCYJ::Weibo) {
            if(const auto* qq=user.getAccount(ServiceTypeCYJ::QQ)) {
                sid=qq->getServiceId();
                output_<<"微博与 QQ 共享服务号码: "<<sid<<"（统一用户仍为 "<<userId<<"）\n";
            } else {
                sid=read("请添加微博服务ID（不能等于统一用户ID "+userId+"）: ");
            }
        } else {
            sid=read("请添加"+label+"服务ID（不能等于统一用户ID "+userId+"）: ");
        }
        requireCYJ(!sid.empty(),"服务ID不能为空");
        controller_.executeCommand([&] { controller_.openService(userId,type,sid); });
        const auto* opened=controller_.findUser(userId).getAccount(type);
        output_<<"已开通 "<<label<<"：服务号码 "<<opened->getServiceId()<<" → 账号键 "<<opened->getAccountId()<<'\n';
    };

    openOne(ServiceTypeCYJ::QQ,"QQ");
    openOne(ServiceTypeCYJ::WeChat,"微信");
    openOne(ServiceTypeCYJ::Weibo,"微博");

    const auto& after=controller_.findUser(userId);
    if(after.getAccount(ServiceTypeCYJ::QQ) && after.getAccount(ServiceTypeCYJ::WeChat)) {
        const auto* wx=dynamic_cast<const WeChatAccountCYJ*>(after.getAccount(ServiceTypeCYJ::WeChat));
        if(wx && wx->getBoundQQId().empty() && askYes("将微信绑定到本人 QQ? y/n: ")) {
            controller_.executeCommand([&] { controller_.bindQQ(userId); });
            output_<<"已绑定：微信账号关联到 QQ 号码 "<<after.getAccount(ServiceTypeCYJ::QQ)->getServiceId()<<"\n";
        }
    }

    output_<<"\n当前账号摘要:\n";
    output_<<"  统一用户ID: "<<userId<<"\n";
    if(after.getAccounts().empty()) output_<<"  （尚未开通服务，登录前请先开通至少一个）\n";
    for(const auto& [t,a]:after.getAccounts()) {
        output_<<"  "<<toStringCYJ(t)<<" 服务号码 "<<a->getServiceId()<<" → 账号键 "<<a->getAccountId()<<'\n';
    }
}
void ConsoleUICYJ::loginWizard() {
    output_<<"\n=== 登录 ===\n";
    output_<<"先输入统一用户ID，再选择【当前操作服务】；其它已开通服务会一并在线。\n";
    const auto id=read("统一用户ID: ");
    const auto& user=controller_.findUser(id);
    std::vector<ServiceTypeCYJ> opened;
    for(const auto& [t,a]:user.getAccounts()) {
        opened.push_back(t);
        output_<<"  "<<opened.size()<<". "<<toStringCYJ(t)<<" 号码 "<<a->getServiceId()<<"（"<<a->getAccountId()<<"）\n";
    }
    requireCYJ(!opened.empty(),"该用户尚未开通任何服务，请先开通服务");
    const int n=choice("选择当前操作服务序号: ");
    requireCYJ(n>=1 && n<=static_cast<int>(opened.size()),"无效服务序号");
    const auto selected=opened[static_cast<std::size_t>(n-1)];
    controller_.login(id,selected);
    output_<<"登录成功。当前操作服务: "<<controller_.getCurrentAccountId()<<"\n";
    output_<<"本人其它已开通服务已自动在线（可在「登录状态」查看）。\n";
}
void ConsoleUICYJ::showMainMenu() {
    output_<<"\n=== 模拟即时通信系统 CYJ ===\n";
    if(controller_.hasCurrentService()) {
        const auto* u=controller_.getCurrentUser();
        output_<<"当前用户: "<<u->getUserId()<<" / "<<controller_.getCurrentAccountId()<<"\n";
    }
    output_<<"1 注册并开通  2 查询用户  3 追加开通服务  4 登录\n5 编辑资料  6 绑定QQ  7 登录状态  8 好友管理\n9 群管理  10 退出登录  11 保存  0 保存并退出\n";
}
std::vector<FriendRelationCYJ> ConsoleUICYJ::showContacts(const std::string& selfAccount) {
    const auto list=controller_.findFriends(selfAccount);
    if(list.empty()) {
        output_<<"（通讯录为空）\n";
        return list;
    }
    output_<<"通讯录（当前服务 "<<selfAccount<<"）:\n";
    for(std::size_t i=0;i<list.size();++i) {
        output_<<(i+1)<<". "<<list[i].getFriendAccountId()<<" | "<<list[i].getRemark()<<'\n';
    }
    return list;
}
void ConsoleUICYJ::showFriendMenu() {
    while(true) {
        try {
            requireCYJ(controller_.hasCurrentService(),"请先登录并选择当前操作服务");
            const auto self=controller_.getCurrentAccountId();
            output_<<"\n好友管理（当前服务 "<<self<<"）\n";
            output_<<"1 添加  2 修改备注  3 删除  4 通讯录  5 共同好友  6 跨服务推荐  0 返回\n";
            const int n=choice("> "); if(n==0) return; requireCYJ(n>=1 && n<=6,"无效菜单项");
            if(n==4) {
                showContacts(self);
            } else if(n==3) {
                const auto list=showContacts(self);
                if(list.empty()) { output_<<"无可删好友\n"; continue; }
                const int idx=choice("输入要删除的序号: ");
                requireCYJ(idx>=1 && idx<=static_cast<int>(list.size()),"无效序号");
                const auto target=list[static_cast<std::size_t>(idx-1)].getFriendAccountId();
                controller_.executeCommand([&] { controller_.removeFriend(self,target); });
                output_<<"已删除 "<<target<<" 并保存\n";
            } else if(n==5) {
                const auto b=peerOnCurrentService("对方同服务号码或账号键: ");
                const auto common=controller_.findCommonFriends(self,b);
                if(common.empty()) output_<<"（无共同好友）\n";
                for(const auto& key:common) output_<<key<<'\n';
            } else if(n==6) {
                output_<<"选择好友来源服务（将映射到当前服务 "<<toStringCYJ(controller_.getCurrentService())<<"）:\n";
                const auto source=pickOtherOpenedService("来源服务序号: ");
                const auto list=controller_.recommendFriend(self,source);
                if(list.empty()) { output_<<"暂无可推荐好友\n"; continue; }
                output_<<"推荐结果:\n";
                for(std::size_t i=0;i<list.size();++i) output_<<"  "<<(i+1)<<". "<<list[i]<<'\n';
                if(!askYes("是否添加为当前服务好友? y/n: ")) { output_<<"已取消添加\n"; continue; }
                if(list.size()==1) {
                    controller_.executeCommand([&] { controller_.addFriend(self,list[0]); });
                    output_<<"已添加 "<<list[0]<<" 并保存\n";
                } else {
                    const int idx=choice("选择要添加的序号: ");
                    requireCYJ(idx>=1 && idx<=static_cast<int>(list.size()),"无效序号");
                    const auto target=list[static_cast<std::size_t>(idx-1)];
                    controller_.executeCommand([&] { controller_.addFriend(self,target); });
                    output_<<"已添加 "<<target<<" 并保存\n";
                }
            } else {
                const auto b=peerOnCurrentService("好友同服务号码或账号键: ");
                std::string r; if(n==1 || n==2) r=read("备注（可空）: ");
                controller_.executeCommand([&] {
                    if(n==1) controller_.addFriend(self,b,r);
                    else controller_.updateFriend(self,b,r);
                });
                output_<<"操作成功并已保存\n";
            }
        } catch(const std::exception& e) { output_<<"操作失败: "<<e.what()<<'\n'; }
    }
}
void ConsoleUICYJ::approvePendingWizard(const std::string& actor) {
    output_<<"\n=== 审批入群申请（QQ 群；群主/管理员）===\n";
    std::vector<std::string> groupsWithPending;
    for(const auto& [gid,g]:controller_.getGroupManager().getGroups()) {
        if(g.getPolicy().getName()!="QQ" || g.getPendingRequests().empty()) continue;
        if(!g.hasMember(actor)) continue;
        const auto role=g.roleOf(actor);
        if(role!=GroupRoleCYJ::Owner && role!=GroupRoleCYJ::Admin) continue;
        if(g.getServiceType()!=controller_.getCurrentService()) continue;
        groupsWithPending.push_back(gid);
        output_<<"  "<<groupsWithPending.size()<<". 群 "<<gid<<"「"<<g.getName()<<"」待审 "<<g.getPendingRequests().size()<<" 人\n";
    }
    if(groupsWithPending.empty()) { output_<<"当前没有待你审批的入群申请\n"; return; }
    const int gi=choice("选择群序号: ");
    requireCYJ(gi>=1 && gi<=static_cast<int>(groupsWithPending.size()),"无效群序号");
    const auto gid=groupsWithPending[static_cast<std::size_t>(gi-1)];
    while(true) {
        const auto& g=controller_.getGroupManager().findGroup(gid);
        std::vector<std::string> pending(g.getPendingRequests().begin(),g.getPendingRequests().end());
        if(pending.empty()) { output_<<"群 "<<gid<<" 已无待审批申请\n"; return; }
        output_<<"群 "<<gid<<" 待审批:\n";
        for(std::size_t i=0;i<pending.size();++i) output_<<"  "<<(i+1)<<". "<<pending[i]<<'\n';
        output_<<"  0. 返回\n";
        const int pi=choice("选择申请人序号: ");
        if(pi==0) return;
        requireCYJ(pi>=1 && pi<=static_cast<int>(pending.size()),"无效序号");
        const auto target=pending[static_cast<std::size_t>(pi-1)];
        const bool yes=askYes("对 "+target+" 批准? y/n: ");
        controller_.executeCommand([&] { controller_.approveRequest(gid,actor,target,yes); });
        output_<<(yes?"已同意 ":"已拒绝 ")<<target<<" 并保存\n";
    }
}
void ConsoleUICYJ::inviteFriendToGroupWizard(const std::string& actor) {
    output_<<"\n=== 推荐好友入群 ===\n";
    output_<<"说明：你须已是群成员；对方须是当前服务好友且尚未在群内。\n";
    output_<<"QQ 群：可申请也可推荐；微信群：只能推荐（不可申请）。\n";
    const auto gid=read("群号: ");
    const auto& g=controller_.getGroupManager().findGroup(gid);
    requireCYJ(g.getServiceType()==controller_.getCurrentService(),"请用与该群相同服务的当前账号操作");
    requireCYJ(g.hasMember(actor),"你不是该群成员，无法推荐");
    requireCYJ(g.getPolicy().canJoin(false,true),"当前群规则不允许推荐入群");
    std::vector<std::string> candidates;
    for(const auto& r:controller_.findFriends(actor)) {
        const auto& fid=r.getFriendAccountId();
        if(!g.hasMember(fid)) candidates.push_back(fid);
    }
    if(candidates.empty()) { output_<<"没有可推荐入群的好友（需为当前服务好友且不在群内）\n"; return; }
    output_<<"可推荐的好友:\n";
    for(std::size_t i=0;i<candidates.size();++i) output_<<"  "<<(i+1)<<". "<<candidates[i]<<'\n';
    const int idx=choice("选择要推荐入群的序号: ");
    requireCYJ(idx>=1 && idx<=static_cast<int>(candidates.size()),"无效序号");
    const auto target=candidates[static_cast<std::size_t>(idx-1)];
    controller_.executeCommand([&] { controller_.inviteMember(gid,actor,target); });
    output_<<"已将 "<<target<<" 推荐入群 "<<gid<<" 并保存\n";
}
void ConsoleUICYJ::kickMemberWizard(const std::string& actor) {
    output_<<"\n=== 踢出群成员 ===\n";
    const auto gid=read("群号: ");
    const auto& g=controller_.getGroupManager().findGroup(gid);
    requireCYJ(g.getServiceType()==controller_.getCurrentService(),"请用与该群相同服务的当前账号操作");
    requireCYJ(g.hasMember(actor),"你不是该群成员");
    std::vector<std::string> all;
    output_<<"群 "<<gid<<" 成员:\n";
    for(const auto& [acc,m]:g.getMembers()) {
        all.push_back(acc);
        const bool can=g.getPolicy().canKick(g.roleOf(actor),m.getRole());
        output_<<"  "<<all.size()<<". "<<acc<<" | "<<toStringCYJ(m.getRole())<<(can?"":"（不可踢）")<<'\n';
    }
    if(all.empty()) { output_<<"群内无成员\n"; return; }
    const int idx=choice("输入要踢出的序号: ");
    requireCYJ(idx>=1 && idx<=static_cast<int>(all.size()),"无效序号");
    const auto target=all[static_cast<std::size_t>(idx-1)];
    requireCYJ(g.getPolicy().canKick(g.roleOf(actor),g.roleOf(target)),"该成员不可踢（权限不足或为群主）");
    controller_.executeCommand([&] { controller_.kickMember(gid,actor,target); });
    output_<<"已将 "<<target<<" 踢出群 "<<gid<<" 并保存\n";
}
void ConsoleUICYJ::setAdminWizard(const std::string& actor) {
    output_<<"\n=== 设置管理员（仅 QQ 管理规则）===\n";
    const auto gid=read("群号: ");
    const auto& g=controller_.getGroupManager().findGroup(gid);
    requireCYJ(g.getServiceType()==controller_.getCurrentService(),"请用与该群相同服务的当前账号操作");
    requireCYJ(g.hasMember(actor),"你不是该群成员");
    if(!g.getPolicy().canManageAdmin(g.roleOf(actor))) {
        output_<<"不允许设置管理员：当前群规则为 "<<g.getPolicy().getName()<<"，或你不是群主。\n";
        return;
    }
    std::vector<std::string> all;
    output_<<"群 "<<gid<<" 成员:\n";
    for(const auto& [acc,m]:g.getMembers()) {
        all.push_back(acc);
        output_<<"  "<<all.size()<<". "<<acc<<" | "<<toStringCYJ(m.getRole());
        if(acc==g.getOwnerAccountId()) output_<<"（不可改）";
        output_<<'\n';
    }
    const int idx=choice("选择成员序号: ");
    requireCYJ(idx>=1 && idx<=static_cast<int>(all.size()),"无效序号");
    const auto target=all[static_cast<std::size_t>(idx-1)];
    requireCYJ(target!=g.getOwnerAccountId(),"不能修改群主角色");
    const bool yes=askYes("设为管理员? y/n（n 表示取消管理员）: ");
    controller_.executeCommand([&] { controller_.setAdmin(gid,actor,target,yes); });
    output_<<(yes?"已设为管理员: ":"已取消管理员: ")<<target<<" 并保存\n";
}
void ConsoleUICYJ::changePolicyWizard(const std::string& actor) {
    output_<<"\n=== 切换群管理规则（不改变账号所属服务）===\n";
    const auto gid=read("群号: ");
    const auto& g=controller_.getGroupManager().findGroup(gid);
    requireCYJ(g.getServiceType()==controller_.getCurrentService(),"请用与该群相同服务的当前账号操作");
    requireCYJ(actor==g.getOwnerAccountId(),"仅群主可切换管理规则");
    output_<<"当前规则: "<<g.getPolicy().getName()<<"\n";
    output_<<"选择新管理规则:\n  1. QQ\n  2. WeChat\n";
    const int idx=choice("序号: ");
    requireCYJ(idx==1 || idx==2,"请选择 1 或 2");
    const std::string policy=idx==1?"QQ":"WeChat";
    if(policy==g.getPolicy().getName()) { output_<<"规则未变化\n"; return; }
    controller_.executeCommand([&] { controller_.changeGroupPolicy(gid,actor,policy); });
    output_<<"群 "<<gid<<" 管理规则已切换为 "<<policy<<" 并保存\n";
}
void ConsoleUICYJ::createSubgroupWizard(const std::string& actor) {
    output_<<"\n=== 创建临时讨论组（仅 QQ 管理规则）===\n";
    const auto parent=read("父群号: ");
    const auto& g=controller_.getGroupManager().findGroup(parent);
    requireCYJ(g.getServiceType()==controller_.getCurrentService(),"请用与该群相同服务的当前账号操作");
    requireCYJ(g.hasMember(actor),"你不是该群成员");
    if(!g.getPolicy().canCreateSubgroup(g.roleOf(actor))) {
        output_<<"不允许创建临时讨论组：当前群规则为 "<<g.getPolicy().getName()
               <<"，或你无权创建（需群主/管理员，且为 QQ 规则）。\n";
        return;
    }
    requireCYJ(g.getParentGroupId().empty(),"不能在讨论组下再创建讨论组");
    const auto newId=read("讨论组ID: ");
    const auto name=read("讨论组名称: ");
    std::vector<std::string> roster;
    output_<<"父群成员（创建者 "<<actor<<" 将自动在讨论组中）:\n";
    for(const auto& [acc,m]:g.getMembers()) {
        roster.push_back(acc);
        output_<<"  "<<roster.size()<<". "<<acc<<" | "<<toStringCYJ(m.getRole());
        if(acc==actor) output_<<"（创建者）";
        output_<<'\n';
    }
    const auto line=read("选择成员序号（多人用空格分隔，如 1 3 4，可空）: ");
    std::vector<std::string> members;
    std::istringstream in(line);
    int idx=0;
    while(in>>idx) {
        requireCYJ(idx>=1 && idx<=static_cast<int>(roster.size()),"无效成员序号");
        const auto& acc=roster[static_cast<std::size_t>(idx-1)];
        if(acc!=actor) members.push_back(acc);
    }
    controller_.executeCommand([&] { controller_.createSubgroup(parent,actor,newId,name,members); });
    output_<<"已创建讨论组 "<<newId<<" 并保存\n";
}
void ConsoleUICYJ::showGroupMenu() {
    while(true) {
        try {
            output_<<"\n群: 1 创建  2 列表  3 成员与申请  4 申请入群  5 审批申请\n6 推荐好友入群  7 退群  8 踢人  9 设置管理员\n10 切换管理规则  11 创建临时讨论组  12 解散  0 返回\n";
            if(controller_.hasCurrentService()) {
                output_<<"（群操作默认使用当前服务 "<<controller_.getCurrentAccountId()<<"）\n";
                output_<<"提示：QQ 群可申请也可推荐好友入群；微信群只能推荐好友入群（不可申请）。\n";
            }
            const int n=choice("> "); if(n==0) return; requireCYJ(n>=1 && n<=12,"无效菜单项");
            if(n==2) { showGroups(); continue; } if(n==3) { showMembers(); continue; }
            requireCYJ(controller_.hasCurrentService(),"请先登录并选择当前操作服务");
            const auto a=controller_.getCurrentAccountId();
            if(n==5) { approvePendingWizard(a); continue; }
            if(n==6) { inviteFriendToGroupWizard(a); continue; }
            if(n==8) { kickMemberWizard(a); continue; }
            if(n==9) { setAdminWizard(a); continue; }
            if(n==10) { changePolicyWizard(a); continue; }
            if(n==11) { createSubgroupWizard(a); continue; }
            const auto id=read("群号: ");
            std::string name;
            if(n==1) name=read("群名称: ");
            if(n==12 && read("解散群及其讨论组，确认 y: ")!="y") continue;
            controller_.executeCommand([&] {
                switch(n) {
                    case 1: controller_.createGroup(id,name,a); break;
                    case 4: controller_.joinGroup(id,a); break;
                    case 7: controller_.leaveGroup(id,a); break;
                    case 12: controller_.dissolveGroup(id,a); break;
                }
            }); output_<<(n==4?"申请已提交，等待审批\n":"操作成功并已保存\n");
        } catch(const std::exception& e) { output_<<"操作失败: "<<e.what()<<'\n'; }
    }
}
void ConsoleUICYJ::run() {
    try {
        while(true) {
            showMainMenu();
            try {
                int n=choice("> "); requireCYJ(n>=0 && n<=11,"无效菜单项");
                if(n==0) { controller_.save(); output_<<"已保存，再见\n"; return; }
                if(n==1) { registerWizard(); continue; }
                if(n==2) { showUsers(); continue; }
                if(n==3) {
                    std::string id;
                    if(const auto* u=controller_.getCurrentUser()) id=u->getUserId();
                    else id=read("统一用户ID: ");
                    openServiceWizard(id,false);
                    continue;
                }
                if(n==4) { loginWizard(); continue; }
                if(n==7) { showLoginStatus(); continue; }
                if(n==8) { showFriendMenu(); continue; }
                if(n==9) { showGroupMenu(); continue; }
                if(n==10) { controller_.logout(); output_<<"已退出全部服务\n"; continue; }
                if(n==11) { controller_.save(); output_<<"已保存\n"; continue; }
                std::string name,birth,location;
                if(n==5) { name=read("昵称: "); location=read("所在地（可空）: "); birth=read("出生日期 YYYY-MM-DD（可空）: "); }
                controller_.executeCommand([&] {
                    if(n==5) controller_.updateProfile(name,location,birth);
                    else if(n==6) controller_.bindQQ();
                }); output_<<"操作成功并已保存\n";
            } catch(const std::exception& e) { output_<<"操作失败: "<<e.what()<<'\n'; }
        }
    } catch(const EndOfInputCYJ&) { controller_.save(); output_<<"输入结束，数据已保存\n"; }
}
