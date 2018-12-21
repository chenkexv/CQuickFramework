#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_CMyFrameExtension.h"
#include "Zend/zend_list.h"
#include "Zend/zend_interfaces.h"

#ifdef PHP_WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winbase.h>

LARGE_INTEGER liOldIdleTime = {0,0}; 
LARGE_INTEGER liOldSystemTime = {0,0};
 
#define SystemBasicInformation 0 
#define SystemPerformanceInformation 2 
#define SystemTimeInformation 3
 
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
 
typedef struct 
{ 
 DWORD dwUnknown1; 
 ULONG uKeMaximumIncrement; 
 ULONG uPageSize; 
 ULONG uMmNumberOfPhysicalPages; 
 ULONG uMmLowestPhysicalPage; 
 ULONG uMmHighestPhysicalPage; 
 ULONG uAllocationGranularity; 
 PVOID pLowestUserAddress; 
 PVOID pMmHighestUserAddress; 
 ULONG uKeActiveProcessors; 
 BYTE bKeNumberProcessors; 
 BYTE bUnknown2; 
 WORD wUnknown3; 
} SYSTEM_BASIC_INFORMATION;
 
typedef struct 
{ 
 LARGE_INTEGER liIdleTime; 
 DWORD dwSpare[76]; 
} SYSTEM_PERFORMANCE_INFORMATION;
 
typedef struct 
{ 
 LARGE_INTEGER liKeBootTime; 
 LARGE_INTEGER liKeSystemTime; 
 LARGE_INTEGER liExpTimeZoneBias; 
 ULONG uCurrentTimeZoneId; 
 DWORD dwReserved; 
} SYSTEM_TIME_INFORMATION;
 
typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
PROCNTQSI NtQuerySystemInformation;
 
 
//CPU核心数
int getCpuCoreNumber()  
{  
    SYSTEM_INFO info;  
    GetSystemInfo(&info);  
    return (int)info.dwNumberOfProcessors;  
}

typedef void(WINAPI* FunctionGlobalMemoryStatusEx)(LPMEMORYSTATUS);
MEMORYSTATUS GetMemoryStatus()
{
	HMODULE hModule;
	FunctionGlobalMemoryStatusEx GlobalMemoryStatusEx;
	MEMORYSTATUS status;
	status.dwLength = sizeof(status);
	hModule = LoadLibrary("kernel32.dll");
	if(NULL==hModule){
		return;
	}
       
	GlobalMemoryStatusEx   = (FunctionGlobalMemoryStatusEx)GetProcAddress(hModule,"GlobalMemoryStatusEx");
	if(NULL==GlobalMemoryStatusEx){
		return;
	}
	GlobalMemoryStatusEx(&status);
	FreeLibrary(hModule);
	return status;
}

//返回内存MB
long getMemory()
{
	char *all;
	MEMORYSTATUS status;
	GlobalMemoryStatus(&status);
	return status.dwTotalPhys/1024/1024;
}

long getFreeMemory()
{
	char *all;
	MEMORYSTATUS status;
	GlobalMemoryStatus(&status);
	return status.dwAvailPhys/1024/1024;
}

//获取虚拟内存大小
long getFreeVirtualMemory()
{
	char *all;
	MEMORYSTATUS status;
	GlobalMemoryStatus(&status);
	return status.dwAvailVirtual/1024/1024;
}

//所有虚拟内存
long getVirtualMemory()
{
	char *all;
	MEMORYSTATUS status;
	GlobalMemoryStatus(&status);
	return status.dwTotalVirtual/1024/1024;
}
 
//获得CPU使用率 对NT内核有效
long GetCpuUsage()
{
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo; 
    SYSTEM_TIME_INFORMATION SysTimeInfo; 
    SYSTEM_BASIC_INFORMATION SysBaseInfo; 
    double dbIdleTime = 0; 
    double dbSystemTime = 0; 
    LONG status; 

	SYSTEM_INFO si;
	GetSystemInfo(&si);
 
    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle("ntdll"),"NtQuerySystemInformation");
    if (!NtQuerySystemInformation) 
        return -1;
 
    status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0); 
    if (status!=NO_ERROR) {
        return -1;
	}

    status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL); 
    if (status != NO_ERROR) {
        return -1;
	}

    if (liOldIdleTime.QuadPart != 0) { 
        dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime); 
        dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

        dbIdleTime = dbIdleTime / dbSystemTime;
        dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)si.dwNumberOfProcessors + 0.5;
 
    }

    liOldIdleTime = SysPerfInfo.liIdleTime; 
    liOldSystemTime = SysTimeInfo.liKeSystemTime;
 
    return (int)dbIdleTime;
}

