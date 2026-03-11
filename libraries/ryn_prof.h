/*
*/

/*
Example:

#include "ryn_prof.h"

void Foo(int I) {
  int J, K, X = 0;

  if (I < 50000) {
    ryn_BEGIN_TIMED_BLOCK(case_1);

    for (J = 0; J < 10000; J++) {
      X += 1;
    }
    ryn_END_TIMED_BLOCK(case_1);
  } else {
    ryn_BEGIN_TIMED_BLOCK(case_2);
    for (K = 0; K < 10000; K++) {
      X += 1;
    }
    ryn_END_TIMED_BLOCK(case_2);
  }
}

int main(void) {
  int I;
  ryn_BeginProfile();

  ryn_BEGIN_TIMED_BLOCK(foo);
  for (I = 0; I < 80000; I++) {
    Foo(I);
  }
  ryn_END_TIMED_BLOCK(foo);

  ryn_EndAndPrintProfile();
  return 0;
}
*/




// TODO: this only works on mac right now...
#if OS_MAC
# include <mach-o/getsect.h>
#endif

#if Ryn_Prof_Use_Print
# define Print_String(s)\
    printf(s)

# define Print_Format_String(f, ...)\
    printf(f, __VA_ARGS__)
#else
# define Print_String(s)
# define Print_Format_String(f, ...)
#endif

typedef struct
{
    uint64_t ElapsedExclusive;
    uint64_t ElapsedInclusive;
    uint64_t HitCount;
    uint64_t ProcessedByteCount;
    char *Label;
} ryn_timer_data;




// naming/typing symbol-set
#define SYMBOL_SET_DEFINE ryn_sym_timer
#define ryn_sym_timer_Type      ryn_timer_data
#define ryn_sym_timer_section   "_rpftmr"
#define RYN_PROF_SYM_ID(N)   SymbolID(ryn_sym_timer, N)
#define RYN_PROF_SYM_RAW(N)  SymbolRaw(ryn_sym_timer, N)
#define RYN_PROF_SYM_DECL(N) SymbolDeclare(ryn_sym_timer, N)
#include "../libraries/mr4th/src/mr4th_symbol_set.define.h"










typedef struct
{
    uint64_t StartTime;
    uint64_t EndTime;

    /* ryn_timer_data *DebugTimers; */
} ryn_profiler;

static ryn_profiler ryn_GlobalProfiler;
uint32_t ryn_GlobalActiveTimer;

#ifndef ryn_PROFILER
#define ryn_PROFILER 1
#endif

#if ryn_PROFILER
#include <stdio.h>
#include <x86intrin.h>
#include <sys/time.h>

#define ryn_ArrayCount(a) ((sizeof(a))/(sizeof((a)[0])))

uint64_t ryn_ReadCPUTimer(void);
uint64_t ryn_ReadOSTimer(void);
void ryn_BeginProfile(void);
void ryn_EndProfile(void);
void ryn_EndAndPrintProfile(uint64_t CPUFreq);

inline uint64_t ryn_ReadCPUTimer(void)
{
    return __rdtsc();
}

static uint64_t ryn_GetOSTimerFreq(void)
{
    return 1000000;
}

uint64_t ryn_ReadOSTimer(void)
{
    struct timeval Value;
    gettimeofday(&Value, 0);
    uint64_t Result = ryn_GetOSTimerFreq()*(uint64_t)Value.tv_sec + (uint64_t)Value.tv_usec;
    return Result;
}

static int ryn_EstimateCpuFrequency(uint64_t MsToWait)
{
    uint64_t MillisecondsToWait = MsToWait;
    uint64_t OSFreq = ryn_GetOSTimerFreq();
    uint64_t CPUStart = ryn_ReadCPUTimer();
    uint64_t OSStart = ryn_ReadOSTimer();
    uint64_t OSEnd = 0;
    uint64_t OSElapsed = 0;
    uint64_t OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while(OSElapsed < OSWaitTime)
    {
        OSEnd = ryn_ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }
    uint64_t CPUEnd = ryn_ReadCPUTimer();
    uint64_t CPUElapsed = CPUEnd - CPUStart;
    uint64_t CPUFreq = 0;
    if(OSElapsed)
    {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }
    return CPUFreq;
}




static ryn_timer_data ryn_prof_ZeroTimerData;


#define ryn__BEGIN_TIMED_BLOCK(TimerKey, TargetTimer, ByteCount)\
  static RYN_PROF_SYM_DECL(TimerKey);\
  ryn_timer_data *Timer##__##TimerKey##__Data = SymbolMetadata(ryn_sym_timer, TimerKey);\
  Timer##__##TimerKey##__Data = Timer##__##TimerKey##__Data ? Timer##__##TimerKey##__Data : &ryn_prof_ZeroTimerData;\
  uint32_t ParentTimer##TimerKey = ryn_GlobalActiveTimer;\
  Timer##__##TimerKey##__Data->Label = #TimerKey;\
  Timer##__##TimerKey##__Data->ProcessedByteCount += ByteCount;\
  uint64_t OldTSCElapsedInclusive##TimerKey = Timer##__##TimerKey##__Data->ElapsedInclusive;\
  TargetTimer = RYN_PROF_SYM_ID(TimerKey);\
  uint64_t StartTime##TimerKey = ryn_ReadCPUTimer();


