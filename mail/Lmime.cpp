#include "Lmime.hpp"
Lmime::Lmime(const string &filename)
{
    ifstream mailFile;
    mailFile.open(filename, ios::in);
    if (mailFile.is_open())
    {
        stringstream buffer;
        buffer << mailFile.rdbuf();
        mail = buffer.str();
        mailFile.close();
        message.begin = 0;
        message.end = mail.length();
    }
    else
    {
        message.begin = 0;
        message.end = 0;
        mail.clear();
        message.subPart.clear();
    }
}
bool Lmime::load(const string &filename)
{
    ifstream mailFile;
    mailFile.open(filename, ios::in);
    if (mailFile.is_open())
    {
        stringstream buffer;
        buffer << mailFile.rdbuf();
        mail = buffer.str();
        mailFile.close();
        message.begin = 0;
        message.end = mail.length();
        return true;
    }
    else
    {
        message.begin = 0;
        message.end = 0;
        mail.clear();
        message.subPart.clear();
    }
    return false;
}
Lmime::~Lmime()
{
    if (!mail.empty())
    {
        mail.clear();
    }
}
void Lmime::decode()
{
    if (!mail.empty() && message.end > 0)
        decode_helper(message);
}
void Lmime::decode_helper(mailPart &part)
{
    int pos, end;
    part.subPart.clear();
    part.boundary.clear();
    part.charset = utf8;
    part.contentType = textPlain;
    part.contentTypeEncoding = quotedPrintable;
    pos = mail.find("Content-Type: ", part.begin);
    part.contentBegin = part.begin;
    if (pos != string::npos && pos < part.end)
    {
        if (mail.substr(pos + 14, strlen("text/plain")).compare("text/plain") == 0)
        {
            part.contentType = textPlain;
        }
        else if (mail.substr(pos + 14, strlen("text/html")).compare("text/html") == 0)
        {
            part.contentType = textHTML;
        }
        else if (mail.substr(pos + 14, strlen("multipart/")).compare("multipart/") == 0)
        {
            part.contentType = multiPart;
        }
        else if (mail.substr(pos + 14, strlen("application/pdf")).compare("application/pdf") == 0)
        {
            part.contentType = pdf;
        }
        else if (mail.substr(pos + 14, strlen("application/msword")).compare("application/msword") == 0)
        {
            part.contentType = msword;
        }
        else
        {
            part.contentType = textPlain;
        }
        if (pos > part.contentBegin)
            part.contentBegin = pos;
    }
    pos = mail.find("charset=", part.begin);
    if (pos != string::npos && pos < part.end)
    {
        if (mail.substr(pos + strlen("charset="), strlen("UTF-8")).compare("UTF-8") == 0)
        {
            part.charset = utf8;
        }
        else if (mail.substr(pos + strlen("charset="), strlen("gb2312")).compare("gb2312") == 0)
        {
            part.charset = gb2312;
        }
        if (pos > part.contentBegin)
            part.contentBegin = pos;
    }
    pos = mail.find("Content-Transfer-Encoding: ", part.begin);
    if (pos != string::npos && pos < part.end)
    {
        if (mail.substr(pos + strlen("Content-Transfer-Encoding: "), strlen("quoted-printable")).compare("quoted-printable") == 0)
        {
            part.contentTypeEncoding = quotedPrintable;
        }
        if (mail.substr(pos + strlen("Content-Transfer-Encoding: "), strlen("7bit")).compare("7bit") == 0)
        {
            part.contentTypeEncoding = sevenBit;
        }
        if (mail.substr(pos + strlen("Content-Transfer-Encoding: "), strlen("base64")).compare("base64") == 0)
        {
            part.contentTypeEncoding = base64;
        }
        if (pos > part.contentBegin)
            part.contentBegin = pos;
    }
    pos = mail.find("Content-Disposition: ", part.begin);
    if (pos != string::npos && pos < part.end)
    {
        if (mail.substr(pos + strlen("Content-Disposition: "), strlen("attachment")).compare("attachment") == 0)
        {
            part.contentDisposition = attachment;
        }
    }
    pos = mail.find("filename=\"", part.begin);
    if (pos != string::npos && pos < part.end && part.contentDisposition == attachment)
    {
        string seg, decodedSeg;
        int tmpcnt, encode;
        part.filename.clear();
        encode = 0;
        while (1)
        {
            pos = mail.find("=?", pos);
            if (mail.substr(pos + 2, strlen("gb2312")).compare("gb2312") == 0)
                encode = 1;
            pos = mail.find("?", pos + 2);
            pos = mail.find("?", pos + 1);
            pos++;
            end = mail.find("?=", pos);
            seg = mail.substr(pos, end - pos);
            decodedSeg = Decode(seg.data(), seg.size(), tmpcnt);
            part.filename = part.filename + decodedSeg;
            pos = end;
            if (mail.data()[pos + 2] == '\"')
                break;
        }
        printf("decoded filename: %s\n", part.filename.data());
        ofstream storename;
        storename.open("storename.txt", ios::out);
        storename << part.filename.data();
        storename.close();
        if (encode == 1)
        {
            system("iconv -f GB2312 -t UTF-8 storename.txt -o decodedname.txt");
            ifstream f;
            stringstream ss;
            f.open("decodedname.txt", ios::in);
            ss << f.rdbuf();
            part.filename = ss.str();
            f.close();
        }
    }

    if (part.contentType == multiPart)
    {
        pos = mail.find("boundary=", part.begin);
        mailPart newPart;
        if (pos != string::npos && pos < part.end)
        {
            end = mail.find('\n', pos);
            part.boundary = mail.substr(pos + strlen("boundary="), end - pos - strlen("boundary="));
            if (part.boundary.data()[part.boundary.size() - 1] == '\r')
            {
                part.boundary = part.boundary.substr(0, part.boundary.size() - 1);
            }
            if (part.boundary.data()[part.boundary.size() - 1] == ';')
            {
                part.boundary = part.boundary.substr(0, part.boundary.size() - 1);
            }
            if (part.boundary.data()[part.boundary.size() - 1] == '\"')
            {
                part.boundary = part.boundary.substr(0, part.boundary.size() - 1);
            }
            if (part.boundary.data()[0] == '\"')
            {
                part.boundary = part.boundary.substr(1, part.boundary.size() - 1);
            }
            part.boundary = "--" + part.boundary;
            pos += strlen("boundary=\"");
            while (1)
            {
                pos = mail.find(part.boundary, pos + part.boundary.length());
                printf("pos: %d\n", pos);
                if (pos != string::npos && pos < part.end)
                {
                    if (!part.subPart.empty())
                    {
                        part.subPart[part.subPart.size() - 1].end = pos;
                    }
                    newPart.begin = pos + part.boundary.length();
                    newPart.boundary.clear();
                    newPart.subPart.clear();
                    part.subPart.push_back(newPart);
                }
                else
                {
                    part.subPart.pop_back();
                    break;
                }
            }
            printf("out\n");
            for (size_t i = 0; i < part.subPart.size(); i++)
            {
                decode_helper(part.subPart[i]);
            }
            if (pos > part.contentBegin)
                part.contentBegin = pos;
        }
    }
    part.contentBegin = mail.find('\n', part.contentBegin) + 1;
    part.contentBegin = mail.find('\n', part.contentBegin) + 1;
}
void Lmime::partWrite(const string &filename, const mailPart &message)
{
    ofstream fw;
    string tmp;
    string name = filename;
    int len;
    if (name.empty())
    {
        if (message.contentDisposition == attachment)
            name = message.filename;
        else
            name = "default";
    }
    if (name.find('.') == string::npos)
    {
        if (message.contentType == textHTML)
            name = name + ".html";
        else if (message.contentType == pdf)
            name = name + ".pdf";
        else if (message.contentType == msword)
            name = name + ".doc";
        else
            name = name + ".txt";
    }
    fw.open(name, ios::out);
    tmp = mail.substr(message.contentBegin, message.end - message.contentBegin);
    if (message.contentTypeEncoding == quotedPrintable)
        QPdecode(tmp);
    if (message.contentTypeEncoding == base64)
    {
        string decode = Decode(tmp.data(), tmp.length(), len);
        tmp = decode;
    }
    if (message.contentType == textHTML && message.charset == utf8)
        fw << "<meta charset=\"UTF-8\">" << endl;
    fw << tmp;
    fw.close();
}
void Lmime::getBody(string &output, const mailPart &message)
{
    output = mail.substr(message.contentBegin, message.end - message.contentBegin);
    if (message.contentTypeEncoding == quotedPrintable)
        QPdecode(output);
}
void Lmime::QPdecode(string &str)
{
    string out;
    out.clear();
    int pos = 0;
    char tmp;
    char chr[2];
    while (pos < str.length())
    {
        tmp = str.data()[pos++];
        if (tmp != '=')
        {
            out = out + tmp;
        }
        else
        {
            chr[0] = str.data()[pos++];
            chr[1] = str.data()[pos++];
            if (((chr[0] >= '0' && chr[0] <= '9') || (chr[0] >= 'A' && chr[0] <= 'F')) && ((chr[1] >= '0' && chr[1] <= '9') || (chr[1] >= 'A' && chr[1] <= 'F')))
            {
                out = out + (char)((chr[0] >= 'A' ? chr[0] - 'A' + 10 : chr[0] - '0') * 16 + (chr[1] >= 'A' ? chr[1] - 'A' + 10 : chr[1] - '0'));
            }
        }
    }
    str = out;
}

