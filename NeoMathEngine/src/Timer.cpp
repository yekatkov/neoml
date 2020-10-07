#include <common.h>
#pragma hdrstop

#include <NeoMathEngine/Timer.h>
#include <NeoMathEngine/NeoMathEngineDefs.h>

std::map<std::string, CTimer::CTimerStruct> CTimer::Timers;
std::mutex CTimer::TimersGuard;

NEOMATHENGINE_API const std::string& PrintTimers()
{
	static string str;
	str = CTimer::PrintTimers();
	return str;
}

std::list<CAlgoInfo::CInfo> CAlgoInfo::Algo0VsFastAlgoInfo;
std::list<CAlgoInfo::CInfo> CAlgoInfo::FastAlgoInfo;
std::list<CAlgoInfo::CInfo> CAlgoInfo::Algo0Info;
std::mutex CAlgoInfo::AlgoInfoGuard;

NEOMATHENGINE_API const std::string& PrintAlgoInfo()
{
	static string str;
	str = CAlgoInfo::PrintAlgoInfo();
	return str;
}