#define ryn__END_TIMED_BLOCK(TimerKey, TargetTimer)\
  uint64_t Elapsed##TimerKey = ryn_ReadCPUTimer() - StartTime##TimerKey;\
  TargetTimer = ParentTimer##TimerKey;\
  ryn_timer_data *ParentTimer##__##TimerKey##__Data = SymbolMetadataFromID(ryn_sym_timer, ParentTimer##TimerKey);\
  ParentTimer##__##TimerKey##__Data = ParentTimer##__##TimerKey##__Data ? ParentTimer##__##TimerKey##__Data : &ryn_prof_ZeroTimerData;\
  ParentTimer##__##TimerKey##__Data->ElapsedExclusive -= Elapsed##TimerKey;\
  Timer##__##TimerKey##__Data->ElapsedExclusive += Elapsed##TimerKey;\
  Timer##__##TimerKey##__Data->ElapsedInclusive = OldTSCElapsedInclusive##TimerKey + Elapsed##TimerKey;\
  Timer##__##TimerKey##__Data->HitCount += 1;



#define ryn_BEGIN_BANDWIDTH_BLOCK(TimerKey, ByteCount) ryn__BEGIN_TIMED_BLOCK(TimerKey, ryn_GlobalActiveTimer, ByteCount)
#define ryn_BEGIN_TIMED_BLOCK(TimerKey) ryn__BEGIN_TIMED_BLOCK(TimerKey, ryn_GlobalActiveTimer, 0)
#define ryn_END_TIMED_BLOCK(TimerKey) ryn__END_TIMED_BLOCK(TimerKey, ryn_GlobalActiveTimer)

static void PrintTimeElapsed(uint64_t TotalElapsedTime, uint64_t CPUFreq, ryn_timer_data *Timer)
{
    double Percent = 100.0 * ((double)Timer->ElapsedExclusive / (double)TotalElapsedTime);
    Print_Format_String("  %s[%llu]: %llu (%.2f%%", Timer->Label, Timer->HitCount, Timer->ElapsedExclusive, Percent);
    if(Timer->ElapsedInclusive != Timer->ElapsedExclusive)
    {
        double PercentWithChildren = 100.0 * ((double)Timer->ElapsedInclusive / (double)TotalElapsedTime);
        Print_Format_String(", %.2f%% w/children", PercentWithChildren);
    }

    if(Timer->ProcessedByteCount)
    {
        double Megabyte = 1024.0f*1024.0f;
        double Gigabyte = Megabyte*1024.0f;

        double Seconds = (double)Timer->ElapsedInclusive / (double)CPUFreq;
        double BytesPerSecond = (double)Timer->ProcessedByteCount / Seconds;
        double Megabytes = (double)Timer->ProcessedByteCount / (double)Megabyte;
        double GigabytesPerSecond = BytesPerSecond / Gigabyte;

        Print_Format_String("  %.3fmb at %.2fgb/s", Megabytes, GigabytesPerSecond);
    }
    Print_String(")\n");
}

void ryn_BeginProfile(void)
{
    ryn_GlobalProfiler.StartTime = ryn_ReadCPUTimer();
}

void ryn_EndProfile(void)
{
    ryn_GlobalProfiler.EndTime = ryn_ReadCPUTimer();
}

void ryn_EndAndPrintProfile(uint64_t CPUFreq)
{
    ryn_GlobalProfiler.EndTime = ryn_ReadCPUTimer();

    uint64_t TotalElapsedTime = ryn_GlobalProfiler.EndTime - ryn_GlobalProfiler.StartTime;

    if(CPUFreq)
    {
        float TotalElapsedTimeInMs = 1000.0 * (double)TotalElapsedTime / (double)CPUFreq;
        Print_Format_String("\nTotal time: %0.4fms (CPU freq %llu)\n", TotalElapsedTimeInMs, CPUFreq);
    }

    for(uint32_t TimerIndex = 0; TimerIndex < SymbolCount(ryn_sym_timer); ++TimerIndex)
    {
        ryn_timer_data *Timer = SymbolMetadataFromID(ryn_sym_timer, TimerIndex+1);
        if(Timer->ElapsedInclusive)
        {
            PrintTimeElapsed(TotalElapsedTime, CPUFreq, Timer);
        }
    }
}

#else // ryn_PROFILER

#define ryn_BEGIN_TIMED_BLOCK(...)
#define ryn_END_TIMED_BLOCK(...)

#define ryn_BeginProfile(...)
#define ryn_EndProfile(...)

static int ryn_EstimateCpuFrequency(uint64_t MsToWait)
{
    return 0;
}

#endif // ryn_PROFILER

#undef ryn_ArrayCount
