#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

inline std::string convertCodePage(const std::string& input, UINT sourceCodePage, UINT targetCodePage)
{
    if (input.empty())
    {
        return {};
    }

    const int wideLength = ::MultiByteToWideChar(sourceCodePage, 0, input.c_str(), -1, nullptr, 0);
    if (wideLength <= 0)
    {
        return {};
    }

    std::vector<wchar_t> wideBuffer(static_cast<size_t>(wideLength));
    if (::MultiByteToWideChar(sourceCodePage, 0, input.c_str(), -1, wideBuffer.data(), wideLength) <= 0)
    {
        return {};
    }

    const int outputLength = ::WideCharToMultiByte(targetCodePage, 0, wideBuffer.data(), -1, nullptr, 0, nullptr, nullptr);
    if (outputLength <= 0)
    {
        return {};
    }

    std::vector<char> outputBuffer(static_cast<size_t>(outputLength));
    if (::WideCharToMultiByte(targetCodePage, 0, wideBuffer.data(), -1, outputBuffer.data(), outputLength, nullptr, nullptr) <= 0)
    {
        return {};
    }

    return std::string(outputBuffer.data());
}

inline std::string gbkToUtf8(const std::string& gbkText)
{
    return convertCodePage(gbkText, CP_ACP, CP_UTF8);
}

inline std::string utf8ToGbk(const std::string& utf8Text)
{
    return convertCodePage(utf8Text, CP_UTF8, CP_ACP);
}
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #include <iconv.h>

inline std::string convertEncoding(const std::string& input, const char* fromEncoding, const char* toEncoding)
{
    if (input.empty())
    {
        return {};
    }

    iconv_t converter = iconv_open(toEncoding, fromEncoding);
    if (converter == reinterpret_cast<iconv_t>(-1))
    {
        return {};
    }

    std::vector<char> outputBuffer(input.size() * 4 + 1, '\0');
    char*             inputBuffer     = const_cast<char*>(input.data());
    size_t            inputBytesLeft  = input.size();
    char*             outputPointer   = outputBuffer.data();
    size_t            outputBytesLeft = outputBuffer.size() - 1;

    const size_t result = iconv(converter, &inputBuffer, &inputBytesLeft, &outputPointer, &outputBytesLeft);
    iconv_close(converter);
    if (result == static_cast<size_t>(-1))
    {
        return {};
    }

    return std::string(outputBuffer.data(), static_cast<size_t>(outputPointer - outputBuffer.data()));
}

inline std::string gbkToUtf8(const std::string& gbkText)
{
    return convertEncoding(gbkText, "GBK", "UTF-8");
}

inline std::string utf8ToGbk(const std::string& utf8Text)
{
    return convertEncoding(utf8Text, "UTF-8", "GBK");
}
#endif

/**
 * @brief 移除字符串尾部的回车和换行符。
 * @param text 原始字符串。
 * @return std::string 处理后的字符串。
 */
inline std::string removeCrlf(const std::string& text)
{
    size_t end = text.size();
    while (end > 0 && (text[end - 1] == '\r' || text[end - 1] == '\n'))
    {
        --end;
    }
    return text.substr(0, end);
}

inline std::string gbk_to_utf8(const std::string& gbkText)
{
    return gbkToUtf8(gbkText);
}

inline std::string utf8_to_gbk(const std::string& utf8Text)
{
    return utf8ToGbk(utf8Text);
}

inline std::string RemoveCRLF(const std::string& text)
{
    return removeCrlf(text);
}

#endif
