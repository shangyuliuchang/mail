#include <iostream>
#include <chrono>
#include <time.h>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
using namespace std;
struct getMsg_t
{
    const string *url;
    string *recv;
};
struct dlFile_t
{
    const string *url;
    const string *filename;
};
string recvHead, recvBody;
int sockfd, sockac;
int finish = 0;
int fileFree = 1;
const string token = "&access_token=6zTBIIdMfSGlGSF7SXrLWQeOLsXD565PzxK4QEQQ3yqFn5djPmosfH1gzxNTj4Ia";
size_t writeToString(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (stream == &recvHead)
        return nmemb;
    *((string *)stream) = *((string *)stream) + (char *)ptr;
    return nmemb;
}
size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (stream == nullptr)
        return nmemb;
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}
void downloadFile(const string &url, const string &filename)
{
    CURL *curl;
    FILE *head, *body;
    string completeName;
    completeName = "files/" + filename;
    body = fopen(completeName.data(), "wb");
    // head = fopen("header", "wb");
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
    // curl_easy_setopt(curl, CURLOPT_WRITEHEADER, head);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, nullptr);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(body);
    // fclose(head);
}
void downloadFile(const string *url, const string *filename)
{
    CURL *curl;
    FILE *head, *body;
    string completeName;
    completeName = "files/" + *filename;
    body = fopen(completeName.data(), "wb");
    // head = fopen("header", "wb");
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url->data());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, nullptr);
    // curl_easy_setopt(curl, CURLOPT_WRITEHEADER, head);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(body);
    // fclose(head);
}
void getMessage(const string &url)
{
    CURL *curl;
    recvBody.clear();
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &recvHead);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recvBody);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
void getMessage(const string *url, string *recv)
{
    CURL *curl;
    recvBody.clear();
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url->data());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &recvHead);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, recv);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
void *getMessageMulti(void *infoStruct)
{
    getMessage(((getMsg_t *)infoStruct)->url, ((getMsg_t *)infoStruct)->recv);
    return NULL;
}
void *downloadFileMulti(void *info)
{
    downloadFile(((dlFile_t *)info)->url, ((dlFile_t *)info)->filename);
    return NULL;
}
struct info
{
    int id;
    string name;
    string title;
    string url;
    string date;
    string timeStr;
    bool read;
    bool submitted;
    tm time;
};
enum valueType
{
    str,
    num
};
struct values
{
    string key;
    valueType valType;
};
struct decode_t
{
    vector<info> *lists;
    const vector<values> *value;
    const string *recv;
};
void decodeMessage(vector<info> &list, const vector<values> &keys, const string &recvBody)
{
    list.clear();
    if (keys.empty())
        return;
    if (recvBody.empty())
        return;
    int pos = 0, end = 0;
    info newInfo;
    string tmpStr;
    int tmpNum;
    bool tmTf;
    time_t tmpTime;
    tm tmpTm;
    while (pos < recvBody.length())
    {
        for (size_t i = 0; i < keys.size(); i++)
        {
            pos = recvBody.find('\"' + keys[i].key + "\":", pos);
            if (pos == string::npos)
                return;
            pos += keys[i].key.length() + 3;
            if (keys[i].valType == str)
            {
                end = recvBody.find(",\"", pos);
                if (end == string::npos)
                {
                    end = recvBody.find("}", pos);
                    if (end == string::npos)
                        return;
                }
                tmpStr = recvBody.substr(pos, end - pos);
                if (tmpStr.data()[0] == '\"' && tmpStr.data()[tmpStr.length() - 1] == '\"')
                    tmpStr = tmpStr.substr(1, tmpStr.length() - 2);
                if (keys[i].key.find("name") != string::npos)
                    newInfo.name = tmpStr;
                if (keys[i].key.find("title") != string::npos)
                    newInfo.title = tmpStr;
                if (keys[i].key.find("date") != string::npos)
                    newInfo.date = tmpStr;
                if (keys[i].key.find("due") != string::npos)
                {
                    newInfo.date = tmpStr;
                    sscanf(tmpStr.data(), "%d-%d-%dT%d:%d:%dZ", &(tmpTm.tm_year), &(tmpTm.tm_mon), &(tmpTm.tm_mday), &(tmpTm.tm_hour), &(tmpTm.tm_min), &(tmpTm.tm_sec));
                    tmpTm.tm_year -= 1900;
                    tmpTm.tm_mon--;
                    tmpTime = mktime(&tmpTm);
                    tmpTime += 60 * 60 * 8;
                    tmpTm = *localtime(&tmpTime);
                    newInfo.time = tmpTm;
                    newInfo.timeStr = asctime(&tmpTm);
                    newInfo.timeStr = newInfo.timeStr.substr(0, newInfo.timeStr.length() - 1);
                }
                if (keys[i].key.find("url") != string::npos)
                    newInfo.url = tmpStr;
                if (keys[i].key.find("read") != string::npos)
                    if (tmpStr == "read")
                        newInfo.read = true;
                    else
                        newInfo.read = false;
                if (keys[i].key.find("submitted") != string::npos)
                    if (tmpStr == "true")
                        newInfo.submitted = true;
                    else
                        newInfo.submitted = false;
            }
            else if (keys[i].valType == num)
            {
                sscanf(recvBody.data() + pos, "%d", &tmpNum);
                if (keys[i].key.find("id") != string::npos)
                    newInfo.id = tmpNum;
            }
        }
        list.push_back(newInfo);
    }
    if (list.empty())
        cout << recvBody.data() << endl;
}
void *decodeMessageMulti(void *info)
{
    decodeMessage(*(((decode_t *)info)->lists), *(((decode_t *)info)->value), *(((decode_t *)info)->recv));
    return NULL;
}
void decodeMessage(vector<info> &list, const vector<values> &keys)
{
    list.clear();
    if (keys.empty())
        return;
    if (recvBody.empty())
        return;
    int pos = 0, end = 0;
    info newInfo;
    string tmpStr;
    int tmpNum;
    bool tmTf;
    time_t tmpTime;
    tm tmpTm;
    while (pos < recvBody.length())
    {
        for (size_t i = 0; i < keys.size(); i++)
        {
            pos = recvBody.find('\"' + keys[i].key + "\":", pos);
            if (pos == string::npos)
                return;
            pos += keys[i].key.length() + 3;
            if (keys[i].valType == str)
            {
                end = recvBody.find(",\"", pos);
                if (end == string::npos)
                {
                    end = recvBody.find("}", pos);
                    if (end == string::npos)
                        return;
                }
                tmpStr = recvBody.substr(pos, end - pos);
                if (tmpStr.data()[0] == '\"' && tmpStr.data()[tmpStr.length() - 1] == '\"')
                    tmpStr = tmpStr.substr(1, tmpStr.length() - 2);
                if (keys[i].key.find("name") != string::npos)
                    newInfo.name = tmpStr;
                if (keys[i].key.find("title") != string::npos)
                    newInfo.title = tmpStr;
                if (keys[i].key.find("date") != string::npos)
                    newInfo.date = tmpStr;
                if (keys[i].key.find("due") != string::npos)
                {
                    newInfo.date = tmpStr;
                    sscanf(tmpStr.data(), "%d-%d-%dT%d:%d:%dZ", &(tmpTm.tm_year), &(tmpTm.tm_mon), &(tmpTm.tm_mday), &(tmpTm.tm_hour), &(tmpTm.tm_min), &(tmpTm.tm_sec));
                    tmpTm.tm_year -= 1900;
                    tmpTm.tm_mon--;
                    tmpTime = mktime(&tmpTm);
                    tmpTime += 60 * 60 * 8;
                    tmpTm = *localtime(&tmpTime);
                    newInfo.time = tmpTm;
                    newInfo.timeStr = asctime(&tmpTm);
                    newInfo.timeStr = newInfo.timeStr.substr(0, newInfo.timeStr.length() - 1);
                }
                if (keys[i].key.find("url") != string::npos)
                    newInfo.url = tmpStr;
                if (keys[i].key.find("read") != string::npos)
                    if (tmpStr == "read")
                        newInfo.read = true;
                    else
                        newInfo.read = false;
                if (keys[i].key.find("submitted") != string::npos)
                    if (tmpStr == "true")
                        newInfo.submitted = true;
                    else
                        newInfo.submitted = false;
            }
            else if (keys[i].valType == num)
            {
                sscanf(recvBody.data() + pos, "%d", &tmpNum);
                if (keys[i].key.find("id") != string::npos)
                    newInfo.id = tmpNum;
            }
        }
        list.push_back(newInfo);
    }
}
void getCurrentTime(string &nowOut, string &nextMonOut)
{
    using namespace std::chrono;
    time_t now = system_clock::to_time_t(system_clock::now());
    struct tm *nowInfo = localtime(&now);
    char nowStr[100];
    strftime(nowStr, 100, "%F", nowInfo);
    nowOut = nowStr;
    now += 3600 * 24 * 30;
    nowInfo = localtime(&now);
    strftime(nowStr, 100, "%F", nowInfo);
    nextMonOut = nowStr;
}
void getAndDecodeMsgMulti(const vector<string> &urls, const vector<values> &keys, vector<vector<info>> &output)
{
    output.clear();
    if (urls.empty() || keys.empty())
        return;
    vector<pthread_t> threads(urls.size());
    vector<getMsg_t> getMsgs(urls.size());
    vector<string> rslts(urls.size());
    vector<decode_t> decodes(urls.size());
    output.resize(urls.size());
    int *ret = NULL;
    for (size_t i = 0; i < urls.size(); i++)
    {
        getMsgs[i].url = &(urls[i]);
        getMsgs[i].recv = &(rslts[i]);
        pthread_create(&(threads[i]), NULL, getMessageMulti, (void *)(&(getMsgs[i])));
    }
    for (size_t i = 0; i < urls.size(); i++)
        pthread_join(threads[i], (void **)&ret);
    for (size_t i = 0; i < urls.size(); i++)
    {
        decodes[i].lists = &(output[i]);
        decodes[i].value = &(keys);
        decodes[i].recv = &(rslts[i]);
        pthread_create(&(threads[i]), NULL, decodeMessageMulti, (void *)(&(decodes[i])));
    }
    for (size_t i = 0; i < urls.size(); i++)
        pthread_join(threads[i], (void **)&ret);
}
void getAllAnns(const vector<info> &courses, vector<vector<info>> &anns)
{
    stringstream ss;
    vector<info> annInfo;
    vector<values> annKeys({{"title", str}, {"read_state", str}});
    vector<string> urls;
    anns.clear();
    if (courses.empty())
        return;
    for (size_t i = 0; i < courses.size(); i++)
    {
        ss.clear();
        ss.str("");
        ss << "https://www.umjicanvas.com/api/v1/courses/" << courses[i].id << "/discussion_topics?only_announcements=true?per_page=50" << token;
        urls.push_back(ss.str());
    }
    getAndDecodeMsgMulti(urls, annKeys, anns);
}
void downloadFilesMulti(const vector<string> &urls, const vector<string> &filenames)
{
    vector<pthread_t> threads(urls.size());
    vector<dlFile_t> infos(urls.size());
    int *ret = nullptr;
    for (size_t i = 0; i < urls.size(); i++)
    {
        infos[i].url = &(urls[i]);
        infos[i].filename = &(filenames[i]);
        pthread_create(&(threads[i]), NULL, downloadFileMulti, &(infos[i]));
    }
    for (size_t i = 0; i < urls.size(); i++)
        pthread_join(threads[i], (void **)&ret);
}
void getCourses(vector<info> &courseInfo)
{
    string courseURL = "https://umjicanvas.com/api/v1/users/self/favorites/courses?per_page=50" + token;
    vector<values> courseKeys({{"id", num}, {"name", str}});
    getMessage(courseURL);
    decodeMessage(courseInfo, courseKeys);
}
void getCalendar(const vector<info> &course, vector<info> &calendar)
{
    stringstream ss;
    string now, nextMon;
    ss.clear();
    ss.str("");
    ss << "https://umjicanvas.com/api/v1/calendar_events?type=assignment&per_page=50";
    for (size_t i = 0; i < course.size(); i++)
        ss << "&context_codes[]=course_" << course[i].id;
    getCurrentTime(now, nextMon);
    ss << "&start_date=" << now;
    ss << "&end_date=" << nextMon;
    ss << token;
    vector<values> calendarKeys({{"title", str}, {"due_at", str}, {"user_submitted", str}});
    getMessage(ss.str());
    decodeMessage(calendar, calendarKeys);
}
void getFiles(const vector<info> &course, vector<vector<info>> &files)
{
    files.clear();
    vector<info> singleCourseFile;
    stringstream ss;
    vector<values> fileKeys({{"id", num}, {"display_name", str}, {"updated_at", str}});
    vector<string> urls;
    for (size_t i = 0; i < course.size(); i++)
    {
        ss.clear();
        ss.str("");
        ss << "https://umjicanvas.com/api/v1/courses/" << course[i].id << "/files?per_page=50&sort=updated_at&order=desc" << token;
        urls.push_back(ss.str());
    }
    getAndDecodeMsgMulti(urls, fileKeys, files);
}
void storeFileNames(const vector<info> &course, const vector<vector<info>> &files, const vector<vector<info>> &originalFiles)
{
    ofstream f;
    stringstream ss;
    if (course.size() != files.size())
        return;
    for (size_t i = 0; i < course.size(); i++)
    {
        // if (!originalFiles[i].empty())
        // {
        ss.str("");
        ss.clear();
        ss << "filenames/" << course[i].id << ".txt";
        f.open(ss.str(), ios::out);
        for (size_t j = 0; j < files[i].size(); j++)
        {
            f << files[i][j].id << " " << files[i][j].date << " " << endl;
        }
        f.close();
        // }
    }
}
void loadFileNames(const vector<info> &course, vector<vector<info>> &files)
{
    stringstream ss;
    ifstream f;
    string tmp;
    vector<info> singleCourseFiles;
    info newFile;
    char date[100];
    int id;
    files.clear();
    for (size_t i = 0; i < course.size(); i++)
    {
        ss.clear();
        ss.str("");
        ss << "filenames/" << course[i].id << ".txt";
        singleCourseFiles.clear();
        f.open(ss.str(), ios::in);
        while (f.is_open() && !f.eof())
        {
            getline(f, tmp);
            if (tmp.empty())
                break;
            sscanf(tmp.data(), "%d %s", &id, date);
            newFile.id = id;
            newFile.date = date;
            singleCourseFiles.push_back(newFile);
        }
        f.close();
        files.push_back(singleCourseFiles);
    }
}
void removeOldFiles(vector<vector<info>> &filenames, const vector<vector<info>> &oriFilenames)
{
    size_t indexCurr, indexOri;
    for (size_t i = 0; i < filenames.size(); i++)
    {
        indexCurr = 0;
        indexOri = 0;
        if (oriFilenames[i].empty())
        {
            // filenames[i] = vector<info>(filenames[i].begin(), filenames[i].end());
            // continue;
        }
        while (indexCurr < filenames[i].size() && indexOri < oriFilenames[i].size())
        {
            if (filenames[i][indexCurr].id == oriFilenames[i][indexOri].id && filenames[i][indexCurr].date == oriFilenames[i][indexOri].date)
                break;
            if (filenames[i][indexCurr].date > oriFilenames[i][indexOri].date)
                indexCurr++;
            else
                indexOri++;
        }
        filenames[i] = vector<info>(filenames[i].begin(), filenames[i].begin() + indexCurr);
    }
}
void getPublicURL(vector<vector<info>> &files)
{
    stringstream ss;
    vector<values> urlKeys({{"public_url", str}});
    vector<info> publicURLs;
    vector<string> urls;
    vector<vector<info>> rslts;
    int cnt = 0;
    urls.clear();
    for (size_t i = 0; i < files.size(); i++)
    {
        for (size_t j = 0; j < files[i].size(); j++)
        {
            ss.clear();
            ss.str("");
            ss << "https://www.umjicanvas.com/api/v1/files/" << files[i][j].id << "/public_url?" << token;
            urls.push_back(ss.str());
            // getMessage(ss.str());
            // decodeMessage(publicURLs, urlKeys);
            // files[i][j].url = publicURLs[0].url;
        }
    }
    getAndDecodeMsgMulti(urls, urlKeys, rslts);
    cnt = 0;
    for (size_t i = 0; i < files.size(); i++)
    {
        for (size_t j = 0; j < files[i].size(); j++)
        {
            if (!rslts[cnt].empty())
            {
                files[i][j].url = rslts[cnt][0].url;
                cnt++;
            }
        }
    }
}
void downloadFiles(const vector<vector<info>> &files)
{
    ofstream f;
    vector<string> urls;
    vector<string> names;
    f.open("tobesent.txt", ios::app);
    for (size_t i = 0; i < files.size(); i++)
    {
        for (size_t j = 0; j < files[i].size(); j++)
        {
            // downloadFile(files[i][j].url, files[i][j].name);
            urls.push_back(files[i][j].url);
            names.push_back(files[i][j].name);
            f << files[i][j].name << endl;
        }
    }
    f.close();
    downloadFilesMulti(urls, names);
}
int waitFor(int sockac, const string &target)
{
    string buf;
    buf.clear();
    char tmp[100];
    int timeout = 0;
    while (buf.find(target) == string::npos)
    {
        memset(tmp, 0, sizeof(tmp));
        if (recv(sockac, tmp, sizeof(tmp) - 1, 0) == -1)
            return -1;
        buf = buf + tmp;
        timeout++;
        if (timeout > 10)
            return -1;
    }
    return 0;
}
int sendOneFile(int sockac, const string &filename)
{
    if (filename.empty())
        return -1;
    ifstream f;
    stringstream ss, length;
    if (filename == "ann.txt")
        f.open(filename, ios::in);
    else if (filename == "contribution.png")
        f.open("/home/pi/project/canvas/con/data/img/" + filename, ios::in);
    else
        f.open("files/" + filename, ios::in);
    if (!f.is_open())
        return -1;
    ss.clear();
    ss.str("");
    ss << f.rdbuf();
    f.close();
    length.clear();
    length.str("");
    length << ss.str().length();

    if (send(sockac, filename.data(), filename.length(), 0) == -1)
        return -1;
    cout << "wait for the first CONTENT" << endl;
    if (waitFor(sockac, "CONTENT") == -1)
        return -1;
    if (send(sockac, length.str().data(), length.str().length(), 0) == -1)
        return -1;
    cout << "wait for the second CONTENT" << endl;
    if (waitFor(sockac, "CONTENT") == -1)
        return -1;
    if (send(sockac, ss.str().data(), ss.str().length(), 0) == -1)
        return -1;
    cout << "wait for the NEXT" << endl;
    if (waitFor(sockac, "NEXT") == -1)
        return -1;
    return 0;
}
// int sendOneFile(int sockac, const string &filename)
// {
//     if (filename.empty())
//         return -1;
//     string name;
//     char buf[4096];
//     int size;
//     FILE *f;
//     if (filename == "ann.txt")
//         name = filename;
//     else if (filename == "contribution.png")
//         name = "/home/pi/project/canvas/con/data/img" + filename;
//     else
//         name = "files/" + filename;
//     f = fopen(name.data(), "rb");
//     if (!f)
//         return -1;
//     fseek(f, 0, SEEK_END);
//     size = ftell(f);
//     fseek(f, 0, SEEK_SET);
//     sprintf(buf, "%d", size);
//     if (send(sockac, filename.data(), filename.length(), 0) == -1)
//     {
//         fclose(f);
//         return -1;
//     }
//     cout << "wait for the first CONTENT" << endl;
//     if (waitFor(sockac, "CONTENT") == -1)
//     {
//         fclose(f);
//         return -1;
//     }
//     if (send(sockac, buf, strlen(buf), 0) == -1)
//     {
//         fclose(f);
//         return -1;
//     }
//     cout << "wait for the second CONTENT" << endl;
//     if (waitFor(sockac, "CONTENT") == -1)
//     {
//         fclose(f);
//         return -1;
//     }
//     while (!feof(f))
//     {
//         memset(buf, 0, sizeof(buf));
//         size = fread(buf, 1, sizeof(buf) - 1, f);
//         if (send(sockac, buf, size, 0) == -1)
//         {
//             fclose(f);
//             return -1;
//         }
//         // cout << "send size: " << size << endl;
//         if (waitFor(sockac, "OK") == -1)
//         {
//             fclose(f);
//             return -1;
//         }
//     }
//     cout << "wait for the NEXT" << endl;
//     if (waitFor(sockac, "NEXT") == -1)
//     {
//         fclose(f);
//         return -1;
//     }
//     fclose(f);
//     return 0;
// }
void getFilesTobeSent(vector<string> &files)
{
    files.clear();
    ifstream f;
    string str;
    f.open("tobesent.txt", ios::in);
    if (!f.is_open())
        return;
    while (!f.eof())
    {
        getline(f, str);
        if (str.empty())
            break;
        files.push_back(str);
    }
    f.close();
}
void wait(int second)
{
    for (int i = 0; i < second; i++)
    {
        if (finish)
            break;
        sleep(1);
    }
}
void lock()
{
    while (!fileFree)
        usleep(1000 * 100);
    fileFree = 0;
}
void unlock()
{
    fileFree = 1;
}
void *sendToPC(void *arg)
{
    struct sockaddr_in servaddr;
    int cnt = 0;
    int timeout = 5;
    int flag;
    int bufferSize = 1024 * 1024;
    vector<string> files;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(1600);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&bufferSize, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(sockfd, SOMAXCONN);
    while (!finish)
    {
        cnt = sizeof(servaddr);
        sockac = accept(sockfd, (struct sockaddr *)&servaddr, (socklen_t *)&cnt);
        //        setsockopt(sockac, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
        //        setsockopt(sockac, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
        if (sockac == -1)
            break;
        cout << "connected" << endl;
        lock();
        getFilesTobeSent(files);
        files.push_back("contribution.png");
        files.push_back("ann.txt");
        for (size_t i = 0; i < files.size(); i++)
        {
            cout << "sending " << files[i].data() << " start" << endl;
            flag = sendOneFile(sockac, files[i]);
            if (flag == -1)
                break;
            cout << "sending " << files[i].data() << " done" << endl;
        }
        close(sockac);
        if (flag != -1)
        {
            system("rm -f files/*");
            system("rm -f tobesent.txt");
        }
        unlock();
    }
    close(sockfd);
    return NULL;
}
void handler(int signum)
{
    finish = 1;
    shutdown(sockfd, SHUT_RDWR);
}
void writeAnn(const vector<info> &courses, const vector<vector<info>> &anns, const vector<info> &calendar)
{
    ofstream f;
    f.open("ann.txt", ios::out);
    if (!f.is_open())
        return;
    for (size_t i = 2; i < courses.size(); i++)
    {
        f << courses[i].name << endl;
        for (size_t j = 0; j < anns[i].size(); j++)
        {
            if (anns[i][j].read == false)
                f << "    " << anns[i][j].title << endl;
        }
    }
    for (size_t i = 0; i < calendar.size(); i++)
    {
        if (calendar[i].submitted == false)
            f << calendar[i].timeStr << " : " << calendar[i].title << endl;
    }
    f.close();
}
void genContri()
{
    static int cnt = 0;
    cnt = (cnt + 1) % 30;
    if (cnt == 2)
    {
        system("bash /home/pi/project/canvas2/contri/run.sh /home/pi/project/canvas2/contri/");
    }
}
int main(void)
{
    signal(SIGINT, handler);
    pthread_t th;
    int *thread_ret = NULL;
    int ret = pthread_create(&th, NULL, sendToPC, NULL);
    vector<info> courseInfo;
    vector<vector<info>> anns;
    vector<info> calendarInfo;
    vector<vector<info>> files;
    vector<vector<info>> originalFiles;
    curl_global_init(CURL_GLOBAL_ALL);
    while (!finish)
    {
        getCourses(courseInfo);
        cout << "get course done" << endl;
        if (finish)
            break;
        getAllAnns(courseInfo, anns);
        cout << "get anns done" << endl;
        if (finish)
            break;
        getCalendar(courseInfo, calendarInfo);
        cout << "get calendar done" << endl;
        if (finish)
            break;
        lock();
        writeAnn(courseInfo, anns, calendarInfo);
        cout << "write ann.txt done" << endl;
        unlock();
        if (finish)
            break;
        getFiles(courseInfo, files);
        cout << "get files done" << endl;
        if (finish)
            break;
        loadFileNames(courseInfo, originalFiles);
        cout << "get filenames done" << endl;
        if (finish)
            break;
        storeFileNames(courseInfo, files, originalFiles);
        removeOldFiles(files, originalFiles);
        if (finish)
            break;
        cout << "get new files done" << endl;
        getPublicURL(files);
        if (finish)
            break;
        cout << "get urls done" << endl;
        lock();
        downloadFiles(files);
        unlock();
        if (finish)
            break;
        cout << "download new files done" << endl;
        lock();
        genContri();
        unlock();
        if (finish)
            break;
        cout << "genContri done" << endl;
        wait(60);
    }
    curl_global_cleanup();
    pthread_join(th, (void **)&thread_ret);
    return 0;
}
