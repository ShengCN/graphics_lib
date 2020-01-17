#include "asset_manager.h"

asset_manager::asset_manager() {
	if(cur_camera == nullptr) {
		cur_camera = std::make_shared<ppc>(w, h, 60.0f);
	}
}

