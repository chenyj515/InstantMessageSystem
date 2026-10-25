#include "TypesCYJ.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string toStringCYJ(ServiceTypeCYJ t) {
    switch (t) { case ServiceTypeCYJ::QQ: return "QQ"; case ServiceTypeCYJ::WeChat: return "WeChat"; case ServiceTypeCYJ::Weibo: return "Weibo"; }
    throw std::runtime_error("Invalid service type");
}
ServiceTypeCYJ serviceTypeCYJ(const std::string& s) {
    if (s == "QQ") return ServiceTypeCYJ::QQ;
    if (s == "WeChat") return ServiceTypeCYJ::WeChat;
    if (s == "Weibo") return ServiceTypeCYJ::Weibo;
    throw std::runtime_error("Service must be QQ, WeChat or Weibo");
}
std::string toStringCYJ(GroupRoleCYJ r) {
    switch (r) { case GroupRoleCYJ::Owner: return "Owner"; case GroupRoleCYJ::Admin: return "Admin"; case GroupRoleCYJ::Member: return "Member"; }
    throw std::runtime_error("Invalid group role");
}
GroupRoleCYJ groupRoleCYJ(const std::string& s) {
    if (s == "Owner") return GroupRoleCYJ::Owner;
    if (s == "Admin") return GroupRoleCYJ::Admin;
    if (s == "Member") return GroupRoleCYJ::Member;
    throw std::runtime_error("Invalid group role");
}
void requireCYJ(bool c, const std::string& m) { if (!c) throw std::runtime_error(m); }
void validateTextCYJ(const std::string& s, const std::string& f, bool empty) {
    requireCYJ((empty || !s.empty()) && s.size() <= 1024, f + ": empty or too long");
    requireCYJ(std::none_of(s.begin(), s.end(), [](unsigned char c) { return c < 32 || c == 127; }), f + ": control characters forbidden");
}
void validateIdCYJ(const std::string& s) {
    requireCYJ(!s.empty() && s.size() <= 64 && std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
    }), "ID: use 1-64 ASCII letters, digits, underscore or hyphen");
}
void validateDateCYJ(const std::string& s, bool empty) {
    if (empty && s.empty()) return;
    requireCYJ(s.size() == 10 && s[4] == '-' && s[7] == '-', "Date format: YYYY-MM-DD");
    for (std::size_t i=0;i<s.size();++i) if (i!=4 && i!=7) requireCYJ(s[i]>='0' && s[i]<='9', "Invalid date");
    int y=std::stoi(s.substr(0,4)), m=std::stoi(s.substr(5,2)), d=std::stoi(s.substr(8,2));
    requireCYJ(y >= 1900 && m>=1 && m<=12, "Invalid date");
    const int days[]{31,28,31,30,31,30,31,31,30,31,30,31};
    int limit=days[m-1]+(m==2 && (y%400==0 || (y%4==0 && y%100!=0)) ? 1 : 0);
    requireCYJ(d>=1 && d<=limit, "Invalid calendar date");
}
std::string todayCYJ() {
    auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    std::ostringstream out; out << std::put_time(&tm, "%Y-%m-%d"); return out.str();
}
int tAgeYearsCYJ(const std::string& registerDate) {
    validateDateCYJ(registerDate,false);
    const auto today=todayCYJ();
    const int y1=std::stoi(registerDate.substr(0,4)), m1=std::stoi(registerDate.substr(5,2)), d1=std::stoi(registerDate.substr(8,2));
    const int y2=std::stoi(today.substr(0,4)), m2=std::stoi(today.substr(5,2)), d2=std::stoi(today.substr(8,2));
    int age=y2-y1;
    if(m2<m1 || (m2==m1 && d2<d1)) --age;
    return age<0?0:age;
}
