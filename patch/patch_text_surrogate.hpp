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
#ifdef PATCH_SWITCH_TEXT_SURROGATE

#include <Windows.h>
#include "util.hpp"
#include "util_magic.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
    inline class text_surrogate_t {
        inline static const char key[] = "text_surrogate";

        bool enabled = true;
        bool enabled_i;

        inline static bool surroFlag;
        inline static USHORT codePoint;

    public:
        static int __stdcall check_surrogate(USHORT currentChar, short* pNextChar);

        static DWORD WINAPI GetGlyphOutlineW_wrap(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, const MAT2* lpmat2);
        inline static auto GetGlyphOutlineW_wrap_ptr = &GetGlyphOutlineW_wrap;

        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;

            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x5057d, 6);
                h.store_i16(0, '\x90\xe9');

                auto& cursor = GLOBAL::executable_memory_cursor;
                h.replaceNearJmp(2, cursor);

                static const char code_put[] =
                    "\x0f\x84XXXX"                  // JZ exedit_base + 0x50583
                    "\x56"                          // PUSH ESI
                    "\x50"                          // PUSH EAX
                    "\xe8XXXX"                      // CALL check_surrogate
                    "\x85\xc0"                      // TEST EAX,EAX
                    "\x66\x8b\x46\xfe"              // MOV AX,DWORD PTR [ESI - 4]
                    "\x74\x0a"                      // JZ is_not_surrogate
                    "\x83\xc6\x02"                  // ADD ESI,0x2
                    "\x89\xB4\x24\x84\x01\x00\x00"  // MOV DWORD PTR [ESP+0x184],ESI
                    // is_not_surrogate:
                    "\xe9XXXX"                      // JMP exedit_base + 0x50ba4
                    ;

                memcpy(cursor, code_put, sizeof(code_put) - 1);
                store_i32(cursor + 2, GLOBAL::exedit_base + 0x50583 - (uint32_t)(cursor + 6));
                ReplaceNearJmp((i32)cursor + 9, &check_surrogate);
                cursor += sizeof(code_put) - 1;
                store_i32(cursor - 4, GLOBAL::exedit_base + 0x50ba4 - (uint32_t)cursor);

                OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x50bfa, 4).store_i32(0, &GetGlyphOutlineW_wrap_ptr);
            }

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
    } text_surrogate;
} // namespace patch
#endif // ifdef PATCH_SWITCH_TEXT_SURROGATE
