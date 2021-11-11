#include <curl/curl.h>
#include <sstream>
#include<fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
#include <vector>
#include <algorithm>
using namespace std;
FILE *recvHead;
FILE *recvBody;
int cnt, finish;
struct activity{
	string name;
	int act;
};

struct{
	bool operator()(activity a, activity b) const{
		return a.act<b.act;
	}
}comp;
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream){
	int tmp=0;
	int act;
	if(stream == recvHead)
		return fwrite(ptr, size, nmemb, (FILE *)stream);
	else{
		if(size*nmemb==2) finish=1;
		char *str=(char *)ptr;
		char *pos=(char *)ptr;
//		while(pos<(char *)ptr+size*nmemb){
//			pos=strstr(pos, "\"name\":");
//			if(pos){
//				pos+=8;
//				str=strstr(pos, "\",\"");
//				if(str){
//					for(char *j=pos;j<str;j++)
//						fputc(j[0], (FILE *)stream);
//					pos=strstr(str, "\"total_activity_time\":");
//					if(pos){
//						pos+=22;
//						sscanf(pos, "%d", &act);
//						fprintf((FILE *)stream, ": %d\n", act);
//					}else break;
//				}else break;
//			}else break;
//		}
		for(size_t i=0;i<size*nmemb;i++){
			if(str[i]=='\"') tmp++;
			if(str[i]=='{'){
				fputc(str[i], (FILE *)stream);
				cnt++;
			}
			else if(str[i]=='}'){
				fputc(str[i], (FILE *)stream);
				cnt--;
			}
			else if(str[i]==',' && cnt==0){
				fputc(str[i], (FILE *)stream);
				fputc('\n', (FILE *)stream);
				tmp=0;
			}else 
				fputc(str[i], (FILE *)stream);
		}
		return nmemb;
	}
	if(((char *)ptr)[2]=='i'){
		printf("\n\n\n find one!\n\n\n");
		fwrite("\n", 1, 1, (FILE *)stream);
		fwrite(ptr, size, nmemb, (FILE *)stream);
	}
	return nmemb;
}
int main(void){
	CURL *curl;
	CURLcode ret;
	curl_global_init(CURL_GLOBAL_ALL);
	printf("init done\n");
	char url[1000], filename[100];
	int start;
	//scanf("%d", &start);
	//scanf("%s", filename);
	//system("rm body.txt");
	//system("rm header.txt");
	//for(int i=start;i<2384;i++){
	recvHead=fopen("header.txt", "wb");
	recvBody=fopen("activity.txt", "wb");
	for(int i=1;i<100 && !finish;i++){
		printf("page: %d\n", i);
		curl=curl_easy_init();
		sprintf(url, "https://umjicanvas.com/api/v1/courses/7/users?access_token=6zTBIIdMfSGlGSF7SXrLWQeOLsXD565PzxK4QEQQ3yqFn5djPmosfH1gzxNTj4Ia&include&per_page=50&page=%d&include[]=enrollments", i);
		//sprintf(url, "https://umjicanvas.com/api/v1/courses/7/users?access_token=6zTBIIdMfSGlGSF7SXrLWQeOLsXD565PzxK4QEQQ3yqFn5djPmosfH1gzxNTj4Ia&include&per_page=50&page=%d&include[]=email", i);
		//sprintf(url, "https://umjicanvas.com/api/v1/courses/2242/users/%d?access_token=6zTBIIdMfSGlGSF7SXrLWQeOLsXD565PzxK4QEQQ3yqFn5djPmosfH1gzxNTj4Ia", i);
		//printf("%s\n", url);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, recvHead);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, recvBody);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fwrite("\n", 1, 1, recvBody);
	}
	fclose(recvHead);
	fclose(recvBody);
	stringstream ss;
	ifstream fp;
	fp.open("activity.txt", ios::in);
	ss<<fp.rdbuf();
	string str=ss.str();
	recvBody=fopen("decoded.txt","w");
	int pos, end, act;
	pos=end=0;
	activity newNode;
	vector<activity> list;
	list.clear();
	while(pos<str.size()){
		pos=str.find("\"name\":", pos);
		if(pos!=string::npos){
			pos+=8;
			end=str.find("\",\"", pos);
			if(end!=string::npos){
				//for(int j=pos;j<end;j++)
				//	fputc(str.data()[j], recvBody);
				newNode.name=str.substr(pos, end-pos);
				pos=str.find("\"total_activity_time\":", pos);
				if(pos!=string::npos){
					pos+=22;
					sscanf(str.data()+pos, "%d", &act);
					newNode.act=act;
					//fprintf(recvBody, ": %d\n", act);
					list.push_back(newNode);
				}else break;
			}else break;
		}else break;
	}
	std::sort(list.begin(), list.end(), comp);
	for(int i=0;i<list.size();i++){
		fprintf(recvBody, "%s: %d\n", list[i].name.data(), list[i].act);
	}
	fclose(recvBody);
	fp.close();
	return 0;
}
