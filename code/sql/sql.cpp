#include "sql.h"

User_manager::User_manager() {
    conn = mysql_init(NULL);

    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8");    

    if (!mysql_real_connect(conn, host, user, passwd, database_name, port, NULL, 0)) {
        fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(conn));
        exit(1);
    }
    // cout << "connect successful!\n";
}

User_manager::~User_manager() {
    mysql_close(conn);
}

bool User_manager::m_insert(User& user) {
    char sql_command[1024];
    sprintf(sql_command, "insert into %s (name, passwd) values('%s', '%s')", 
        table_name, user.m_name.c_str(), user.m_passwd.c_str());

    return do_command(sql_command);
}

bool User_manager::m_update(User& user) {
    char sql_command[1024];
    sprintf(sql_command, "update %s set passwd = '%s' where name = %s", 
        table_name, user.m_passwd.c_str(), user.m_name.c_str());
    
    return do_command(sql_command);
}

bool User_manager::m_delete(User& user) {
    char sql_command[1024];
    sprintf(sql_command, "delete from %s where name = %s", table_name, user.m_name.c_str());
    
    return do_command(sql_command);
}

vector<User> User_manager::m_query(string condition = "") {
    char sql_command[1024];
    sprintf(sql_command, "select * from %s %s", table_name, condition.c_str());
    if (!do_command(sql_command)) return {};

    vector<User> userlist;
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (row = mysql_fetch_row(res)) {
        User user;
        user.m_name = row[0];
        user.m_passwd = row[1];
        userlist.push_back(user);
    }
    return userlist;
}

bool User_manager::do_command(char sql_command[]) {
    cout << sql_command << endl;
    if (mysql_query(conn, sql_command)) {
        fprintf(stderr, "Failed error: %s\n", mysql_error(conn));
        return false;
    }
    return true;
}

/*
int main() {
    User_manager* m_manager = User_manager::GetInstance();
    User user("Xavier", "key001");
    // m_manager->m_insert(user);
    vector<User> users = m_manager->m_query();
    for (auto& user: users) {
        cout  << "name: " << user.m_name << ", passwd: " << user.m_passwd << endl;
    }

    return 0;
}
*/