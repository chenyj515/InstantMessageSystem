#include "ConsoleUICYJ.h"
#include "WeChatAccountCYJ.h"
#include <chrono>
#include <fstream>
#include <functional>
#include <sstream>

namespace {
int checks=0;
void check(bool value,const std::string& message) {
    ++checks; if(!value) throw std::runtime_error("CHECK FAILED: "+message);
}
void rejects(const std::function<void()>& call,const std::string& message) {
    bool threw=false; try { call(); } catch(const std::exception&) { threw=true; } check(threw,message);
}
void createUser(PlatformControllerCYJ& p,const std::string& id,const std::string& qq) {
    p.registerUser(id,"昵称 \"quote\" \\ path | "+id,"2004-02-29","吉林 长春");
    p.openService(id,ServiceTypeCYJ::QQ,qq); p.openService(id,ServiceTypeCYJ::WeChat,"wx_"+id);
}
}
int main() {
    try {
        const auto root=std::filesystem::current_path()/("ims-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        PlatformControllerCYJ p(root/"main"); check(!p.initialize(),"missing database initializes empty");
        createUser(p,"u1","10001"); createUser(p,"u2","10002"); createUser(p,"u3","10003"); createUser(p,"u4","10004");
        check(p.getUsers().size()==4,"registered four users");
        check(p.nextUserId()=="UID1","auto unified user id sequence");
        const auto autoId=p.createUser("自动用户","2001-01-01","北京");
        check(autoId=="UID1" && p.findUser("UID1").getNickname()=="自动用户","createUser auto-generates unified id");
        check(p.nextServiceId(ServiceTypeCYJ::QQ)=="10005","auto QQ service number");
        check(p.nextServiceId(ServiceTypeCYJ::WeChat)=="wx10001","auto WeChat service number");
        rejects([&]{p.openService("UID1",ServiceTypeCYJ::QQ,"UID1");},"service number must differ from unified user id");
        rejects([&]{p.openService("UID1",ServiceTypeCYJ::WeChat,"10001");},"WeChat number must differ from QQ/Weibo numbers");
        rejects([&]{p.registerUser("u1","duplicate");},"duplicate user");
        rejects([&]{p.registerUser("bad:id","bad");},"ID validation");
        rejects([&]{p.registerUser("bad_date","bad","2003-02-29");},"calendar validation");
        rejects([&]{p.openService("u1",ServiceTypeCYJ::QQ,"20000");},"duplicate activation");
        rejects([&]{p.openService("u1",ServiceTypeCYJ::Weibo,"10002");},"shared ID mismatch");
        rejects([&]{p.openService("u2",ServiceTypeCYJ::Weibo,"10001");},"cross-user reserved shared ID");
        p.openService("u1",ServiceTypeCYJ::Weibo,"10001");
        p.login("u1",ServiceTypeCYJ::QQ);
        check(p.getLoginStatus().size()==3,"all activated services visible");
        check(p.hasCurrentService() && p.getCurrentService()==ServiceTypeCYJ::QQ,"current operating service recorded");
        check(p.getCurrentAccountId()=="QQ:10001","current account follows selected service");
        for(const auto& [t,a]:p.findUser("u1").getAccounts()) { (void)t; check(a->isLoggedIn(),"unified login"); }
        check(tAgeYearsCYJ("2020-01-01")>=0,"T-age from register date");
        rejects([&]{p.addFriend("QQ:10002","QQ:10003");},"cannot impersonate another account");
        rejects([&]{p.addFriend("QQ:10001","WeChat:wx_u2");},"no mixed-service friends");
        rejects([&]{p.addFriend("QQ:10001","QQ:10001");},"cannot friend self");
        rejects([&]{p.addFriend("QQ:10001","QQ:missing");},"friend must exist");
        p.addFriend("QQ:10001","QQ:10002","同学 \"B\" \\ | test"); p.addFriend("QQ:10001","QQ:10003");
        check(p.getFriendManager().areFriends("QQ:10002","QQ:10001"),"symmetric friendship");
        rejects([&]{p.addFriend("QQ:10001","QQ:10002");},"duplicate friend");
        p.updateFriend("QQ:10001","QQ:10002","我的备注");
        check(p.findFriends("QQ:10001").at(0).getRemark()=="我的备注","remark update");
        check(p.getFriendManager().findFriends("QQ:10002").at(0).getRemark().empty(),"remarks are directional");
        rejects([&]{p.recommendFriend("WeChat:wx_u1",ServiceTypeCYJ::QQ);},"QQ recommendation requires binding");
        p.bindQQ();
        const auto recommendations=p.recommendFriend("WeChat:wx_u1",ServiceTypeCYJ::QQ);
        check(recommendations==std::vector<std::string>({"WeChat:wx_u2","WeChat:wx_u3"}),"map source friends to target service accounts");
        check(!p.getFriendManager().areFriends("WeChat:wx_u1","WeChat:wx_u2"),"recommendation never auto-adds");
        p.addFriend("WeChat:wx_u1","WeChat:wx_u2");
        check(p.recommendFriend("WeChat:wx_u1",ServiceTypeCYJ::QQ).size()==1,"exclude existing target friends");
        p.createGroup("g1","课程群","QQ:10001");
        p.createGroup("wxg","微信群","WeChat:wx_u1"); p.inviteMember("wxg","WeChat:wx_u1","WeChat:wx_u2");
        rejects([&]{p.inviteMember("wxg","WeChat:wx_u1","WeChat:wx_u3");},"WeChat recommendation requires friendship");
        rejects([&]{p.createGroup("wb","no","Weibo:10001");},"no unrequested Weibo group");
        p.login("u2",ServiceTypeCYJ::QQ);
        rejects([&]{p.login("u2",ServiceTypeCYJ::Weibo);},"unopened service login fails");
        check(p.getCurrentUser()->getUserId()=="u2" && !p.findUser("u2").getAccount(ServiceTypeCYJ::Weibo),"failed login never activates service or destroys session");
        check(!p.findAccount("QQ:10001").isLoggedIn(),"previous user logged out");
        p.addFriend("QQ:10002","QQ:10003");
        check(p.findCommonFriends("QQ:10002","QQ:10001")==std::vector<std::string>{"QQ:10003"},"common friends intersection");
        p.joinGroup("g1","QQ:10002");
        check(!p.getGroupManager().findGroup("g1").hasMember("QQ:10002"),"application does not bypass approval");
        rejects([&]{p.joinGroup("g1","QQ:10002");},"duplicate pending application");
        rejects([&]{p.joinGroup("wxg","WeChat:wx_u2");},"WeChat cannot apply directly");
        rejects([&]{p.joinGroup("g1","WeChat:wx_u2");},"group account service mismatch");
        p.login("u3",ServiceTypeCYJ::QQ); p.joinGroup("g1","QQ:10003");
        p.login("u4",ServiceTypeCYJ::QQ); p.joinGroup("g1","QQ:10004");
        p.login("u1",ServiceTypeCYJ::QQ);
        p.approveRequest("g1","QQ:10001","QQ:10004",false);
        check(!p.getGroupManager().findGroup("g1").getPendingRequests().count("QQ:10004"),"reject pending request");
        p.approveRequest("g1","QQ:10001","QQ:10002",true); p.approveRequest("g1","QQ:10001","QQ:10003",true);
        p.setAdmin("g1","QQ:10001","QQ:10002",true);
        p.login("u3",ServiceTypeCYJ::QQ);
        rejects([&]{p.kickMember("g1","QQ:10003","QQ:10002");},"ordinary member cannot kick admin");
        rejects([&]{p.changeGroupPolicy("g1","QQ:10003","WeChat");},"ordinary member cannot switch policy");
        p.login("u2",ServiceTypeCYJ::QQ);
        rejects([&]{p.kickMember("g1","QQ:10002","QQ:10001");},"admin cannot kick owner");
        p.createSubgroup("g1","QQ:10002","discussion","讨论组",{"QQ:10001","QQ:10003"});
        check(p.getGroupManager().findGroup("discussion").getMembers().size()==3,"QQ admin creates selected discussion subgroup");
        rejects([&]{p.createSubgroup("g1","QQ:10002","badsub","bad",{"QQ:10004"});},"subgroup selection restricted to parent members");
        p.kickMember("g1","QQ:10002","QQ:10003");
        check(!p.getGroupManager().findGroup("g1").hasMember("QQ:10003"),"QQ admin kicks ordinary member");
        p.login("u1",ServiceTypeCYJ::QQ);
        p.inviteMember("g1","QQ:10001","QQ:10003");
        check(p.getGroupManager().findGroup("g1").hasMember("QQ:10003"),"QQ also allows friend invitation into group");
        p.kickMember("g1","QQ:10001","QQ:10003");
        p.changeGroupPolicy("g1","QQ:10001","WeChat"); const auto& group=p.getGroupManager().findGroup("g1");
        check(group.getGroupId()=="g1" && group.getOwnerAccountId()=="QQ:10001" && group.getMembers().size()==2,"policy switch preserves identity and membership");
        check(group.roleOf("QQ:10002")==GroupRoleCYJ::Member,"switch demotes incompatible administrators");
        rejects([&]{p.setAdmin("g1","QQ:10001","QQ:10002",true);},"WeChat forbids administrator role");
        rejects([&]{p.createSubgroup("g1","QQ:10001","dis2","bad",{});},"WeChat forbids subgroup");
        rejects([&]{p.leaveGroup("g1","QQ:10001");},"owner cannot abandon group");
        p.inviteMember("g1","QQ:10001","QQ:10003");
        check(group.hasMember("QQ:10003"),"switch changes rules without converting service identities");
        p.login("u2",ServiceTypeCYJ::QQ);
        rejects([&]{p.kickMember("g1","QQ:10002","QQ:10003");},"former admin loses kick privilege");
        p.leaveGroup("g1","QQ:10002"); check(!group.hasMember("QQ:10002"),"leave group");
        p.login("u1",ServiceTypeCYJ::QQ); p.removeFriend("QQ:10001","QQ:10002");
        check(!p.getFriendManager().areFriends("QQ:10002","QQ:10001"),"delete both friendship directions");
        p.save();
        PlatformControllerCYJ restored(root/"main"); check(restored.initialize(),"reload saved data");
        check(restored.getUsers().size()==5 && restored.findUser("u1").getNickname()==p.findUser("u1").getNickname(),"Unicode, quotes, spaces, backslash round trip");
        check(restored.getCurrentUser()==nullptr && !restored.hasCurrentService() && !restored.findAccount("QQ:10001").isLoggedIn(),"sessions not persisted");
        const auto& wx=dynamic_cast<const WeChatAccountCYJ&>(restored.findAccount("WeChat:wx_u1")); check(wx.getBoundQQId()=="10001","binding persisted");
        check(restored.getGroupManager().findGroup("g1").getPolicy().getName()=="WeChat" && restored.getGroupManager().findGroup("discussion").getParentGroupId()=="g1","policy and subgroup persisted");
        check(restored.getFriendManager().getRelations().size()==p.getFriendManager().getRelations().size(),"friendship persisted");
        std::filesystem::rename(root/"main/platform.txt",root/"main/platform.txt.bak");
        PlatformControllerCYJ recovered(root/"main"); check(recovered.initialize() && recovered.getUsers().size()==5,"recover interrupted rotation from backup"); recovered.save();
        {
            std::ofstream out(root/"main/platform.txt",std::ios::trunc); out<<"IMS_CYJ 1\nUSER \"partial\" \"partial\" \"\" \"2026-01-01\" \"\"\n";
        }
        rejects([&]{restored.initialize();},"truncated snapshot rejected"); check(restored.getUsers().size()==5,"failed load leaves existing memory untouched");
        p.save();
        {
            std::ofstream out(root/"main/platform.txt",std::ios::app); out<<"UNKNOWN bad\n";
        }
        rejects([&]{restored.initialize();},"records after END rejected");
        p.save();
        std::filesystem::create_directories(root/"invalid");
        {
            std::ofstream out(root/"invalid/platform.txt"); out<<"IMS_CYJ 1\nFRIEND \"QQ:missing\" \"QQ:absent\" \"\"\nEND\n";
        }
        PlatformControllerCYJ invalid(root/"invalid"); rejects([&]{invalid.initialize();},"dangling references rejected");
        {
            std::ofstream out(root/"blocked"); out<<"not a directory";
        }
        PlatformControllerCYJ blocked(root/"blocked"); rejects([&]{blocked.executeCommand([&]{blocked.registerUser("dirty","未保存");});},"write error reported");
        check(blocked.getUsers().count("dirty")==1,"unsaved memory retained for retry");
        PlatformControllerCYJ ui(root/"ui");
        // 注册并开通向导：自动 UID1 → 用户输入 QQ 服务ID → 登录选服务
        std::istringstream input("junk\n1\n菜单测试\n2004-01-01\n上海\ny\n10009\nn\nn\n4\nUID1\n1\n7\n8\n0\n9\n0\n0\n"); std::ostringstream output;
        ConsoleUICYJ(ui,input,output).run();
        check(ui.findUser("UID1").getAccount(ServiceTypeCYJ::QQ)!=nullptr,"scripted menu registration and activation");
        check(ui.findAccount("QQ:10009").getUserId()=="UID1","wizard keeps unified user id distinct from QQ number");
        check(ui.findUser("UID1").getAccount(ServiceTypeCYJ::QQ)->getServiceId()!="UID1","service id differs from user id");
        check(std::filesystem::exists(root/"ui/platform.txt"),"menu persists changes");
        std::istringstream eof; ConsoleUICYJ(ui,eof,output).run(); check(output.str().find("输入结束")!=std::string::npos,"EOF exits cleanly");
        p.login("u1",ServiceTypeCYJ::QQ); p.dissolveGroup("g1","QQ:10001");
        check(!p.getGroupManager().getGroups().count("g1") && !p.getGroupManager().getGroups().count("discussion"),"dissolve removes child discussion groups");
        std::cout<<"PASS: "<<checks<<" checks\nTest data: "<<root.u8string()<<'\n';
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
