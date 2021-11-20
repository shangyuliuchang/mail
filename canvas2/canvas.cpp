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
using namespace std;
string recvHead, recvBody;
const string token = "&access_token=6zTBIIdMfSGlGSF7SXrLWQeOLsXD565PzxK4QEQQ3yqFn5djPmosfH1gzxNTj4Ia";
size_t writeToString(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (stream != &recvBody)
        return nmemb;
    recvBody = recvBody + (char *)ptr;
    return nmemb;
}
void getMessage(const string &url)
{
    CURL *curl;
    recvBody.clear();
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &recvHead);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recvBody);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
struct info
{
    int id;
    string name;
    string title;
    string url;
    string due;
    bool read;
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
                    return;
                tmpStr = recvBody.substr(pos, end - pos);
                if (tmpStr.data()[0] == '\"' && tmpStr.data()[tmpStr.length() - 1] == '\"')
                    tmpStr = tmpStr.substr(1, tmpStr.length() - 2);
                if (keys[i].key.find("name") != string::npos)
                    newInfo.name = tmpStr;
                if (keys[i].key.find("title") != string::npos)
                    newInfo.title = tmpStr;
                if (keys[i].key.find("due") != string::npos)
                {
                    newInfo.due = tmpStr;
                    sscanf(tmpStr.data(), "%d-%d-%dT%d:%d:%dZ", &(tmpTm.tm_year), &(tmpTm.tm_mon), &(tmpTm.tm_mday), &(tmpTm.tm_hour), &(tmpTm.tm_min), &(tmpTm.tm_sec));
                    tmpTm.tm_year -= 1900;
                    tmpTm.tm_mon--;
                    tmpTime = mktime(&tmpTm);
                    tmpTime += 60 * 60 * 8;
                    tmpTm = *gmtime(&tmpTime);
                    newInfo.time = tmpTm;
                }
                if (keys[i].key.find("url") != string::npos)
                    newInfo.url = tmpStr;
                if (keys[i].key.find("read") != string::npos)
                    if (tmpStr == "read")
                        newInfo.read = true;
                    else
                        newInfo.read = false;
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
    time_t nextMon = now + 3600 * 24 * 30;
    struct tm *nowInfo = localtime(&now);
    struct tm *nextMonInfo = localtime(&nextMon);
    char nowStr[100], nextMonStr[100];
    strftime(nowStr, 100, "%F", nowInfo);
    strftime(nextMonStr, 100, "%F", nextMonInfo);
    nowOut = nowStr;
    nextMonOut = nextMonStr;
}
void getAllAnns(const vector<info> &courses, vector<vector<info>> &anns)
{
    stringstream ss;
    vector<info> annInfo;
    vector<values> annKeys({{"title", str}, {"read_state", str}});
    anns.clear();
    if (courses.empty())
        return;
    for (size_t i = 0; i < courses.size(); i++)
    {
        ss.clear();
        ss.str("");
        ss << "https://www.umjicanvas.com/api/v1/courses/" << courses[i].id << "/discussion_topics?only_announcements=true?per_page=50" << token;
        getMessage(ss.str());
        decodeMessage(annInfo, annKeys);
        anns.push_back(annInfo);
    }
}
int main(void)
{
    string courseURL = "https://umjicanvas.com/api/v1/users/self/favorites/courses?per_page=50" + token;
    vector<values> courseKeys({{"id", num}, {"name", str}});
    vector<info> courseInfo;
    vector<vector<info>> anns;
    curl_global_init(CURL_GLOBAL_ALL);
    getMessage(courseURL);
    decodeMessage(courseInfo, courseKeys);
    getAllAnns(courseInfo, anns);
    curl_global_cleanup();
    return 0;
}