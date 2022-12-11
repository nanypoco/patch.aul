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
#include "macro.h"
#ifdef PATCH_SWITCH_GET_FONTNAME

#include <Windows.h>
#include "util.hpp"
#include "util_magic.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
    // 長いフォント名を正常に展開できないのをどうにかする
    inline class get_fontname_t {
        inline static const char key[] = "get_fontname";

        bool enabled = true;
        bool enabled_i;

        inline static bool surroFlag;
        inline static USHORT codePoint;
	    
        static int CALLBACK enumfontfamproc_wrap(ENUMLOGFONTW* param_1, ENUMTEXTMETRICW* param_2, DWORD param_3, HDC param_4);

        static int WINAPI EnumFontFamiliesA_wrap(HDC hdc, LPLOGFONTA lplf, FONTENUMPROCA enumfontfamproc, LPARAM lpfam);
        inline static auto EnumFontFamiliesA_wrap_ptr = &EnumFontFamiliesA_wrap;

        static LRESULT WINAPI SendMessageA_wrap1(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
        inline static auto SendMessageA_wrap1_ptr = &SendMessageA_wrap1;

        static LRESULT WINAPI SendMessageA_wrap2(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
        inline static auto SendMessageA_wrap2_ptr = &SendMessageA_wrap2;

        static int CALLBACK effep1(ENUMLOGFONTW* elf, ENUMTEXTMETRICW* metric, DWORD fonttype, LPARAM lParam);
        static int CALLBACK effep2(ENUMLOGFONTW* elf, ENUMTEXTMETRICW* metric, DWORD fonttype, LPARAM lParam);

    public:
        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;
            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8c600, 4).store_i32(0, &enumfontfamproc_wrap);
            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8c609, 4).store_i32(0, &EnumFontFamiliesA_wrap_ptr);
            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8c320, 4).store_i32(0, &SendMessageA_wrap1_ptr);
            //OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8bd07, 4).store_i32(0, &SendMessageA_wrap1_ptr);
            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8b958, 4).store_i32(0, &SendMessageA_wrap2_ptr);
        }
        void switching(bool flag) { enabled = flag; }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled_i; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
                });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }
    } get_fontname;
} // namespace patch
#endif // ifdef PATCH_SWITCH_GET_FONTNAME
