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

#include <cstdio>
#include <cstdarg>

#include <chrono>
#include <mutex>

#include "../../include/quickgl/util/Log.hpp"

namespace qgl {
	Log::Log(int line, const char* fileName, const char* fmt, ...) {
		FILE* file = (FILE*)GetFile();
		
		fprintf(file, "log %s:%3.3d \tt:%.3fms  \tdt:%.4fms  \t", fileName, line,
				GetSecFromStart()*1000.0f,
				GetSecFromLast()*1000.0f);
		
		va_list args;
		va_start(args, fmt);
		vfprintf(file, fmt, args);
		va_end(args);
		
		fprintf(file, "\n");
		
		if(sync)
			fflush(file);
	}
	
	void Log::EmptyLine(int emptyLines) {
		FILE* file = (FILE*)GetFile();
		for(int i=0; i<emptyLines; ++i)
			fprintf(file, "\n");
		if(sync)
			fflush(file);
	}
	
	bool Log::sync = true;
	
	void* Log::GetFile() {
		static FILE* file = nullptr;
		if(file == nullptr)
			file = fopen(LOG_FILE, "w");
		return file;
	}
	
	static std::mutex LOG_MUTEX_GLOBAL_OBJECT;
	
	void Log::LockStart() {
		LOG_MUTEX_GLOBAL_OBJECT.lock();
	}
	
	void Log::LockEnd() {
		LOG_MUTEX_GLOBAL_OBJECT.unlock();
	}
	
	double Log::GetSecFromStart() {
		const static auto start = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		double sec = std::chrono::duration_cast<
			std::chrono::duration<double>>(now-start).count();
		return sec;
	}
	
	double Log::GetSecFromLast() {
		static auto last = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		double sec = std::chrono::duration_cast<
			std::chrono::duration<double>>(now-last).count();
		last = now;
		return sec;
	}
}

