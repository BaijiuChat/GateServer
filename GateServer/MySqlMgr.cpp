#include "MySqlMgr.h"

MySqlMgr::~MySqlMgr() 
{
}

int MySqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    return _dao.RegUser(name, email, pwd);
}

int MySqlMgr::CheckEmail(const std::string& name, const std::string& email) {
    return _dao.CheckEmail(name, email);
}

int MySqlMgr::UpdatePwd(const std::string& name, const std::string& pwd) {
    return _dao.UpdatePwd(name, pwd);
}

int MySqlMgr::CheckLogin(const std::string& email, const std::string& pwd, UserInfo userInfo) {
    return _dao.CheckLogin(email, pwd, userInfo);
}

MySqlMgr::MySqlMgr() 
{
}