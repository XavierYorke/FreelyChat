#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <conio.h>

#include <iostream>
#include <limits>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define BUFFER_LEN 1024
#define NAME_LEN 21
#define STATE_LEN 5

const static char* Y = "OK";
const static char* N = "NO";

char name[NAME_LEN];

string GbkToUtf8(const char* src_str) {
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}

string Utf8ToGbk(const char* src_str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}

// receive message and print out
void* handle_recv(void* data) {
    SOCKET sock = *(int*)data;
    char msg[BUFFER_LEN] = {0};
    string recv_str;
    while (true) {
        memset(msg, 0, sizeof(msg));
        recv_str.clear();
        int len = recv(sock, msg, sizeof(msg) - 1, 0);

        if (len == -1) {
            // return NULL;
            continue;
        } 
        else if (len == 0) {
            cout << "The server stops running\n";
            closesocket(sock);
            exit(-1);
        }
        recv_str = msg;
        recv_str = Utf8ToGbk(recv_str.c_str());
        if (!recv_str.empty() && recv_str.back() != '\n') recv_str += '\n';
        cout << recv_str;
    }
    return NULL;
}

void* handle_send(void* data) {
    SOCKET sock = *(int*)data;
    string msg;
    while (true) {
        msg.clear();

        getline(cin, msg);

        msg = GbkToUtf8(msg.c_str());
        send(sock, msg.c_str(), msg.size(), 0);
    }
}

void handle_login(SOCKET sock);
void handle_register(SOCKET sock);
string handle_name_or_password(SOCKET sock, bool isPasswd, bool isFirst);

void start(SOCKET sock) {
    cout << "Please select enter 1 to log in and 2 to register: ";
    char choose[2] = {0};
    while (true) {
        memset(choose, 0, 2);
        cin >> choose;
        if (strcmp(choose, "1") != 0 && strcmp(choose, "2") != 0) {
            cout << "Please enter 1 or 2: ";
        } else {
            // send state
            int len = send(sock, choose, strlen(choose), 0);
            if (strcmp(choose, "1") == 0)
                handle_login(sock);
            else
                handle_register(sock);
            return;
        }
    }
}

void handle_login(SOCKET sock) {
    cout << "Please enter your name: ";
    // send name or Quit
    if (handle_name_or_password(sock, false, false) == "Q") {
        start(sock);
        return;
    }

    char state[STATE_LEN] = {0};

    while (true) {
        while (true) {
            memset(state, 0, STATE_LEN);
            // recv state
            int len = recv(sock, state, sizeof(state), 0);
            if (len == -1)
                continue;
            else if (len == 0) {
                closesocket(sock);
                exit(-1);
            } else
                break;
        }

        if (strcmp(state, Y) != 0) {
            cout << "The name is not registered, please re-enter(enter Q/q to quit): ";

            if (handle_name_or_password(sock, false, false) == "Q") {
                start(sock);
                return;
            }
        } 
        else {
            cout << "Please enter your password(enter Q/q to quit): ";
            memset(state, 0, STATE_LEN);
            while (true) {
                if (handle_name_or_password(sock, true, false) == "Q") {
                    start(sock);
                    return;
                }

                while (true) {
                    memset(state, 0, STATE_LEN);
                    int len = recv(sock, state, sizeof(state), 0);
                    if (len == -1)
                        continue;
                    else if (len == 0) {
                        closesocket(sock);
                        exit(-1);
                    } else
                        break;
                }

                if (strcmp(state, Y) != 0) {
                    cout << "Wrong password, please enter again(enter Q/q to quit): ";
                } 
                else {
                    cout << "Login successful!\n";
                    return;
                }
            }
        }
    }
}

