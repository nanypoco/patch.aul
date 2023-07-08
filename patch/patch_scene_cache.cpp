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

#include "patch_scene_cache.hpp"

#ifdef PATCH_SWITCH_SCENE_CACHE


static int get_state_idx(bool is_saving, bool fast_process) {
	if (is_saving) return 1;
	if (fast_process) return 2;
	return 0;
};

static AviUtl::SharedMemoryInfo* smem_info[50][3] = {};

namespace patch {

	void* __cdecl scene_cache_t::get_scene_image_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int scene_idx, int frame, int subframe, int* w, int* h) {
		int* SceneDisplaying = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SceneDisplaying);
		if (*SceneDisplaying != 0) {
			return get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		}
		
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);

		auto state_idx = get_state_idx(*(int*)(GLOBAL::exedit_base + OFS::ExEdit::is_saving), *(int*)(GLOBAL::exedit_base + OFS::ExEdit::fast_process));
		auto& smem = smem_info[scene_idx][state_idx];
		
		void* smem_ptr = a_exfunc->get_shared_mem(std::bit_cast<int32_t>(&smem), (frame << 7) | subframe, smem);
		if (smem_ptr != nullptr) {
			reinterpret_cast<void(__cdecl*)(int, int*, int*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::get_scene_size)(scene_idx, w, h, efpip);
			return smem_ptr;
		}

		auto t0 = std::chrono::system_clock::now();
		void* img_ptr = get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		if (img_ptr == nullptr) return nullptr;

		if (std::chrono::milliseconds{ threshold_time_ms } < std::chrono::system_clock::now() - t0) {
			int yc_size, flag;
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				yc_size = 8;
				flag = 0x13000003;
			} else {
				yc_size = 6;
				flag = 0x13000002;
			}
			void* smem_ptr = a_exfunc->create_shared_mem(std::bit_cast<int32_t>(&smem), (frame << 7) | subframe, *h * efpip->scene_line * yc_size, &smem);
			if (smem_ptr == nullptr) return img_ptr;
			memcpy(smem_ptr, img_ptr, *h * efpip->scene_line * yc_size);
		}
		return img_ptr;
	}

	void scene_cache_t::delete_scene_cache() {
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		for (int i = 1; i < 50; i++) {
			for (int state = 0; state < 2; state++) {
				auto& smem = smem_info[i][state];
				a_exfunc->delete_shared_mem(std::bit_cast<int32_t>(&smem), smem);
			}
		}
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SCENE_CACHE
