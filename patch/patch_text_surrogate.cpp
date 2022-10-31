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

#pragma once
#include "patch_text_surrogate.hpp"

#ifdef PATCH_SWITCH_TEXT_SURROGATE
namespace patch {

    int __stdcall text_surrogate_t::check_surrogate(USHORT currentChar, short* p) {
        if (IS_HIGH_SURROGATE(currentChar)) {
            if (IS_LOW_SURROGATE((USHORT)*p)) {
                surroFlag = true;
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
                ;

                if (GetCharacterPlacementW(hdc, wc, 3, 0, &gcp, GCP_GLYPHSHAPE) == 0 || gcp.lpGlyphs[0] == gcp.lpGlyphs[1]) {
                    //関数失敗 or フォントが文字に非対応(0x20と同じ文字が返ってくるらしい)
                    codePoint = 0;
                    return 1;
                    //surroFlag = false;
                    //return 0;
                }

                codePoint = gcp.lpGlyphs[1];

                return 1;
            }
        }
        surroFlag = false;
        return 0;
    }

    DWORD WINAPI text_surrogate_t::GetGlyphOutlineW_wrap(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, const MAT2* lpmat2) {
        if (surroFlag) {
            //初回のサイズ取得ではそのまま
            if (cjBuffer != 0) { surroFlag = false; }
            return GetGlyphOutlineW(hdc, codePoint, fuFormat | GGO_GLYPH_INDEX, lpgm, cjBuffer, pvBuffer, lpmat2);
        }
        return GetGlyphOutlineW(hdc, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, lpmat2);
    }
}
#endif
