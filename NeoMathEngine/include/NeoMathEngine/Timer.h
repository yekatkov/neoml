#pragma once

#include <chrono>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <list>
#include <mutex>
#include <string>
#include <array>

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
	std::chrono::system_clock::time_point startTime;
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


	static void AddInfo( CInfo&& info, int idx )
	{
		const std::lock_guard<std::mutex> lock( Lock );
		Info[idx].push_back( info );
	}

	static void AddCountedInfo( std::list<int> key, CInfo&& info, int idx )
	{
		const std::lock_guard<std::mutex> lock( Lock );	
		CountedInfo[idx].insert( std::make_pair( key, info ) );
	}

	static std::string PrintAlgoInfo( int idx, const char* head, const char* tag )
	{
		const std::lock_guard<std::mutex> lock( Lock );

		using namespace std;
		stringstream ss;

		printInfo( ss, Info[idx], head, tag );

		for( auto i : Info ) {
			i.clear();
		}
		return ss.str();

	}

	static std::string PrintCountedAlgoInfo( int idx, const char* idxTag, const char* head1, const char* head2, const char* tag )
	{
		const std::lock_guard<std::mutex> lock( Lock );

		using namespace std;
		stringstream ss;

		printCountedInfo( ss, CountedInfo[idx], idxTag, head1, head2, tag );

		for( auto i : CountedInfo ) {
			i.clear();
		}
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

	static void printCountedInfo( stringstream& ss, const std::multimap<std::list<int>, CInfo>& countedinfo, 
		const char* idxTag, const char* head1, const char* head2, const char* tag )
	{
		if( countedinfo.empty() ) {
			return;
		}

		ss << ";" << endl;
		ss << endl << "_head_" << ";" << ";" << head1 << endl;
		for( auto i = countedinfo.begin(), end = countedinfo.end(); i != end; i = countedinfo.upper_bound( i->first ) ) {
			if( i == countedinfo.begin() ) {
				ss << "_head_" << tag << idxTag << ";";
			} else {
				ss << "_head_;;";
			}
			ss << countedinfo.count( ( *i ).first ) << ";";
			for( auto& j : ( *i ).first ) {
				ss << j << ";";
			}
			ss << endl;
		}
		
		ss << endl << tag << ";" << head2 << endl;
		for( auto& i : countedinfo ) {
			ss << tag << ";";
			for( auto& t : i.second.Timers ) {
				ss << t.GetTimeInMs() << ";";
			}
			for( auto& d : i.second.Dimentions ) {
				ss << d << ";";
			}
			ss << endl;
		}
	}

	static std::array <std::multimap<std::list<int>, CInfo>, 10> CountedInfo;
	static std::array<std::list<CInfo>, 10> Info;
	static std::mutex Lock;
};


