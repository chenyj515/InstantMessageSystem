#pragma once
#include <stdexcept>
#include <string>

enum class ServiceTypeCYJ { QQ, WeChat, Weibo };
enum class GroupRoleCYJ { Owner, Admin, Member };
std::string toStringCYJ(ServiceTypeCYJ type);
ServiceTypeCYJ serviceTypeCYJ(const std::string& value);
std::string toStringCYJ(GroupRoleCYJ role);
GroupRoleCYJ groupRoleCYJ(const std::string& value);
std::string accountKeyCYJ(ServiceTypeCYJ type, const std::string& id);
void requireCYJ(bool condition, const std::string& message);
void validateTextCYJ(const std::string& value, const std::string& field, bool allowEmpty = false);
void validateIdCYJ(const std::string& value);
void validateDateCYJ(const std::string& value, bool allowEmpty = true);
std::string todayCYJ();
// T龄：按申请/注册日期计算到今天的整年数（不足一年为 0）。
int tAgeYearsCYJ(const std::string& registerDate);
