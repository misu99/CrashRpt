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
    static bool Initialize(const LPCTSTR &confName);    //��ʼ�������������ļ�����ڳ�������·��
    static void AddFile(const LPCTSTR &strFilePath);    //����ļ�(��־�ȣ�ͬ�����·��)
    static void SetCrashCallback(PFNCRASHCALLBACK pfnCallbackFunc, LPVOID lpParam); //���ûص�����

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

    bool SetInstance(const TConfParams &tconf); //���ò���ʵ��

    std::wstring GetAppPath(void);  //��ȡ����Ŀ¼
    std::wstring GetAppName(void);  //��ȡ������
    bool GetConfParams(const LPCTSTR &confName, TConfParams &tconf); //��ȡ���ò���
	WORD GetSmtpSecurity(const LPCTSTR &str);

    inline std::wstring GetConfValue(const LPCTSTR &location, const LPCTSTR &name);   //��ȡ�����ֶ�

    static CrashHelper *m_pInstance;
    std::wstring m_strConfPath;	//�����ļ�·��
    std::wstring m_strAppPath;	//��������·��

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