//获取进程pid
long getProcessInfo(char *sendName)
{
	char name[1024];
	PROCESSENTRY32 pe32;
	HANDLE hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb = sizeof(PPROCESS_MEMORY_COUNTERS);
	sprintf(name,"%s%s",sendName,".exe");


	if (!hSnapshot){
		return -1;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32 );
	if (!Process32First (hSnapshot, &pe32)){
		return -1;
	}

	do
	{
		if( strcmp(name,pe32.szExeFile) == 0) {
			//打开进程内存空间
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pe32.th32ProcessID); 
			if(NULL == hProcess) return -1;
			if(GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc))){
				return pmc.WorkingSetSize / 1024 ;
			}
		}
	}
	while(Process32Next (hSnapshot, &pe32));
}

//当前宿主进程信息
long getCurrentProcessInfo()
{
	PROCESS_MEMORY_COUNTERS pmc;
	if(GetProcessMemoryInfo(GetCurrentProcess(),&pmc,sizeof(pmc))){
		return pmc.WorkingSetSize / 1024 ;
	}else{
		return 0;
	}
}

//获取硬盘空间
long getDiskAll(char *disk,int type)
{
	ULARGE_INTEGER all,
		 free,
		 has;

	int status;

	status = GetDiskFreeSpaceEx(disk,&free,&all,&has);
	
	if(status == 0) return 0;

	if(type == 1){
		return free.QuadPart /1024/1024/1024;
	}else if(type == 2){
		return all.QuadPart /1024/1024/1024;
	}
}


//获取最大内存占用
void getProcessList(zval **returnZval)
{
	HashTable *processTable;

	zval *thisAddVal,
		 *returnData;

	PROCESSENTRY32 pe32;
	HANDLE hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
	PROCESS_MEMORY_COUNTERS pmc;

	TSRMLS_FETCH();

	pmc.cb = sizeof(PPROCESS_MEMORY_COUNTERS);

	ALLOC_HASHTABLE(processTable);
	zend_hash_init(processTable,256,NULL,NULL,0);

	if (!hSnapshot){
		return -1;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32 );
	if (!Process32First (hSnapshot, &pe32)){
		return -1;
	}

	do{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pe32.th32ProcessID); 
		if(NULL != hProcess){
			if(GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc))){
	
				if(zend_hash_exists(processTable,pe32.szExeFile,strlen(pe32.szExeFile)+1) == 1){
					char *newKey;
					newKey = estrdup(contact(pe32.szExeFile,"*"));
					MAKE_STD_ZVAL(thisAddVal);
					ZVAL_LONG(thisAddVal,pmc.WorkingSetSize / 1024);
					zend_hash_add(processTable,newKey,strlen(newKey)+1,&thisAddVal,sizeof(zval*),NULL);
				}else{
					MAKE_STD_ZVAL(thisAddVal);
					ZVAL_LONG(thisAddVal,pmc.WorkingSetSize / 1024);
					zend_hash_add(processTable,pe32.szExeFile,strlen(pe32.szExeFile)+1,&thisAddVal,sizeof(zval*),NULL);
				}
			}
		}
	}
	while(Process32Next(hSnapshot, &pe32));

	MAKE_STD_ZVAL(*returnZval);
	Z_TYPE_P(*returnZval) = IS_ARRAY;
	Z_ARRVAL_P(*returnZval) = processTable;
}



//获取基于Vista内核的CPU使用率
int getVistaCPU()
{
	return 5;
}

//尝试获取服务器当前TCP连接数

#else
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
//非window下
unsigned long getMemory()
{
	struct sysinfo si;
    sysinfo(&si);
    return (si.totalram/1024/1024);
}

unsigned long getFreeMemory()
{
	struct sysinfo si;
    sysinfo(&si);
    return (si.freeram/1024/1024);
}

//获取虚拟内存大小
unsigned long getFreeVirtualMemory()
{
	struct sysinfo si;
    sysinfo(&si);
	return (si.freeswap/1024/1024);
}

//所有虚拟内存
unsigned long getVirtualMemory()
{
	struct sysinfo si;
    sysinfo(&si);
    return (si.totalswap/1024/1024);
}
 
unsigned long GetCpuUsage()
{

	
}

//获取进程pid
unsigned long getProcessInfo(char *name)
{



	return 0;
}

//当前宿主进程信息
unsigned long getCurrentProcessInfo()
{
	return 0;
}

//获取硬盘空间
unsigned long getDiskAll(char *disk,int type)
{

}


//获取最大内存占用
void getProcessList(zval **returnZval)
{

}


long getVistaCPU()
{
	return 0;
}

#endif