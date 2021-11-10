#ifndef LMIME
#define LMIME

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
using namespace std;

enum contentType_e
{
    textPlain,
    textHTML,
    multiPart,
    pdf,
    msword
};
enum contentDisposition_e
{
	attachment
};
enum charset_e
{
    utf8,
    gb2312
};
enum format_e
{
    fixed
};
enum contentTransEncoding_e
{
    quotedPrintable,
    sevenBit,
    base64
};
struct mailPart
{
    contentType_e contentType;
    charset_e charset;
    format_e format;
    contentTransEncoding_e contentTypeEncoding;
	contentDisposition_e contentDisposition;
    string boundary;
	string filename;
    int begin, end, contentBegin;
    vector<mailPart> subPart;
};

class Lmime
{
private:
    mailPart message;
    string mail;
    void decode_helper(mailPart &part);
    void getAllParts_helper(vector<mailPart> &parts, const mailPart &msg);
    string Decode(const char *Data, int DataByte, int &OutByte);

public:
    Lmime(const string &filename);
    Lmime();
    bool load(const string &filename);
    ~Lmime();
    void decode();
    inline const mailPart &getMessage()
    {
        return message;
    }
    inline const string &getMail()
    {
        return mail;
    }
    void partWrite(const string &filename, const mailPart &message);
    void getBody(string &output, const mailPart &message);
    void QPdecode(string &str);
    void getAllParts(vector<mailPart> &parts);
};

#endif
