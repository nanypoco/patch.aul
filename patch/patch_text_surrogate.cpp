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

#include "patch_text_surrogate.hpp"
#include "patch.hpp"
#include "iostream"

#ifdef PATCH_SWITCH_TEXT_SURROGATE
namespace patch {

#if 0
    int __stdcall text_surrogate_t::check_surrogate(USHORT currentChar, short* p) {
        if (IS_SURROGATE_PAIR(currentChar,(USHORT)*p) || (nChar & 0xFFF0) == 0xFE00) {
            isReplacedCodePoint = true;
            WCHAR wc[3];
            wc[0] = 0x20;
            wc[1] = currentChar;
            wc[2] = *p;

            HDC hdc = load_i32<HDC>(GLOBAL::exedit_base + 0x19660c);

            WCHAR lpgBuf[3] = L"";
            GCP_RESULTSW gcp = {};
            gcp.lStructSize = sizeof(gcp);
            gcp.lpGlyphs = lpgBuf;
            gcp.nGlyphs = 3;

            if (GetCharacterPlacementW(hdc, wc, 3, 0, &gcp, GCP_GLYPHSHAPE) == 0 || gcp.lpGlyphs[0] == gcp.lpGlyphs[1]) {
                //関数失敗 or フォントが文字に非対応(0x20と同じ文字が返ってくるらしい)
                codePoint = 0;
                return 1;
                //isReplacedCodePoint = false;
                //return 0;
            }

            codePoint = gcp.lpGlyphs[1];

            return 1;
        }
        else {
            HDC hdc = load_i32<HDC>(GLOBAL::exedit_base + 0x19660c);

            WCHAR lpgBuf[4] = L"";
            GCP_RESULTSW gcp = {};
            gcp.lStructSize = sizeof(gcp);
            gcp.lpGlyphs = lpgBuf;
            gcp.nGlyphs = 3;

            WCHAR wc[4]={0x20,currentChar,*p};
            wc[0] = 0x20;
            wc[1] = currentChar;
            wc[2] = *p;

            int moveAmount = 0;
            USHORT nChar = (USHORT)*p;
            USHORT nnChar = (USHORT)*(p+1);
            if (IS_SURROGATE_PAIR(nChar,nnChar)) {
                //U+E0100～U+E01EF
                //C = (H - 0xD800) * 0x400 + (L - 0xDC00) + 0x10000;
                int sc = 0x10000 + (nChar - 0xD800) * 0x400 + (nnChar - 0xDC00);
                if (0xE0100 <= sc && sc < 0xE01F0) {
                    wc[3] = nnChar;
                    gcp.nGlyphs = 4;
                    isReplacedCodePoint = true;
                    moveAmount = 2;
                }
                else { isReplacedCodePoint = false; return 0; }
            }
            else if ((nChar & 0xFFF0) == 0xFE00) {
                isReplacedCodePoint = true;
                moveAmount = 1;
            }
            else { isReplacedCodePoint = false; return 0; }

            //std::cout << "mA=" << moveAmount << std::endl;

            //std::cout << gcp.nGlyphs << std::endl;
            GetCharacterPlacementW(hdc, wc, gcp.nGlyphs, 0, &gcp, GCP_GLYPHSHAPE);
            //std::cout << gcp.nGlyphs << std::endl;
            /*for (int i = 0; i < gcp.nGlyphs; i++) {
                std::cout << i << "=" << (DWORD)gcp.lpGlyphs[i] << std::endl;
            }*/
            codePoint = gcp.lpGlyphs[1];
            return moveAmount*2;
            
        }
        isReplacedCodePoint = false;
        return 0;
    }
#endif

    int __stdcall text_surrogate_t::check_surrogate(WCHAR currentChar, WCHAR* p) {
        if (*p == 0) { isReplacedCodePoint = false; return 0; }
        WCHAR nChar = *p;
        WCHAR nnChar = *(p+1);
        //サロゲートペア or SVS 標準異体字シーケンス U+FE00-U+FE0F or モンゴル異体字
        if (IS_SURROGATE_PAIR(currentChar, nChar) || (nChar & 0xFFF0) == 0xFE00 || (0x180B <= nChar && nChar <= 0x180D)) {

            isReplacedCodePoint = true;
            WCHAR wc[3] = { 0x20, currentChar, nChar };

            HDC hdc = load_i32<HDC>(GLOBAL::exedit_base + 0x19660c);

            WCHAR lpgBuf[3] = L"";
            GCP_RESULTSW gcp = {};
            gcp.lStructSize = sizeof(gcp);
            gcp.lpGlyphs = lpgBuf;
            gcp.nGlyphs = 3;

            if (GetCharacterPlacementW(hdc, wc, gcp.nGlyphs, 0, &gcp, GCP_GLYPHSHAPE) == 0 || gcp.lpGlyphs[0] == gcp.lpGlyphs[1]) {
                //関数失敗 or フォントが文字に非対応(0x20と同じ文字が返ってくるらしい)
                codePoint = 0;
            }
            else { codePoint = gcp.lpGlyphs[1]; }
            return 1;
        }
        //IVS 漢字異体字シーケンス U+E0100-U+E01EF
        //3文字分変換を試みる
        else if (nnChar != 0 && IS_SURROGATE_PAIR(nChar, nnChar)) {
            int sc = 0x10000 + (nChar - 0xD800) * 0x400 + (nnChar - 0xDC00);
            if (0xE0100 <= sc && sc < 0xE01F0) {

                isReplacedCodePoint = true;
                WCHAR wc[4] = { 0x20, currentChar, nChar, nnChar };

                HDC hdc = load_i32<HDC>(GLOBAL::exedit_base + 0x19660c);

                WCHAR lpgBuf[4] = L"";
                GCP_RESULTSW gcp = {};
                gcp.lStructSize = sizeof(gcp);
                gcp.lpGlyphs = lpgBuf;
                gcp.nGlyphs = 4;

                if (GetCharacterPlacementW(hdc, wc, gcp.nGlyphs, 0, &gcp, GCP_GLYPHSHAPE) == 0 || gcp.lpGlyphs[0] == gcp.lpGlyphs[1]) {
                    //関数失敗 or フォントが文字に非対応(0x20と同じ文字が返ってくるらしい)
                    codePoint = 0;
                }
                else { codePoint = gcp.lpGlyphs[1]; }
                return 2;
            }
        }
        isReplacedCodePoint = false;
        return 0;
    }

    DWORD WINAPI text_surrogate_t::GetGlyphOutlineW_wrap(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, const MAT2* lpmat2) {

        DWORD (WINAPI *GGOW)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, const MAT2*);
        if (patch::fast::text.is_enabled_i()) {
            GGOW = patch::fast::text.GetGlyphOutlineW;
        }
        else {
            GGOW = ::GetGlyphOutlineW;
        }

        if (isReplacedCodePoint) {
            //初回のサイズ取得ではそのまま
            if (cjBuffer != 0) { isReplacedCodePoint = false; }
            
            return GGOW(hdc, codePoint, fuFormat | GGO_GLYPH_INDEX, lpgm, cjBuffer, pvBuffer, lpmat2);
        }
        return GGOW(hdc, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, lpmat2);
    }

}
#endif
