#include <iostream>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <stddef.h>
using namespace std;
int finish;
int num = 0;
int *tmp;
int mouseDown = 0;
int points[1000][2];
int first = 1;
int refresh, autorefresh;
int minx, maxx, miny, maxy;
int sendx, sendy, sendwid, sendhei, show;
int lastx, lasty;
int dispText;
int erase;
int pos;

void base64_encode(const char *str, char *res)
{
    long len;
    long str_len;
    int i, j;
    //定义base64编码表
    const char *base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    //计算经过base64编码后的字符串长度
    str_len = strlen(str);
    if (str_len % 3 == 0)
        len = str_len / 3 * 4;
    else
        len = (str_len / 3 + 1) * 4;

    res[len + 0] = '\r';
    res[len + 1] = '\n';
    res[len + 2] = '\0';

    //以3个8位字符为一组进行编码
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
    {
        res[i] = base64_table[str[j] >> 2];                                     //取出第一个字符的前6位并找出对应的结果字符
        res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)];     //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符
        res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
        res[i + 3] = base64_table[str[j + 2] & 0x3f];                           //取出第三个字符的后6位并找出结果字符
    }

    switch (str_len % 3)
    {
    case 1:
        res[i - 2] = '=';
        res[i - 1] = '=';
        break;
    case 2:
        res[i - 1] = '=';
        break;
    }
}

int main()
{
    FILE *fp;
    int len;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[4096], rec[100];
    struct sockaddr_in servaddr;
start:
    refresh = 0;
    memset(&servaddr, 0, sizeof(servaddr));
    struct hostent *host = gethostbyname("smtp.163.com");
    if (!host)
    {
        printf("get addr failed\n");
        return 0;
    }
    else
        for (int i = 0; host->h_addr_list[i]; i++)
        {
            printf("IP: %s\n", inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
        }
    servaddr.sin_family = host->h_addrtype;
    servaddr.sin_port = htons(25);
    servaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
    int i = 1;
    while (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) <0)
    {
        if (host->h_addr_list[i])
        {
            servaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
            i++;
        }
        else
            return 0;
    }
    printf("connect success\n");

    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("hello: %s\n", rec);

    // strcpy(buffer, "EHLO smtp.163.com\r\n");
    // send(sockfd, buffer, strlen(buffer), 0);
    // while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
    //     Sleep(100);
    // if (len < 0)
    //     goto skip;
    // printf("helo: %s\n", rec);

    // strcpy(buffer, "STARTTLS\r\n");
    // send(sockfd, buffer, strlen(buffer), 0);
    // while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
    //     Sleep(100);
    // if (len < 0)
    //     goto skip;
    // printf("helo: %s\n", rec);

    strcpy(buffer, "HELO smtp.163.com\r\n");
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("helo: %s\n", rec);

    strcpy(buffer, "AUTH LOGIN\r\n");
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("hello: %s\n", rec);

    // strcpy(buffer, "c2hhbmd5dWxpdWNoYW5nQDE2My5jb20=\r\n");
    base64_encode("shangyuliuchang@163.com", buffer);
    printf("user: %s\n", buffer);
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    // strcpy(buffer, "MjAwMTAzMDQ=\r\n");
    base64_encode("20010304", buffer);
    printf("psw: %s\n", buffer);
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    strcpy(buffer, "MAIL FROM: <shangyuliuchang@163.com> \r\n");
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    strcpy(buffer, "RCPT TO: <shangyuliuchang@sjtu.edu.cn> \r\n");
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    strcpy(buffer, "DATA\r\n");
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    fp = fopen("mail.txt", "r");
    pos = 0;
    while (!feof(fp))
        buffer[pos++] = fgetc(fp);
    buffer[pos] = 0;
    fclose(fp);
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    strcpy(buffer, "QUIT\r\n");
    send(sockfd, buffer, strlen(buffer), 0);
    while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        sleep(100);
    if (len < 0)
        goto skip;
    printf("%s\n", rec);

    printf("success\n");
skip:
    close(sockfd);
}
