//
//  main.cpp
//  ip2mail
//
//  Created by BlueCocoa on 15/7/29.
//  Copyright (c) 2015 0xBBC. All rights reserved.
//

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>
#include <netdb.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

/**
 *  @brief  Key保存接口名, Value保存对应的IP地址
 */
map<string, string> iptable;

struct data6  {
    unsigned int d4:6;
    unsigned int d3:6;
    unsigned int d2:6;
    unsigned int d1:6;
};

/**
 *  @brief  转为base64编码
 *
 *  Thanks to http://blog.csdn.net/ailinty/article/details/6980636
 */
void base64(char *, char *, int);

char con628(char);

/**
 *  @brief  打开socket
 */
int open_socket(struct sockaddr * addr);

/**
 *  @brief  查找所请求域名的任一IP
 *
 *  @param  domain_name 需要查询的域名
 *
 *  @return 该域名的任一IP
 */
std::string nslookup1(const char * domain_name);

/**
 *  @brief  发送邮件
 *
 *  @param username 用户名
 *  @param password 密码
 *  @param email    收件人
 *  @param smtp     SMTP服务器地址
 *  @param subject  邮件主题
 *  @param body     内容
 */
void sendmail(const char * username, const char * password, const char * email, const char * smtp,const char * subject, const char * body);

/**
 *  @brief  获取本地所有接口的IP地址
 */
void get_all_ip(void);

/**
 *  @brief 输出启动时间
 */
void uptime(void);

void base64(char * dbuf, char * buf128, size_t len) {
    struct data6 *ddd = NULL;
    int i = 0;
    char buf[256] = {0};
    char *tmp = NULL;
    char cc = '\0';
    memset(buf, 0, 256);
    strcpy(buf, buf128);
    
    for(i = 1; i <= len/3; i++) {
        tmp = buf+(i-1)*3;
        cc = tmp[2];
        tmp[2] = tmp[0];
        tmp[0] = cc;
        ddd = (struct data6 *)tmp;
        dbuf[(i-1)*4+0] = con628((unsigned int)ddd->d1); dbuf[(i-1)*4+1] = con628((unsigned int)ddd->d2); dbuf[(i-1)*4+2] = con628((unsigned int)ddd->d3); dbuf[(i-1)*4+3] = con628((unsigned int)ddd->d4);
    }
    
    if(len % 3 == 1) {
        tmp = buf+(i-1)*3;
        cc = tmp[2];
        tmp[2] = tmp[0];
        tmp[0] = cc;
        ddd = (struct data6 *)tmp;
        dbuf[(i-1)*4+0] = con628((unsigned int)ddd->d1); dbuf[(i-1)*4+1] = con628((unsigned int)ddd->d2); dbuf[(i-1)*4+2] = '=';
        dbuf[(i-1)*4+3] = '=';
    }
    
    if(len % 3 == 2) {
        tmp = buf+(i-1)*3;
        cc = tmp[2];
        tmp[2] = tmp[0];
        tmp[0] = cc;
        ddd = (struct data6 *)tmp;
        dbuf[(i-1)*4+0] = con628((unsigned int)ddd->d1); dbuf[(i-1)*4+1] = con628((unsigned int)ddd->d2); dbuf[(i-1)*4+2] = con628((unsigned int)ddd->d3); dbuf[(i-1)*4+3] = '=';
    }
    return;
}

char con628(char c6) {
    char rtn = '\0';
    if (c6 < 26) rtn = c6 + 65;
    else if (c6 < 52) rtn = c6 + 71;
    else if (c6 < 62) rtn = c6 - 4;
    else if (c6 == 62) rtn = 43;
    else rtn = 47;
    return rtn;
}

int open_socket(struct sockaddr * addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        fprintf(stderr, "Open sockfd(TCP) error!\n");
        exit(-1);
    }
    if(connect(sockfd, addr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "Connect sockfd(TCP) error!\n");
        exit(-1);
    }
    return sockfd;
}

std::string nslookup1(const char * domain_name) {
    struct addrinfo hints, *res, *res0;
    int error;
    int s = -1;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(domain_name, "25", &hints, &res0);
    
    if (!error) {
        for (res = res0; res; res = res->ai_next) {
            s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (s < 0) continue;
            
            if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
                close(s);
                continue;
            }
            
            char hbuf[NI_MAXHOST];
            if (!getnameinfo(res->ai_addr, sizeof(struct sockaddr_storage), hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV)) {
                freeaddrinfo(res0);
                return std::string(hbuf);
            }
        }
    }
    return NULL;
}

