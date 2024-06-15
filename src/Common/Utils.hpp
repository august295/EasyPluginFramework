#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>

#if (__cplusplus < 201103L) || (__cplusplus >= 201703L)
    #if defined(_WIN32) || defined(_WIN64)
        #include <windows.h>

static std::string gbk_to_utf8(const std::string& str_gbk)
{
    int      utf16Length = ::MultiByteToWideChar(CP_ACP, 0, str_gbk.c_str(), -1, NULL, 0);
    wchar_t* utf16Buffer = new wchar_t[utf16Length];
    ::MultiByteToWideChar(CP_ACP, 0, str_gbk.c_str(), -1, utf16Buffer, utf16Length);

    int   utf8BufferSize = ::WideCharToMultiByte(CP_UTF8, 0, utf16Buffer, -1, NULL, 0, NULL, NULL);
    char* utf8Buffer     = new char[utf8BufferSize];
    ::WideCharToMultiByte(CP_UTF8, 0, utf16Buffer, -1, utf8Buffer, utf8BufferSize, NULL, NULL);

    std::string utf8String(utf8Buffer);

    delete[] utf16Buffer;
    delete[] utf8Buffer;

    return utf8String;
}

static std::string utf8_to_gbk(const std::string& str_utf8)
{
    int      utf16Length = ::MultiByteToWideChar(CP_UTF8, 0, str_utf8.c_str(), -1, NULL, 0);
    wchar_t* utf16Buffer = new wchar_t[utf16Length];
    ::MultiByteToWideChar(CP_UTF8, 0, str_utf8.c_str(), -1, utf16Buffer, utf16Length);

    int   gbkBufferSize = ::WideCharToMultiByte(CP_ACP, 0, utf16Buffer, -1, NULL, 0, NULL, NULL);
    char* gbkBuffer     = new char[gbkBufferSize];
    ::WideCharToMultiByte(CP_ACP, 0, utf16Buffer, -1, gbkBuffer, gbkBufferSize, NULL, NULL);

    std::string gbkString(gbkBuffer);

    delete[] utf16Buffer;
    delete[] gbkBuffer;

    return gbkString;
}
    #elif defined(linux) || defined(__linux) || defined(__linux__)
        #include <iconv.h>
        #include <iostream>
std::string gbk_to_utf8(const std::string& str_gbk)
{
    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        return "";
    }

    char*  inbuf       = const_cast<char*>(str_gbk.c_str());
    size_t inbytesleft = str_gbk.length();

    size_t outbytesleft = inbytesleft * 2; // 最大可能的输出字节数
    char*  outbuf       = new char[outbytesleft];
    char*  outptr       = outbuf;

    size_t result = iconv(cd, &inbuf, &inbytesleft, &outptr, &outbytesleft);
    if (result == (size_t)-1) {
        delete[] outbuf;
        iconv_close(cd);
        return "";
    }

    iconv_close(cd);

    std::string utf8String(outbuf, outptr - outbuf);
    delete[] outbuf;

    return utf8String;
}

std::string utf8_to_gbk(const std::string& str_utf8)
{
    iconv_t cd = iconv_open("GBK", "UTF-8");
    if (cd == (iconv_t)-1) {
        return "";
    }

    char*  inbuf       = const_cast<char*>(str_utf8.c_str());
    size_t inbytesleft = str_utf8.length();

    size_t outbytesleft = inbytesleft;
    char*  outbuf       = new char[outbytesleft];
    char*  outptr       = outbuf;

    size_t result = iconv(cd, &inbuf, &inbytesleft, &outptr, &outbytesleft);
    if (result == (size_t)-1) {
        delete[] outbuf;
        iconv_close(cd);
        return "";
    }

    iconv_close(cd);

    std::string gbkString(outbuf, outptr - outbuf);
    delete[] outbuf;

    return gbkString;
}
    #endif
#elif (__cplusplus >= 201103L) && (__cplusplus < 201703L)
    #include <codecvt>
    #include <locale>
static std::string gbk_to_utf8(const std::string& str_gbk)
{
    const char*                                                         GBK_LOCALE_NAME = ".936";
    std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> convert(new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    std::wstring_convert<std::codecvt_utf8<wchar_t>>                    convert_utf8;

    std::wstring tmp_wstr = convert.from_bytes(str_gbk);
    return convert_utf8.to_bytes(tmp_wstr);
}

static std::string utf8_to_gbk(const std::string& str_utf8)
{
    const char*                                                         GBK_LOCALE_NAME = ".936";
    std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> convert(new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    std::wstring_convert<std::codecvt_utf8<wchar_t>>                    convert_utf8;

    std::wstring tmp_wstr = convert_utf8.from_bytes(str_utf8);
    return convert.to_bytes(tmp_wstr);
}
#endif

/**
 * @brief 过滤换行符
 * @param str          原生字符串
 * @return std::string 无换行符字符串
 */
inline std::string RemoveCRLF(const std::string& str)
{
    auto i = str.size();
    for (; i >= 0; --i) {
        if (str[i - 1] != '\r' && str[i - 1] != '\n') {
            break;
        }
    }
    return str.substr(0, i);
}

#endif