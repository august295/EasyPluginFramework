#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <codecvt>
#include <locale>
#include <string>

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

/**
 * @brief 编码转换
 * @param str          字符串(gbk)
 * @return std::string 字符串(utf8)
 */
inline std::string gbk_to_utf8(const std::string& str)
{
    // GBK locale name in windows
    const char* GBK_LOCALE_NAME = ".936";
    auto        convert         = std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>>(new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    auto        convert_utf8    = std::wstring_convert<std::codecvt_utf8<wchar_t>>();

    std::wstring tmp_wstr = convert.from_bytes(str);
    return convert_utf8.to_bytes(tmp_wstr);
}

/**
 * @brief 编码转换
 * @param str          字符串(utf8)
 * @return std::string 字符串(gbk)
 */
inline std::string utf8_to_gbk(const std::string& str)
{
    // GBK locale name in windows
    const char* GBK_LOCALE_NAME = ".936";
    auto        convert         = std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>>(new std::codecvt_byname<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
    auto        convert_utf8    = std::wstring_convert<std::codecvt_utf8<wchar_t>>();

    std::wstring tmp_wstr = convert_utf8.from_bytes(str);
    return convert.to_bytes(tmp_wstr);
}

#endif