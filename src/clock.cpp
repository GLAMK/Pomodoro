#define OLC_SOUNDWAVE
#include "include/olcSoundWaveEngine.h"

#define OLC_PGE_APPLICATION 
#include "include/olcPixelGameEngine.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <ctime>

#include <Windows.h>
#include "resource.h"
#include "include/csv.hpp"

#define M_PI 3.141592653589793238462643

class Resource {
public:
	struct Parameters {
		std::size_t size_bytes = 0;
		void* ptr = nullptr;
	};
private:
	HRSRC hResource = nullptr;
	HGLOBAL hMemory = nullptr;

	Parameters p;

public:

	HMODULE GCM()
	{
		HMODULE hModule = NULL;
		GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			nullptr,
			&hModule);

		return hModule;
	}

	Resource(int resource_id, int resource_class) {
		hResource = FindResource(GCM(), MAKEINTRESOURCE(resource_id), MAKEINTRESOURCE(resource_class));
		hMemory = LoadResource(GCM(), hResource);

		p.size_bytes = SizeofResource(GCM(), hResource);
		p.ptr = LockResource(hMemory);
	}

	auto& GetResource() const {
		return p;
	}
};

auto& GetFile(int NAME, int CLASS) {
	Resource very_important(NAME, CLASS);
	return very_important.GetResource();
}

struct Time
{
	int seconds = 0;
	int minutes = 0;
	int hours   = 0;
	friend std::ostream& operator << (std::ostream& os, const Time& tm);
	int GMT_Time = -3;

	int totalTime = 0;

private:
	void CalcTotalTime()
	{
		totalTime = hours * 3600 + minutes * 60 + seconds;
	}
	void SetTime()
	{
		seconds = totalTime % 60;
		minutes = (totalTime / 60) % 60;
		hours = ((totalTime / 3600) % 24);
	}
public:
	Time() = default;

	Time(int s)
	{
		totalTime = s;
		seconds = s % 60;
		minutes = (s / 60) % 60;
		hours = ((s / 3600) % 24);
	}

	Time(int s, int m, int h)
	{
		totalTime = h * 3600 + m * 60 + s;
		seconds = s;
		minutes = m;
		hours = h;
	}

	Time(const Time& t)
	{
		totalTime = t.totalTime;
		seconds = t.seconds;
		minutes = t.minutes;
		hours = t.hours;
	}

	inline Time operator - (const Time& other) const
	{
		return Time(totalTime - other.totalTime);
	}

	inline Time operator + (const Time& other) const
	{
		return Time(totalTime + other.totalTime);
	}

	inline void operator += (const Time& t) 
	{
		totalTime += t.totalTime;
		this->SetTime();
	}

	inline void operator -= (const Time& t) 
	{
		totalTime -= t.totalTime;
		this->SetTime();
	}

	inline bool operator > (const Time& other) const
	{
		return this->totalTime > other.totalTime;
	}

	inline bool operator < (const Time& other) const
	{
		return this->totalTime < other.totalTime;
	}

	inline bool operator == (const Time& other) const
	{
		return this->totalTime == other.totalTime;
	}

	inline bool operator != (const Time& other) const
	{
		return this->totalTime != other.totalTime;
	}

	inline void operator = (const Time& t) 
	{
		totalTime = t.totalTime;
		SetTime();
	}

	void Parser(std::string& sTime)
	{
		this->hours   = atoi(sTime.substr(0, 2).c_str());
		this->minutes = atoi(sTime.substr(3, 2).c_str());
		this->seconds = atoi(sTime.substr(6, 2).c_str());
		CalcTotalTime();
	}

	void SetTime(int s)
	{
		totalTime = s;
		seconds = s % 60;
		minutes = (s / 60) % 60;
		hours = ((s / 3600) % 24);
	}

	std::string ToString(bool GMT) const
	{
		std::string second  =  (seconds < 10) ? ("0" + std::to_string(seconds)) : std::to_string(seconds);
		std::string minute =   (minutes < 10) ? ("0" + std::to_string(minutes) + ":") : std::to_string(minutes) + ":";
		
		int nHours = hours;
		if(GMT) nHours += GMT_Time;
		if(nHours < 0) nHours += 24;
		std::string hour   =   (nHours < 10)   ? ("0" + std::to_string(nHours)   + ":") : std::to_string(nHours)   + ":";
		return hour + minute + second;
	}