void sendmail(const char * username, const char * password, const char * email, const char * smtp,const char * subject, const char * body) {
    struct sockaddr_in addr = { 0 };
    char buf[4096]  = { 0 };
    char rbuf[4096] = { 0 };
    char login[128] = { 0 };
    char pass[128]  = { 0 };
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(25);
    addr.sin_addr.s_addr = inet_addr(nslookup1(smtp).c_str());

    int sockfd = open_socket((struct sockaddr *)&addr);
    memset(rbuf,0,4096);
    while(recv(sockfd, rbuf, 4096, 0) == 0)
    {
        uptime();
        printf("reconnect...\n");
        sleep(2);
        sockfd = open_socket((struct sockaddr *)&addr);
        memset(rbuf,0,4096);
    }
    uptime();
    printf("%s", rbuf);
    
    // EHLO
    memset(buf, 0, 4096);
    sprintf(buf, "HELO %s\r\n",smtp);
    send(sockfd, buf, strlen(buf), 0);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // AUTH LOGIN
    memset(buf, 0, 4096);
    sprintf(buf, "AUTH LOGIN\r\n");
    send(sockfd, buf, strlen(buf), 0);
    uptime();
    printf("%s", buf);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // USER
    memset(buf, 0, 4096);
    sprintf(buf,"%s",username);
    memset(login, 0, 128);
    base64(login, buf, strlen(buf));
    sprintf(buf, "%s\r\n", login);
    send(sockfd, buf, strlen(buf), 0);
    uptime();
    printf("%s", buf);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // PASSWORD
    sprintf(buf, "%s",password);
    base64(pass, buf, strlen(buf));
    sprintf(buf, "%s\r\n", pass);
    send(sockfd, buf, strlen(buf), 0);
    uptime();
    printf("%s", buf);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // MAIL FROM
    memset(buf, 0, 4096);
    sprintf(buf, "MAIL FROM:<%s>\r\n",username);
    send(sockfd, buf, strlen(buf), 0);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // RCPT TO
    memset(buf, 0, 4096);
    sprintf(buf, "RCPT TO:<%s>\r\n", email);
    send(sockfd, buf, strlen(buf), 0);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // DATA
    memset(buf, 0, 4096);
    sprintf(buf, "DATA\r\n");
    send(sockfd, buf, strlen(buf), 0);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // TO
    memset(buf, 0, 4096);
    sprintf(buf, "TO:%s\r\n",email);
    send(sockfd, buf, strlen(buf), 0);
    
    // FROM
    memset(buf, 0, 4096);
    sprintf(buf, "FROM:%s\r\n",username);
    send(sockfd, buf, strlen(buf), 0);
    
    // Subject
    memset(buf, 0, 4096);
    sprintf(buf, "Subject: %s\r\n\r\n", subject);
    send(sockfd, buf, strlen(buf), 0);
    memset(buf, 0, 4096);
    sprintf(buf, "%s\r\n", body);
    send(sockfd, buf, strlen(buf), 0);
    memset(buf, 0, 4096);
    sprintf(buf, ".\r\n");
    send(sockfd, buf, strlen(buf), 0);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    
    // QUIT
    memset(buf, 0, 4096);
    sprintf(buf, "QUIT\r\n");
    send(sockfd, buf, strlen(buf), 0);
    memset(rbuf, 0, 4096);
    recv(sockfd, rbuf, 4096, 0);
    uptime();
    printf("%s", rbuf);
    return;
}

void get_all_ip(void) {
    struct ifaddrs * interfaces;
    char str[INET_ADDRSTRLEN];
    if (getifaddrs(&interfaces)) return;
    struct ifaddrs * address = interfaces;
    while (address) {
        if(address->ifa_addr->sa_family == AF_INET) {
            inet_ntop(AF_INET, &((struct sockaddr_in *)address->ifa_addr)->sin_addr, str, INET_ADDRSTRLEN);
            iptable[address->ifa_name] = str;
        }
        address = address->ifa_next;
    }
    freeifaddrs(interfaces);
}

void uptime(void) {
    FILE * fp = fopen("/proc/stat", "r");
    int usr, nice, sys, idle, io, irq, sirq;
    fscanf(fp, "cpu %d %d %d %d %d %d %d", &usr, &nice, &sys, &idle, &io, &irq, &sirq);
    printf("[%12.6lf] ",((usr + nice + sys + idle + io + irq + sirq)/100.0));
    fclose(fp);
}

int main(int argc, const char * argv[]) {
    get_all_ip();
    
    char ip_data[4096]  = { 0 };
    memset(ip_data, 0, 4096);
    for (map<string, string>::iterator iter = iptable.begin(); iter != iptable.end(); iter++) {        
        strcat(ip_data, iter->first.c_str());
        strcat(ip_data, ":");
        strcat(ip_data, iter->second.c_str());
        strcat(ip_data, "\n");
    }
    
    char date[256]  = { 0 };
    memset(date, 0, 256);
    time_t result = time(NULL);
    if(result != -1) sprintf(date, "IP address of my Raspberry Pi - %s", asctime(localtime(&result)));
    
    sendmail("Your Email Username", "Your Email Password", "Send To", "SMTP Server Address (smtp.126.com)", date, ip_data);
    return 0;
}
