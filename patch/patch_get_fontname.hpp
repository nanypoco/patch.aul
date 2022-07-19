#pragma once
#include "macro.h"
#include <Windows.h>
#ifdef PATCH_SWITCH_GET_FONTNAME

#include "util_magic.hpp"
#include "global.hpp"
#include "config_rw.hpp"
#include "cryptostring.hpp"

namespace patch {
        inline class get_fontname_t {
            inline static const char key[] = "get_fontname";
        public:
            bool enabled = true;
            bool enabled_i;

            static int CALLBACK enumfontfamproc_wrap(ENUMLOGFONT* param_1, NEWTEXTMETRIC* param_2, DWORD param_3, HDC param_4);
            inline static auto enumfontfamproc_wrap_ptr = &enumfontfamproc_wrap;
            static int WINAPI EnumFontFamiliesA(HDC hdc, LPLOGFONTA lplf, FONTENUMPROCA enumfontfamproc, LPARAM lpfam);

            void init() {
                enabled_i = enabled;
                if (!enabled_i)return;
                OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x8c600, 4).store_i32(0,enumfontfamproc_wrap_ptr);
                ExchangeFunction(GLOBAL::exedit_hmod, cstr_gdi32_dll.get(), cstr_EnumFontFamiliesA.get(), &EnumFontFamiliesA);
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