void handle_register(SOCKET sock) {
    cout << "Please enter name(2~20 characters): ";
    if (handle_name_or_password(sock, false, false) == "Q") {
        start(sock);
        return;
    }

    char state[STATE_LEN];

    while (true) {
        while (true) {
            memset(state, 0, STATE_LEN);
            int len = recv(sock, state, sizeof(state), 0);
            if (len == -1)
                continue;
            else if (len == 0) {
                closesocket(sock);
                exit(-1);
            } else
                break;
        }

        if (strcmp(state, Y) != 0) {
            cout << "The name is registered, please re-enter(enter Q/q to quit): ";
            if (handle_name_or_password(sock, false, false) == "Q") {
                start(sock);
                return;
            }
        } else {
            cout << "Please enter your password(2~20 characters,enter Q/q to quit): ";
            memset(state, 0, STATE_LEN);
            while (true) {
                string passwd;
                if ((passwd = handle_name_or_password(sock, true, true)) == "Q") {
                    start(sock);
                    return;
                }
                cout << "Please enter your password again: ";
                string secpasswd = "";
                if ((secpasswd = handle_name_or_password(sock, true, true)) == "Q") {
                    start(sock);
                    return;
                }
                if (passwd != secpasswd) {
                    cout << "The password is inconsistent twice, please re-enter it: ";
                }
                else {
                    passwd = GbkToUtf8(passwd.c_str());
                    send(sock, passwd.c_str(), passwd.size(), 0);
                    cout << "Register successful!\n";
                    return;
                }

                // while (true) {
                //     memset(state, 0, STATE_LEN);
                //     int len = recv(sock, state, sizeof(state), 0);
                //     if (len == -1)
                //         continue;
                //     else if (len == 0) {
                //         closesocket(sock);
                //         exit(-1);
                //     } else
                //         break;
                // }
                // if (strcmp(state, Y) != 0) {
                //     cout << "Something wrong, please enter again(enter Q/q to quit): ";
                // } 
                // else {
                //     cout << "Please enter your password again: ";
                    
                // }
            }
        }
    }
}

string handle_name_or_password(SOCKET sock, bool isPasswd, bool isFirst) {
    string str = "";
    while (true) {
        if (isPasswd) {
            char c;
            int k = 0;
            while ((c = getch()) != '\r') {
                if (c == '\b') {
                    if (k > 0) {
                        --k;
                        cout << "\b \b";
                        str.pop_back();
                    }
                    else {
                        putch(7);
                        ++k;
                    }
                }
                else {
                    cout << "*";
                    str += c;
                }
            }
            cout << '\n';
        }
        else {
            cin >> str;
        }
        if (str == "q" || str == "Q") {
            send(sock, str.c_str(), str.size(), 0);
            return "Q";
        }
        if (str.size() > 20) {
            cout << "Too long, please change another one(enter Q/q to quit): ";
        } else if (str.size() < 2) {
            cout << "Too short, please change another one(enter Q/q to quit): ";
        } else {
            if (isFirst) return str;
            str = GbkToUtf8(str.c_str());
            send(sock, str.c_str(), str.size(), 0);
            break;
        }
    }
    return str;
}

int main() {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        return -1;
    }

    /*---------------------------------------------------------------------------------------------------*/

    // create a socket to connect with the server
    SOCKET client_sock;
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        return -1;
    }
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;

    int server_port = 1230;
    char server_ip[16] = "47.109.81.162";
    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = inet_addr(server_ip);
    // connect the server
    if (connect(client_sock, (SOCKADDR*)&addr, sizeof(addr))) {
        perror("connect");
        return -1;
    }

    // login or register
    cout << "Welcome to FreelyChat!\n";
    start(client_sock);

    // create a new thread to handle receive message
    HANDLE recv_thread =
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_recv,
                     (void*)&client_sock, 0, NULL);

    HANDLE send_thread =
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_send,
                     (void*)&client_sock, 0, NULL);

    WaitForSingleObject(recv_thread, INFINITE);
    WaitForSingleObject(send_thread, INFINITE);

    closesocket(client_sock);

    WSACleanup();
    return 0;
}