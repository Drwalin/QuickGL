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

#include "../../include/quickgl/util/EventQueue.hpp"

#include <mutex>

namespace qgl {
	
	void EventQueue::PushEvent_(std::function<void()> event) {
		std::lock_guard lock(mutex);
		eventsQueue.push(event);
		counter++;
	}
	
	bool EventQueue::ExecuteOneEvent() {
		if(popedEventsForRunningThreads.empty()) {
			std::lock_guard lock(mutex);
			std::swap(eventsQueue, popedEventsForRunningThreads);
		}
		
		if(popedEventsForRunningThreads.empty() == false) {
			popedEventsForRunningThreads.front()();
			popedEventsForRunningThreads.pop();
			counter--;
			return true;
		}
		return false;
	}
	
	bool EventQueue::HasAnyEvents() {
		return counter != 0;
	}
}

