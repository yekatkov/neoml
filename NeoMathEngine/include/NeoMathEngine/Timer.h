#pragma once

#include <chrono>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <list>
#include <mutex>
#include <string>

class CTimer {
public:
	CTimer( bool start = false ) : CTimer( "", start ) {}

	CTimer( const char* _name, bool start = false ) : name( _name ), timeDelay( 0 ), count( 0 ), isStarted( false )
	{
		if( start ) {
			Start();
		}
	}
	~CTimer()
	{
		if( isStarted ) {
			Stop();
		}
		if( !name.empty() ) {
			const std::lock_guard<std::mutex> lock( TimersGuard );
			auto& timer = Timers[name];
			timer.delay += timeDelay;
			timer.count += count;
		}
	}
	void Clear()
	{
		timeDelay = std::chrono::nanoseconds::zero();
		count = 0;
		isStarted = false;
	}

	void Start()
	{
		//assert( !isStarted );
		startTime = std::chrono::high_resolution_clock::now();
		isStarted = true;
	}
	void Stop()
	{
		//assert( isStarted );
		auto stopTime = std::chrono::high_resolution_clock::now();
		timeDelay += std::chrono::duration_cast<std::chrono::nanoseconds>( stopTime - startTime );
		count++;
		isStarted = false;
	}

	float GetTimeInMs() const
	{
		auto currentTime = isStarted ? std::chrono::high_resolution_clock::now() - startTime : timeDelay;
		return currentTime.count() / 1e6f;
	}

	static std::string PrintTimers()
	{
		using namespace std;
		const std::lock_guard<std::mutex> lock( TimersGuard );
		stringstream ss;
		ss << endl << "_tmrs_;Timer name;Total, ms;Avrg, ms;Count" << endl;

		for( auto& timer : Timers ) {
			auto& timerStruct = timer.second;
			ss << "_tmrs_;"
				<< timer.first << ";"
				<< timerStruct.delay.count() / 1000000 << ";"
				<< setprecision( 3 ) << timerStruct.delay.count() / 1e6 / timerStruct.count << ";" << fixed
				<< timerStruct.count << endl;
		}
		Timers.clear();
		return ss.str();
	}

private:
	struct CTimerStruct {
		CTimerStruct() : delay( 0 ), count( 0 ) {}

		std::chrono::nanoseconds delay;
		int64_t count;
	};

	std::string name;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::nanoseconds timeDelay;
	int64_t count;
	bool isStarted;
	static std::map<std::string, CTimerStruct> Timers;
	static std::mutex TimersGuard;
};



class CAlgoInfo {
public:

	struct CInfo {
		std::vector<CTimer> Timers;
		std::vector<int> Dimentions;
	};


	static void AddFastAlgo( CInfo&& fastAlgoInfo )
	{
		const std::lock_guard<std::mutex> lock( AlgoInfoGuard );
		FastAlgoInfo.push_back( fastAlgoInfo );
	}

	static void AddAlgo0Info( CInfo&& algo0Info )
	{
		const std::lock_guard<std::mutex> lock( AlgoInfoGuard );
		Algo0Info.push_back( algo0Info );
	}

	static void AddAlgo0VsFastAlgoInfo( CInfo&& algo0VsFastAlgoInfo )
	{
		const std::lock_guard<std::mutex> lock( AlgoInfoGuard );
		Algo0VsFastAlgoInfo.push_back( algo0VsFastAlgoInfo );
	}

	static std::string PrintAlgoInfo()
	{
		const std::lock_guard<std::mutex> lock( AlgoInfoGuard );

		using namespace std;
		stringstream ss;

		printInfo( ss, Algo0VsFastAlgoInfo, "Algo0;FastAlgo;SW;D;S", "_info_a0vsfa" );
		printInfo( ss, FastAlgoInfo, "t0;t1;t2;SW;SH;FW;FH;D;P;C;FC;idx", "_info_fastAlgo" );
		printInfo( ss, Algo0Info, "full;t0;t1;t2", "_info_algo0" );

		return ss.str();

	}
private:
	static void printInfo( stringstream& ss, const std::list<CInfo>& info, const char* head, const char* tag )
	{
		ss << endl << tag << ";" << head << endl;
		for( auto& i : info ) {
			ss << tag << ";";
			for( auto& t : i.Timers ) {
				ss << t.GetTimeInMs() << ";";
			}
			for( auto& d : i.Dimentions ) {
				ss << d << ";";
			}
			ss << endl;
		}
	}
	static std::list<CInfo> Algo0VsFastAlgoInfo;
	static std::list<CInfo> FastAlgoInfo;
	static std::list<CInfo> Algo0Info;
	static std::mutex AlgoInfoGuard;
};


