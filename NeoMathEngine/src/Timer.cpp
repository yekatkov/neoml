#include <common.h>
#pragma hdrstop

#include <NeoMathEngine/Timer.h>
#include <NeoMathEngine/NeoMathEngineDefs.h>

std::map<std::string, CTimer::CTimerStruct> CTimer::Timers;
std::mutex CTimer::TimersGuard;

std::array<std::multimap<std::list<int>, CAlgoInfo::CInfo>, 10> CAlgoInfo::CountedInfo;
std::array<std::list<CAlgoInfo::CInfo>, 10> CAlgoInfo::Info;
std::mutex CAlgoInfo::Lock;

NEOMATHENGINE_API const std::string& PrintTimers( int idx, const char* head, const char* tag )
{
	static string str;
	str = CAlgoInfo::PrintAlgoInfo( idx, head, tag );
	return str;
}

NEOMATHENGINE_API const std::string& PrintCountedInfo( int idx, const char* idxTag, const char* head1, const char* head2, const char* tag )
{
	static string str;
	str = CAlgoInfo::PrintCountedAlgoInfo( idx, idxTag, head1, head2, tag );
	return str;
}