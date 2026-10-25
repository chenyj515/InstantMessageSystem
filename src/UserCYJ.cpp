#include "UserCYJ.h"
#include <utility>
UserCYJ::UserCYJ(std::string id,std::string name,std::string birth,std::string location,std::string registered)
 : userId_(std::move(id)), nickname_(std::move(name)), birthDate_(std::move(birth)), registerTime_(std::move(registered)), location_(std::move(location)) {
    validateIdCYJ(userId_); validateDateCYJ(registerTime_,false); updateProfile(nickname_,location_,birthDate_);
}
const std::string& UserCYJ::getUserId() const { return userId_; }
const std::string& UserCYJ::getNickname() const { return nickname_; }
const std::string& UserCYJ::getBirthDate() const { return birthDate_; }
const std::string& UserCYJ::getRegisterTime() const { return registerTime_; }
const std::string& UserCYJ::getLocation() const { return location_; }
void UserCYJ::updateProfile(const std::string& name,const std::string& loc,const std::string& birth) {
    validateTextCYJ(name,"Nickname"); validateTextCYJ(loc,"Location",true); validateDateCYJ(birth);
    requireCYJ(birth.empty() || birth<=registerTime_, "Birth date cannot be after registration date");
    nickname_=name; location_=loc; birthDate_=birth;
}
const ServiceAccountCYJ* UserCYJ::getAccount(ServiceTypeCYJ t) const { auto i=accounts_.find(t); return i==accounts_.end()?nullptr:i->second.get(); }
const std::map<ServiceTypeCYJ,std::unique_ptr<ServiceAccountCYJ>>& UserCYJ::getAccounts() const { return accounts_; }
