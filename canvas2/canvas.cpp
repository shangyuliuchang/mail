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
    for (size_t i = 0; i < course.size(); i++)
    {
        ss.clear();
        ss.str("");
        ss << "https://umjicanvas.com/api/v1/courses/" << course[i].id << "/files?per_page=50&sort=updated_at&order=desc" << token;
        getMessage(ss.str());
        decodeMessage(singleCourseFile, fileKeys);
        files.push_back(singleCourseFile);
    }
}
void storeFileNames(const vector<info> &course, const vector<vector<info>> &files)
{
    ofstream f;
    stringstream ss;
    if (course.size() != files.size())
        return;
    for (size_t i = 0; i < course.size(); i++)
    {
        ss.str("");
        ss.clear();
        ss << "filenames/" << course[i].id << ".txt";
        f.open(ss.str(), ios::out);
        for (size_t j = 0; j < files[i].size(); j++)
        {
            f << files[i][j].id << " " << files[i][j].date << " " << endl;
        }
        f.close();
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
    for (size_t i = 0; i < files.size(); i++)
    {
        for (size_t j = 0; j < files[i].size(); j++)
        {
            ss.clear();
            ss.str("");
            ss << "https://www.umjicanvas.com/api/v1/files/" << files[i][j].id << "/public_url?" << token;
            getMessage(ss.str());
            decodeMessage(publicURLs, urlKeys);
            files[i][j].url = publicURLs[0].url;
        }
    }
}
int main(void)
{
    vector<info> courseInfo;
    vector<vector<info>> anns;
    vector<info> calendarInfo;
    vector<vector<info>> files;
    vector<vector<info>> originalFiles;
    curl_global_init(CURL_GLOBAL_ALL);
    getCourses(courseInfo);
    getAllAnns(courseInfo, anns);
    getCalendar(courseInfo, calendarInfo);
    getFiles(courseInfo, files);
    loadFileNames(courseInfo, originalFiles);
    // storeFileNames(courseInfo, files);
    removeOldFiles(files, originalFiles);
    getPublicURL(files);
    curl_global_cleanup();
    return 0;
}