Lmime::Lmime()
{
    mail.clear();
    message.begin = 0;
    message.end = 0;
    message.subPart.clear();
}

void Lmime::getAllParts_helper(vector<mailPart> &parts, const mailPart &msg)
{
    if (msg.contentType != multiPart)
    {
        parts.push_back(msg);
        return;
    }
    else
    {
        for (int i = 0; i < msg.subPart.size(); i++)
        {
            getAllParts_helper(parts, msg.subPart[i]);
        }
    }
}
void Lmime::getAllParts(vector<mailPart> &parts)
{
    parts.clear();
    getAllParts_helper(parts, message);
}
string Lmime::Decode(const char *Data, int DataByte, int &OutByte)
{
    //解码表
    const char DecodeTable[] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            62, // '+'
            0, 0, 0,
            63,                                     // '/'
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
            0, 0, 0, 0, 0, 0, 0,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
            13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
            0, 0, 0, 0, 0, 0,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
            39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
        };
    //返回值
    string strDecode;
    int nValue;
    int i = 0;
    while (i < DataByte)
    {
        if (*Data != '\r' && *Data != '\n')
        {
            nValue = DecodeTable[*Data++] << 18;
            nValue += DecodeTable[*Data++] << 12;
            strDecode += (nValue & 0x00FF0000) >> 16;
            OutByte++;
            if (*Data != '=')
            {
                nValue += DecodeTable[*Data++] << 6;
                strDecode += (nValue & 0x0000FF00) >> 8;
                OutByte++;
                if (*Data != '=')
                {
                    nValue += DecodeTable[*Data++];
                    strDecode += nValue & 0x000000FF;
                    OutByte++;
                }
            }
            i += 4;
        }
        else // 回车换行,跳过
        {
            Data++;
            i++;
        }
    }
    return strDecode;
}
