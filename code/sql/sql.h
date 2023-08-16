/*
 * @Author  :   XavierYorke 
 * @Contact :   mzlxavier1230@gmail.com
 * @Time    :   2023-07-08
 */

#pragma once

#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct User {
    string m_name;
    string m_passwd;
    User(string name, string passwd): m_name(name), m_passwd(passwd) {}
    User() {}
};

class User_manager {

public:
    User_manager();
    ~User_manager();

    bool m_insert(User& stu);
    bool m_update(User& stu);
    bool m_delete(User& stu);
    vector<User> m_query(string condition);
    bool do_command(char sql_command[]);

private:
    MYSQL* conn;
    const char* host = "127.0.0.1";

    const char* user = "root";
    const char* passwd = "123456";
    const char* database_name = "FreelyChat";
    const char* table_name = "User";
    const int port = 3306;
};