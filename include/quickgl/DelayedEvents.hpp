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

#ifndef QUICKGL_DELEYED_EVENTS_HPP
#define QUICKGL_DELEYED_EVENTS_HPP

#include <chrono>
#include <functional>
#include <atomic>
#include <map>
#include <mutex>

namespace qgl {
	
	class DelayedEvents {
	public:
		
		DelayedEvents() = default;
		~DelayedEvents() = default;
		
		template<typename... Args, typename Fn>
		void PushEvent(int msDelay, Fn event, Args... args) {
			PushEvent_(msDelay, std::bind(event, args...));
		}
		
		void PushEvent_(int msDelay, std::function<void()> event);
		
		bool ExecuteOneEvent(); // returns true if anything was executed
		bool HasAnyEvents();
		
	private:
		
		// replace std::function & std::bind with something faster/smaller
		std::multimap<std::chrono::time_point<std::chrono::steady_clock>,
			std::function<void()>> events;
		
		// replace std::queue & std::mutex with something faster
		std::mutex mutex;
	};
}

#endif

