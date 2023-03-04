/*
 *  This file is part of QuickGL.
 *  Copyright (C) 2023 Marek Zalewski aka Drwalin
 *
 *  QuickGL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuickGL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <chrono>
#include <mutex>

#include "../include/quickgl/DelayedEvents.hpp"

namespace qgl {
	void DelayedEvents::PushEvent_(int msDelay, std::function<void()> event) {
		auto t = std::chrono::steady_clock::now()
			+ std::chrono::milliseconds(msDelay);
		std::lock_guard lock(mutex);
		events.insert({t, event});
	}

	bool DelayedEvents::ExecuteOneEvent() {
		std::function<void()> event;
		{
			std::lock_guard lock(mutex);
			auto it = events.begin();
			if(it != events.end()) {
				if(it->first < std::chrono::steady_clock::now()) {
					event = it->second;
					events.erase(it);
				} else {
					return false;
				}
			} else {
				return false;
			}
		}
		event();
		return true;
	}
	
	bool DelayedEvents::HasAnyEvents() {
		std::lock_guard lock(mutex);
		return !events.empty();
	}
}