	int GetHoursGMT()
	{
		int nHours = hours;
		nHours += GMT_Time;
		if(nHours < 0) nHours += 24;
		return nHours;
	}
};

inline std::ostream& operator << (std::ostream& out, const Time& tm)
{
	std::string second  =  (tm.seconds < 10) ? ("0" + std::to_string(tm.seconds))       : std::to_string(tm.seconds);
	std::string minutes =  (tm.minutes < 10) ? ("0" + std::to_string(tm.minutes) + ":") : std::to_string(tm.minutes) + ":";
	std::string hours   =  (tm.hours < 10)   ? ("0" + std::to_string(tm.hours)   + ":") : std::to_string(tm.hours)   + ":";
	out << hours << minutes << second;
	return out;
}

enum class STATUS
{
	RUNNING,
	PAUSED,
	RESET,
	FAILED
};

enum class PROGRESS
{
	FOCUS,
	REST,
	FAILED
};

enum class TYPE
{
	STUDY,
	WORK,
	FAILED
};

std::string ToString(PROGRESS pg)
{
	if      (pg == PROGRESS::FOCUS) return "FOCUS";
	else if (pg == PROGRESS::REST)  return "REST!";
	else                            return "";
}

std::string ToString(TYPE pg)
{
	if      (pg == TYPE::WORK)  return "WORK!";
	else if (pg == TYPE::STUDY) return "STUDY";
	else                            return "";
}

TYPE ToType(std::string& s)
{
	s.erase(std::remove_if(s.begin(), s.end(), 
	[]( auto const& c ) -> bool { return !std::isalnum(c); } ), s.end());
	if      (s == "WORK")  return TYPE::WORK;
	else if (s == "STUDY") return TYPE::STUDY;
	else                   return TYPE::FAILED;
}

std::string CleanText(std::string s)
{
	s.erase(std::remove_if(s.begin(), s.end(), 
	[]( auto const& c ) -> bool { return !std::isalpha(c); } ), s.end());
	return s;
}

class Clock : public olc::PixelGameEngine
{
public: Clock() { sAppName = "Clock"; }

public:
	uint32_t radius = std::min(ScreenHeight(), ScreenWidth()) / 2;
	olc::Pixel color = olc::Pixel(255, 165, 0);

	std::vector<Time> marks;

	bool autoContinue = true;
	bool finishedType = false;

	Time totalTimeStart;
	Time totalTimeElapsedNow;
	Time totalTime = Time(0);

	Time totalWorkTime  = Time(0);
	Time totalStudyTime = Time(0);

	Time focusTime = Time(0, 25, 0);
	Time focusStart;
	Time restTime  = Time(0, 5, 0);
	Time restStart;

	Time bigRestTime = Time(0, 15, 0);
	int restCounter = 4;

	Time timeLeft;
	Time timer;

	STATUS status = STATUS::RESET;
	PROGRESS progress = PROGRESS::FOCUS;
	TYPE type = TYPE::WORK;

	olc::sound::WaveEngine soundEngine;
	olc::sound::Wave soundStartingFocus;
	olc::sound::Wave soundStartingRest;

private:
	void VerifyTimerToday()
	{
		std::ifstream fMarks("marks.csv");
		if(fMarks.is_open())
		{
			std::string stream;
			std::string lastStudy;
			std::string lastWork;

			while(std::getline(fMarks, stream))
			{
				if(stream == "" || stream == ";;;") 
					continue;

				std::istringstream ss(stream);
				std::string field;
				while(std::getline(ss, field, ';'))
				{
					if(ToType(field) == TYPE::WORK)
						lastWork = ss.str();
					if(ToType(field) == TYPE::STUDY)
						lastStudy = ss.str();
				}
			}
			totalWorkTime = GetLastType(lastWork);
			totalStudyTime = GetLastType(lastStudy);

			totalTime = totalStudyTime + totalWorkTime;
		}
		fMarks.close();
	}

