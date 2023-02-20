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

#ifndef QUICKGL_SCHEDULER_HPP
#define QUICKGL_SCHEDULER_HPP

#include <functional>
#include <mutex>
#include <queue>

namespace qgl {
	class Scheduler {
	public:
		
		Scheduler();
		~Scheduler();
		
		void RunAsync();
		
		void ScheduleTask(std::function<void()> task);
		void SchedulePriorityTask(std::function<void()> task);
		
		template<typename... Args>
		void ScheduleTask(std::function<void(Args...)> task, Args... args);
		
		template<typename... Args>
		void SchedulePriorityTask(std::function<void(Args...)> task, Args... args);
		
	private:
		
		std::mutex lockQueue;
		std::queue<std::function<void()>> taskQueue;
		
		std::mutex lockPriorityQueue;
		std::queue<std::function<void()>> priorityTasksQueue;
	};
	
	
	
	template<typename... Args>
	void Scheduler::ScheduleTask(std::function<void(Args...)> task, Args... args) {
		ScheduleTask(std::bind(task, args...));
	}
	
	template<typename... Args>
	void Scheduler::SchedulePriorityTask(std::function<void(Args...)> task, Args... args) {
		SchedulePriorityTask(std::bind(task, args...));
	}
}

#endif

