#include <hpdf.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ft2build.h>
#include <vector>
#include <set>
#include FT_FREETYPE_H
#include <algorithm>
#include <numeric> // std::accumulate
using namespace std;


std::string unicode_to_utf8(FT_ULong codepoint) {
    std::string utf8;
    
    if (codepoint <= 0x7F) {
        utf8 += static_cast<unsigned char>(codepoint);  // 单字节编码
    }
    else if (codepoint <= 0x7FF) {
        utf8 += static_cast<unsigned char>((codepoint >> 6) | 0xC0);  // 双字节编码
        utf8 += static_cast<unsigned char>((codepoint & 0x3F) | 0x80);
    }
    else if (codepoint <= 0xFFFF) {
        utf8 += static_cast<unsigned char>((codepoint >> 12) | 0xE0);  // 三字节编码
        utf8 += static_cast<unsigned char>((codepoint >> 6 & 0x3F) | 0x80);
        utf8 += static_cast<unsigned char>((codepoint & 0x3F) | 0x80);
    }
    else {
        utf8 += static_cast<unsigned char>((codepoint >> 18) | 0xF0);  // 四字节编码
        utf8 += static_cast<unsigned char>((codepoint >> 12 & 0x3F) | 0x80);
        utf8 += static_cast<unsigned char>((codepoint >> 6 & 0x3F) | 0x80);
        utf8 += static_cast<unsigned char>((codepoint & 0x3F) | 0x80);
    }
    
    return utf8;
}


void generatePDF(const string& fontFile, const string& outputFile) {
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        cout << "init library error" << endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(library, fontFile.c_str(), 0, &face)) {
        cout << "new face error" << endl;
        return;
    }

    FT_Select_Charmap(face, ft_encoding_unicode);
    FT_UInt flag;
    FT_ULong char_code = FT_Get_First_Char(face, &flag);

    set<string> words;
    while (flag != 0) {
        auto word = unicode_to_utf8(char_code);
        words.insert(word);
        char_code = FT_Get_Next_Char(face, char_code, &flag);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);  // 创建一个新的PDF文档
    if (!pdf) {
        std::cerr << "Failed to create PDF object!" << std::endl;
        return;
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);  // 压缩所有内容
    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, "UTF-8");

    HPDF_Font font = HPDF_GetFont(pdf, HPDF_LoadTTFontFromFile(pdf, fontFile.c_str(), HPDF_TRUE), "UTF-8");
    if (!font) {
        std::cerr << "Failed to load font!" << std::endl;
        HPDF_Free(pdf);
        return;
    }


    // 将 words 转换为 vector<string> 以便进一步处理
    vector<string> char_list(words.begin(), words.end());

    // 设置每页的字符数，每行字符数
    int page_words = 300, line_words = 15;
    int page_lines = page_words / line_words;


    // 分页输出
    for (size_t page_start_idx = 0; page_start_idx < char_list.size(); page_start_idx += page_words) {
        size_t page_end_idx = min(page_start_idx + page_words, char_list.size());
        vector<string> page_words(char_list.begin() + page_start_idx, char_list.begin() + page_end_idx);
        
        // 创建新页面
        HPDF_Page page = HPDF_AddPage(pdf);

        HPDF_Page_BeginText(page);
        HPDF_REAL page_height = HPDF_Page_GetHeight(page);
        // 页面宽度计算，考虑字体大小
        HPDF_REAL page_width = HPDF_Page_GetWidth(page);
        HPDF_REAL font_size = page_height / (page_lines*1.5); // 300 / 15 = 20 行数
        HPDF_REAL ypos = page_height - font_size;  // Y 轴起始位置
        HPDF_Page_SetFontAndSize(page, font, font_size); // 设置字体


        // 每行输出 15 个字符
        for (size_t row_start_idx = 0; row_start_idx < page_words.size(); row_start_idx += line_words) {
            size_t row_end_idx = min(row_start_idx + line_words, page_words.size());

            // 将这一行的字符连接成一个字符串，确保每行 15 个字符
            auto text = accumulate(page_words.begin() + row_start_idx, page_words.begin() + row_end_idx, string{},
                [](const string &a, const string &b) {
                    return a + b;  // 连接每个字符之间的空格
                });

            // 输出当前行文本
            HPDF_Page_TextOut(page, 50, ypos, text.c_str());
            ypos -= font_size * 1.5f;  // 行间距


        }

        HPDF_Page_EndText(page);

        // 如果已经到达文件末尾，退出循环
        if (page_end_idx >= char_list.size()) break;
    }
    HPDF_SaveToFile(pdf, outputFile.c_str());
    HPDF_Free(pdf);
}


int main() {
    string fontFile = "../simkai.ttf";
    string outputFile = "output.pdf";
    try{
        generatePDF(fontFile, outputFile);
    }catch(std::exception& e){
        std::cout<<" genneratePDF error err is "<<e.what()<<std::endl;
    }
    
    return 0;
}