	Time GetLastType(const std::string& line)
	{
		std::istringstream ss(line);
		std::string field;
		Time tempTotalTime(0);
		int collum = 0;
		bool hasTimerToday = false;
		while(std::getline(ss, field, ';'))
		{
			if(collum == 0)
				tempTotalTime.Parser(field);

			if(collum == 2)
			{
				std::time_t date_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				std::string date_now_name = std::ctime(&date_now);
				date_now_name.pop_back();
				hasTimerToday = field.substr(0, 10) == date_now_name.substr(0, 10);
			}
			++collum;
		}
		return hasTimerToday ? tempTotalTime : Time(0);
	}

	void Help()
	{
		std::cout << "|================================================|\n";
		std::cout << "|write 'H' or 'HELP' for command list            |\n";
		std::cout << "|For changing 'FOCUS' and 'REST' time use:       |\n";
		std::cout << "|> set [focus][rest] [time] [s][m][h]            |\n";
		std::cout << "|Exemple:                                        |\n";
		std::cout << "|> set rest 10 m                                 |\n";
		std::cout << "|                                                |\n";
		std::cout << "|For skip a period use: 'skip'                   |\n";
		std::cout << "|                                                |\n";
		std::cout << "|For turn auto continue: auto continue [on][off] |\n";
		std::cout << "|================================================|\n";
		std::cout << "\n";
	}

public:
	bool OnUserCreate() override
	{
		soundEngine.InitialiseAudio();
		auto t = GetFile(IDR_AUDIO1, AUDIO);
		if(t.size_bytes > 0)
			if(!soundStartingFocus.LoadAudioWaveform("", (char*)t.ptr, t.size_bytes))
				std::cout << "Error with sound\n";

		t = GetFile(IDR_AUDIO2, AUDIO);
		if(t.size_bytes > 0)
			if(!soundStartingRest.LoadAudioWaveform("", (char*)t.ptr, t.size_bytes))
				std::cout << "Error with sound\n";

		olc::sound::synth::ModularSynth synth;
		olc::sound::synth::modules::Oscillator osc1;
		osc1.waveform = olc::sound::synth::modules::Oscillator::Type::Wave;
		osc1.pWave = &soundStartingFocus;
		osc1.frequency = 5.0 / 20000.0;
		osc1.amplitude = 0.5;
		osc1.parameter = 0.5;

		ConsoleCaptureStdOut(true);
		Help();
		VerifyTimerToday();

		return true;
	}

	inline olc::vi2d CenterOfScreen()
	{
		return olc::vi2d(ScreenWidth() / 2, ScreenHeight() / 2);
	}

	bool OnDraw(float fElapsedTime)
	{
		Clear(olc::Pixel(128, 64, 128, (timer.seconds % 128) + 127));

		FillCircle(CenterOfScreen() - olc::vi2d(ScreenWidth()/4, 0), radius, olc::Pixel(155, 155, 155, 255));
		DrawCircle(CenterOfScreen() - olc::vi2d(ScreenWidth()/4, 0), radius + 0, olc::CYAN);

		DrawLine(CenterOfScreen() - olc::vi2d(ScreenWidth()/4, 0), CenterOfScreen() - olc::vi2d(ScreenWidth() / 4, 0) + olc::vi2d(radius * cos(((timer.seconds * 6.0) - 90) * (M_PI / 180.0)), 
																radius * sin(((timer.seconds * 6.0) - 90) * (M_PI / 180.0))), 
																olc::MAGENTA);
		
		DrawLine(CenterOfScreen() - olc::vi2d(ScreenWidth()/4, 0), CenterOfScreen() - olc::vi2d(ScreenWidth() / 4, 0) + olc::vi2d(radius/2 * cos(((timer.minutes * 6.0) - 90) * (M_PI / 180.0)), 
																radius/2 * sin(((timer.minutes * 6.0) - 90) * (M_PI / 180.0))), 
																olc::RED);
		
		DrawLine(CenterOfScreen() - olc::vi2d(ScreenWidth()/4, 0), CenterOfScreen() - olc::vi2d(ScreenWidth() / 4, 0) + olc::vi2d(radius/4 * cos(((timer.GetHoursGMT() * 30.0) - 90) * (M_PI / 180.0)), 
																																  radius/4 * sin(((timer.GetHoursGMT() * 30.0) - 90) * (M_PI / 180.0))), 
																olc::BLUE);
		

		DrawString(CenterOfScreen() - olc::vi2d(+ScreenWidth()/4 + radius/2, -(radius + 20)), timer.ToString(true), olc::WHITE, 2);

		return true;
	}
	
