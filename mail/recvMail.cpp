#include <iostream>
#include <csignal>
#include <pthread.h>
#include <gmime-2.6/gmime/gmime.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "Lmime.hpp"
using namespace std;
int finish = 0;
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
int notSeen[1000];
char *tmpstr;
int cnt;
char chr, chr2, chr1;
int recvFileFree = 0;
GMimeMessage *message;
GMimeStream *stream, filtedStream;
GMimeParser *parser;
GMimeFilter *filter;
GMimeObject *gmobj;
int fd;
int sockfdtrans, sockac;

void *decodeAndTrans(void *arg);

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

void signalHandler(int signum)
{
    printf("get signal %d\n", signum);
    finish = 1;
}
int main()
{
    signal(SIGINT, signalHandler);
    FILE *fp, *fw;
    int len;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int sockac;
    char buffer[4096], rec[4096];
    struct sockaddr_in servaddr;
    pthread_t th;
    int arg = 0;
    int *thread_ret = NULL;
start:
    pthread_create(&th, NULL, decodeAndTrans, &arg);
    refresh = 0;
    while (!finish)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&servaddr, 0, sizeof(servaddr));
        struct hostent *host = gethostbyname("imap.sjtu.edu.cn");
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
        servaddr.sin_port = htons(143);
        servaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
        int i = 1;
        while (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
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

        strcpy(buffer, "a001 LOGIN shangyuliuchang@sjtu.edu.cn LClc20010304\r\n");
        send(sockfd, buffer, strlen(buffer), 0);
        memset(rec, 0, sizeof(rec));
        while (!strstr(rec, "LOGIN completed"))
        {
            memset(rec, 0, sizeof(rec));
            while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
                usleep(1000 * 100);
            if (len < 0)
                break;
            printf("%s", rec);
        }
        strcpy(buffer, "a002 LIST \"\" *\r\n");
        send(sockfd, buffer, strlen(buffer), 0);
        memset(rec, 0, sizeof(rec));
        while (!strstr(rec, "LIST completed"))
        {
            memset(rec, 0, sizeof(rec));
            while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
                usleep(1000 * 100);
            if (len < 0)
                break;
            printf("%s", rec);
        }

        // while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
        //     Sleep(100);
        // if (len < 0)
        //     goto skip;
        // printf("hello: %s", rec);

        rec[0] = 0;
        for (int i = 0; i < 1000; i++)
            notSeen[i] = -1;
        strcpy(buffer, "a003 SELECT INBOX\r\n");
        send(sockfd, buffer, strlen(buffer), 0);
        memset(rec, 0, sizeof(rec));
        while (!strstr(rec, "SELECT completed"))
        {
            memset(rec, 0, sizeof(rec));
            while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
                usleep(100);
            if (len < 0)
                break;
            printf("%s", rec);
            sscanf(rec, "* %d EXISTS", notSeen);
        }

        strcpy(buffer, "a005 SEARCH NOT SEEN\r\n");
        send(sockfd, buffer, strlen(buffer), 0);
        memset(rec, 0, sizeof(rec));
        while (!strstr(rec, "SEARCH completed"))
        {
            memset(rec, 0, sizeof(rec));
            while ((len = recv(sockfd, rec, sizeof(rec), 0)) == 0)
                usleep(1000 * 100);
            if (len < 0)
                break;
            printf("%s", rec);
            // sscanf(rec, "* SEARCH %d", notSeen);
        }

        recvFileFree = 0;
        fp = fopen("recvMail.txt", "w");
        printf("%d\n", notSeen[0]);
        if (notSeen[0] > 0)
        {
            sprintf(buffer, "a006 fetch %d RFC822\r\n", notSeen[0]);
            // strcpy(buffer, "a004 fetch 5797 RFC822\r\n");
            send(sockfd, buffer, strlen(buffer), 0);
            printf("%s\n", buffer);
            memset(rec, 0, sizeof(rec));
            cnt = 0;
            while (!strstr(rec, "FETCH completed"))
            {
                memset(rec, 0, sizeof(rec));
                while ((len = recv(sockfd, rec, sizeof(rec) - 1, 0)) == 0)
                {
                    usleep(1000 * 100);
                    cnt++;
                    if (cnt > 100)
                        break;
                }
                cnt++;
                if (cnt > 100)
                    break;
                if (len < 0)
                    break;
                fprintf(fp, "%s", rec);
            }
        }
        fclose(fp);
        close(sockfd);

        fp = fopen("recvMail.txt", "r");
        fw = fopen("recvMail.tmp", "w");
        cnt = 0;
        while (!feof(fp))
        {
            if (cnt == 0)
            {
                while (fgetc(fp) != '\n')
                    ;
                cnt++;
            }
            else
            {
                chr = fgetc(fp);
                if (chr == '(')
                    cnt++;
                if (chr == ')')
                    cnt--;
                if (cnt == 0)
                    break;
                fputc(chr, fw);
            }
        }
        fclose(fp);
        fclose(fw);
        remove("recvMail.txt");
        rename("recvMail.tmp", "recvMail.txt");

        printf("receive done!\n");
        //fp=fopen("recvMail.txt","rb");

        Lmime lm;
        vector<mailPart> allParts;
        lm.load("recvMail.txt");
        lm.decode();
        printf("decode done!\n");
        lm.getAllParts(allParts);
        for (int i = 0; i < allParts.size(); i++)
        {
            if (allParts[i].contentType == textHTML)
            {
                lm.partWrite("decode", allParts[i]);
                printf("success\n");
                system("sudo cp /home/pi/project/mail/decode.html /var/www/html/index.html");
                break;
            }
        }

        recvFileFree = 1;
        for (int i = 0; i < 60; i++)
        {
            sleep(1);
            if (finish)
                break;
        }
        printf("start the next loop\n");
    }
    printf("start quit\n");
    shutdown(sockfdtrans, SHUT_RDWR);
    //close(sockfdtrans);
    pthread_join(th, (void **)&thread_ret);
    printf("noramlly quit\n");
    return 0;
}

void *decodeAndTrans(void *arg)
{
    struct sockaddr_in servaddr;
    FILE *fw;
    Lmime lm;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(1500);
    sockfdtrans = socket(PF_INET, SOCK_STREAM, 0);
    bind(sockfdtrans, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(sockfdtrans, SOMAXCONN);
    vector<mailPart> allParts;
    while (!finish)
    {
        cnt = sizeof(servaddr);
        sockac = accept(sockfdtrans, (struct sockaddr *)&servaddr, (socklen_t *)&cnt);
        printf("connection!\n");
        if (sockac == -1)
            break;

        while (!recvFileFree)
            usleep(1000 * 100);
        lm.load("recvMail.txt");
        lm.decode();
        printf("decode done!\n");
        lm.getAllParts(allParts);
        for (int i = 0; i < allParts.size(); i++)
        {
            if (allParts[i].contentType == textHTML)
            {
                lm.partWrite("decode", allParts[i]);
                printf("success\n");
                system("sudo cp /home/pi/project/mail/decode.html /var/www/html/index.html");
                ifstream input;
                input.open("decode.html", ios::in);
                stringstream ss;
                ss << input.rdbuf();
                string trans(ss.str());
                input.close();
                ss.clear();

                send(sockac, trans.data(), trans.length(), 0);
                break;
            }
        }
        close(sockac);
    }
    close(sockfdtrans);
    return NULL;
}
