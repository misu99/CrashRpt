/**
 * @file CrashHelper.h
 *
 * @brief Declares the crash handler class.
 */
#pragma once
#ifndef __CRASHHELPER_H__
#define __CRASHHELPER_H__

#include <string>

#include "../CrashRpt.h"

class CrashHelper {
  public:
    static bool Initialize(const LPCTSTR &confName);    //初始化，传入配置文件相对于程序的相对路径
    static void AddFile(const LPCTSTR &strFilePath);    //添加文件(日志等，同上相对路径)
    static void SetCrashCallback(PFNCRASHCALLBACK pfnCallbackFunc, LPVOID lpParam); //设置回调函数

  private:
    struct TConfParams {
        bool isRestart;
        int nRestartTimeout;
        WORD wSmtpSecurity;
        std::wstring dumpDir;
        std::wstring icon;
        std::wstring privacy;
        std::wstring smtpProxy;
        std::wstring smtpUser;
        std::wstring smtpPwd;
        std::wstring httpUrl;
        std::wstring emailSendto;
        std::wstring emailSubject;
        std::wstring emailText;
    };

    CrashHelper(void);
    ~CrashHelper(void);

    bool SetInstance(const TConfParams &tconf); //设置捕获实例

    std::wstring GetAppPath(void);  //获取程序目录
    std::wstring GetAppName(void);  //获取程序名
    bool GetConfParams(const LPCTSTR &confName, TConfParams &tconf); //获取配置参数
	WORD GetSmtpSecurity(const LPCTSTR &str);

    inline std::wstring GetConfValue(const LPCTSTR &location, const LPCTSTR &name);   //获取配置字段

    static CrashHelper *m_pInstance;
    std::wstring m_strConfPath;	//配置文件路径
    std::wstring m_strAppPath;	//程序所在路径

    class GarbageCleanup {
      public:
        ~GarbageCleanup() {
            if (CrashHelper::m_pInstance) {
                delete CrashHelper::m_pInstance;
            }
        }
    };
    static GarbageCleanup garbage;
};
#endif // !__CRASHHELPER_H__