	void KeyboardInput()
	{
		if(!IsConsoleShowing() && GetKey(olc::ESCAPE).bPressed)
			status = STATUS::RESET;

		if(GetKey(olc::TAB).bPressed)
			ConsoleShow(olc::Key::ESCAPE);

		if(!IsConsoleShowing() && GetKey(olc::SPACE).bReleased)
		{
			if(status == STATUS::RESET)
			{
				focusStart          = timer;
				restStart           = timer;
				totalTimeStart      = timer;
				totalTimeElapsedNow = timer;
				timeLeft = focusTime;
				status = STATUS::RUNNING;
			}
			else if(status == STATUS::PAUSED)
			{
				focusStart = timer;
				restStart = timer;
				status = STATUS::RUNNING;
			}
			else if(status == STATUS::RUNNING)
			{
				status = STATUS::PAUSED;
			}
		}
	}

	bool OnConsoleCommand(const std::string& sText) override
	{
		std::stringstream ss;
		ss << sText;
		std::string c1;
		ss >> c1;
		if(c1 == "set")
		{
			ss >> c1;
			Time newTime;
			std::string time;
			ss >> time;
			std::string timeMagnitude;				
			ss >> timeMagnitude;

			if(timeMagnitude == "seconds" || timeMagnitude == "s")
				newTime = Time(atoi(time.c_str()));
			else if(timeMagnitude == "minutes" || timeMagnitude == "m")
				newTime = Time(atoi(time.c_str()) * 60);
			else if(timeMagnitude == "hours" || timeMagnitude == "h")
				newTime = Time(0, 0, atoi(time.c_str()));

			if(c1 == "focus")
				focusTime = newTime;
			else if(c1 == "rest")
				restTime = newTime;
				
			std::cout << "Focus Time: " << focusTime << "\n";
			std::cout << "Rest Time: " << restTime << "\n";

		}
		else if(c1 == "skip")
		{
			timeLeft = Time(0);
		}
		else if(c1 == "auto")
		{
			ss >> c1;
			if(c1 == "continue")
			{
				ss >> c1;
				if(c1 == "on") autoContinue = true;
				else if(c1 == "off") autoContinue = false;
			}
		}
		else if(c1 == "h" || c1 == "help")
		{
			Help();
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override  
	{
		timer.SetTime(time(0));
		OnDraw(fElapsedTime);
		KeyboardInput();
		for(int i = 0; i < marks.size() && i < 5; ++i)
		{
			DrawString(CenterOfScreen() + olc::vi2d(0, - radius + 100 + (20 * i)), std::to_string(i + 1) + ". " + marks[i].ToString(true) + " - " + focusTime.ToString(false), olc::CYAN, 2);
		}

		if(status == STATUS::RUNNING)
		{
			Time progressStart = progress == PROGRESS::FOCUS ? focusStart : restStart;

			DrawString(CenterOfScreen() - olc::vi2d(ScreenWidth()/4 + radius - 40, radius + 40), ToString(progress), olc::WHITE, 4);
			DrawString(CenterOfScreen() - olc::vi2d(-ScreenWidth()/4 + 100, radius + 40), ToString(type),     (type == TYPE::WORK) ? olc::GREY : olc::DARK_GREEN, 4);

			timeLeft      -= (timer - totalTimeElapsedNow);
			totalTime     += (timer - totalTimeElapsedNow);
			(type == TYPE::WORK ? totalWorkTime : totalStudyTime) += (timer - totalTimeElapsedNow);
			totalTimeElapsedNow = timer;

			DrawString(CenterOfScreen() + olc::vi2d(0, - radius + 20), timeLeft.ToString(false) + " - " + progressStart.ToString(true), olc::YELLOW , 2);
			DrawString(CenterOfScreen() + olc::vi2d(0, - radius + 60), (type == TYPE::WORK ? totalWorkTime : totalStudyTime).ToString(false), olc::GREEN, 2);
			DrawString(CenterOfScreen() + olc::vi2d(175, - radius + 60), totalTime.ToString(false), olc::Pixel(255, 255, 255, 64), 2);

			if(timeLeft.seconds < 0)
			{
				if(progress == PROGRESS::FOCUS)
				{
					marks.push_back(focusStart);
					soundEngine.PlayWaveform(&soundStartingRest);
					restStart = timer;
					timeLeft = restTime;
					progress = PROGRESS::REST;
					status = autoContinue ? status : STATUS::PAUSED;
					restCounter -= 1;
					if(restCounter == 0)
					{
						restCounter = 4;
						timeLeft = bigRestTime;
					}
				}
				
				else if(progress == PROGRESS::REST)
				{
					soundEngine.PlayWaveform(&soundStartingFocus);
					focusStart = timer;
					timeLeft = focusTime;
					progress = PROGRESS::FOCUS;
					status = autoContinue ? status : STATUS::PAUSED;
				}
			}
		}

		else
		{
			totalTimeElapsedNow = timer;
			DrawString( CenterOfScreen() - olc::vi2d(ScreenWidth()/4, 40),      "P A U S E D",   olc::Pixel(255, 255, 255, 64) , 4);			
			DrawString( CenterOfScreen() - olc::vi2d(ScreenWidth()/4 + 20, -40),"PRESS 'SPACE'", olc::Pixel(255, 255, 255, 64) , 4);			
			DrawString(olc::vi2d(ScreenWidth() - 195, ScreenHeight() - 12), "Press 'TAB' for commands", olc::WHITE, 1);
			DrawString(olc::vi2d(150, ScreenHeight() - 12), "Big Rest: " + std::to_string(restCounter), olc::Pixel(40 * restCounter, 255 - (20 * restCounter), 20 * restCounter), 1);
			
			std::string sAutoContinue = "Auto Continue: ";
			sAutoContinue += (autoContinue ? "On" : "Off");
			DrawString(olc::vi2d(2, ScreenHeight() - 12), sAutoContinue, autoContinue ? olc::GREEN : olc::RED, 1);
		}

		if(status == STATUS::RESET)
		{
			if(!IsConsoleShowing() && GetKey(olc::K1).bReleased)
				type = TYPE::WORK;
			if(!IsConsoleShowing() && GetKey(olc::K2).bReleased)
				type = TYPE::STUDY;
			olc::Pixel selectedColor    = olc::Pixel(50, 200, 50);
			olc::Pixel notSelectedColor = olc::Pixel(255, 255, 255, 64);
			olc::Pixel workColor  = (type == TYPE::WORK) ? selectedColor : notSelectedColor;
			olc::Pixel studyColor = (type == TYPE::STUDY) ? selectedColor : notSelectedColor;
			DrawString( CenterOfScreen() - olc::vi2d(ScreenWidth()/4 + 80, 160),  "1.WORK",  workColor, 4);			
			DrawString( CenterOfScreen() - olc::vi2d(ScreenWidth()/4 - 200, 160), "2.STUDY", studyColor, 4);			
		}

		return true;
	}
};

int main()
{
	{
		std::time_t date_started = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		
		Clock app;
		if (app.Construct(700, 400, 1, 1)){
			app.Start();
		}

		csvfile csv("marks.csv");
		if(!csv.alreadyExist) csv << "Total Time" << "Started" << "Finished" << endrow;
		std::time_t date_finished = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string date_started_name = std::ctime(&date_started);
		date_started_name.pop_back();
		std::string date_finished_name = std::ctime(&date_finished);
		date_finished_name.pop_back();
		
		csv << app.totalWorkTime;
		csv << date_started_name;
		csv << date_finished_name;
		csv << ToString(TYPE::WORK);
		csv << endrow;
	
		csv << app.totalStudyTime;
		csv << date_started_name;
		csv << date_finished_name;
		csv << ToString(TYPE::STUDY);
		csv << endrow;
	}
	return 0;
}