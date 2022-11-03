/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "patch_get_fontname.hpp"
//#include "gdiplus.h"
//#include "iostream"

#ifdef PATCH_SWITCH_GET_FONTNAME
namespace patch {

    //初回のコンボボックス生成
    int WINAPI get_fontname_t::EnumFontFamiliesA_wrap(HDC hdc, LPLOGFONTA lplf, FONTENUMPROCA enumfontfamproc, LPARAM lpfam) {
        LOGFONTW lf{};
        lf.lfPitchAndFamily = DEFAULT_PITCH;
        return EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)enumfontfamproc, lpfam, 0);
    }

    int CALLBACK get_fontname_t::enumfontfamproc_wrap(ENUMLOGFONTW* elf, ENUMTEXTMETRICW* metric, DWORD fonttype, HDC hdc) {
        LPCWSTR fontnameW;
        //char fontnameA[64];
        int c;
        tagSIZE size{};
        LONG& cxMax = load_i32<LONG&>(GLOBAL::exedit_base + 0x23638c);
        HWND& sFont = load_i32<HWND&>(GLOBAL::exedit_base + 0x23630c);

        if (*(WCHAR*)(elf->elfLogFont.lfFaceName) != L'@') {
            fontnameW = (LPCWSTR)(elf->elfLogFont.lfFaceName);
            //WideCharToMultiByte(CP_ACP, 0, fontnameW, -1, fontnameA, 32, NULL, NULL);

            c = lstrlenW(fontnameW);
            GetTextExtentPoint32W(hdc, fontnameW, c, &size);
            if (cxMax < size.cx) {
                cxMax = size.cx;
            }
            //SendMessageA(sFont, CB_ADDSTRING, 0, (LPARAM)fontnameA);
            SendMessageW(sFont, CB_ADDSTRING, 0, (LPARAM)fontnameW);
        }
        return 1;
    }

    //テキストオブジェクトからフォント名を読んでコンボボックス上で選択
    LRESULT WINAPI get_fontname_t::SendMessageA_wrap1(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
        if (Msg == CB_SELECTSTRING) {
            int index = SendMessageA(hWnd, CB_FINDSTRINGEXACT, -1, lParam);

            //見つからなければ元のフォント名を取得して再度検索
            if (index == CB_ERR) {
                WCHAR fontnameW[32];
                MultiByteToWideChar(CP_ACP, 0, (char*)lParam, -1, fontnameW, 32);

                HDC hdc = GetDC(0);
                LOGFONTW lf = {};
                wcsncpy_s(lf.lfFaceName, 32, fontnameW, 31);

                EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)effep1, (LPARAM)fontnameW, 0);
                return SendMessageW(hWnd, Msg, wParam, (LPARAM)fontnameW);
            }
            return SendMessage(hWnd, CB_SETCURSEL, index, NULL);
        }
        return SendMessageA(hWnd, Msg, wParam, lParam);
    }

    int CALLBACK get_fontname_t::effep1(ENUMLOGFONTW* elf, ENUMTEXTMETRICW* metric, DWORD fonttype, LPARAM lParam) {
        if (wcsncmp(elf->elfFullName, (LPCWSTR)lParam, LF_FULLFACESIZE) == 0) {
            lstrcpyW((LPWSTR)lParam, elf->elfLogFont.lfFaceName);
            return 0;
        }
        return 1;
    }

    //コンボボックスに変更があった時にフォント名をオブジェクトに書き込み
    LRESULT WINAPI get_fontname_t::SendMessageA_wrap2(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
        if (Msg == CB_GETLBTEXT) {
            char getName[64];
            LRESULT r = SendMessageA(hWnd, Msg, wParam, (LPARAM)getName);
            //32バイト以上なら別名を取得
            if (strnlen_s(getName, 64) > 31) {
                WCHAR fontnameW[32];
                MultiByteToWideChar(CP_ACP, 0, getName, -1, fontnameW, 32);

                HDC hdc = GetDC(0);
                LOGFONTW lf{};
                wcsncpy_s(lf.lfFaceName, 32, fontnameW, 31);

                //fullnameをfontnameWに入れる 1が返ると失敗
                EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)effep2, (LPARAM)&fontnameW, 0);
                /*
                if (EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)effep2, (LPARAM)&fontnameW, 0) == 1) {
                    return r;
                }
                */
                WideCharToMultiByte(CP_ACP, 0, fontnameW, -1, (char*)lParam, 32, NULL, NULL);
                return r;
            }
        }
        return SendMessageA(hWnd, Msg, wParam, lParam);
    }

    int CALLBACK get_fontname_t::effep2(ENUMLOGFONTW* elf, ENUMTEXTMETRICW* metric, DWORD fonttype, LPARAM lParam) {
        /* GDI+で英語フォント名取得(未完)
        static Gdiplus::GdiplusStartupInput GdiplusStartupInput;
        static ULONG_PTR GdiplusToken;
        if (!GdiplusToken) {
            Gdiplus::GdiplusStartup(&GdiplusToken, &GdiplusStartupInput, nullptr);
        }
        Gdiplus::FontFamily ff(elf->elfLogFont.lfFaceName);
        WCHAR e[64] = {};
        ff.GetFamilyName(e, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
        //etc
        //Gdiplus::GdiplusShutdown(GdiplusToken);
        */
        if (wcsncmp(elf->elfLogFont.lfFaceName, (LPCWSTR)lParam, LF_FACESIZE) == 0) {
            lstrcpyW((LPWSTR)lParam, elf->elfFullName);
            return 0;
        }
        return 1;
    }

}
#endif
