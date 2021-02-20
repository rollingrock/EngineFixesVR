#pragma once

#include "skse64/GameEvents.h"

namespace patches {
	class CellEventHandler : public BSTEventSink<TESCellAttachDetachEvent> {
	public:
		virtual EventResult ReceiveEvent(TESCellAttachDetachEvent* evn, EventDispatcher<TESCellAttachDetachEvent>* dis) {

			//_MESSAGE("attached is %d", evn->attached);
			if (evn->attached == 0) {
				resetMap();
			}
			//_MESSAGE("cell event %d", cur);
			return kEvent_Continue;
		}

		void resetMap();
	};
}
