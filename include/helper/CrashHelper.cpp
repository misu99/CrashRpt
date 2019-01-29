#include "CrashHelper.h"
#include <tchar.h>

#ifdef _CRASHRPTTEST
#	ifdef _DEBUG
#	pragma comment(lib,"CrashRpt1403d.lib")
#	else
#	pragma comment(lib,"CrashRpt1403.lib")
#	endif
#else
# pragma comment(lib,"CrashRpt1403.lib")
#endif

CrashHelper::GarbageCleanup CrashHelper::garbage;
CrashHelper *CrashHelper::m_pInstance = new CrashHelper;

//�ص�ʾ��
//int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO *pInfo) {
//    switch (pInfo->nStage) {
//    case CR_CB_STAGE_PREPARE:   //�쳣����ǰ
//        //��������(��ر���־�Խ���ļ�ռ��)
//        break;
//
//    case CR_CB_STAGE_FINISH:    //�쳣�������
//        break;
//    }
//
//    //������һ�׶�
//    return CR_CB_NOTIFY_NEXT_STAGE;
//    //return CR_CB_DODEFAULT;
//}

CrashHelper::CrashHelper() {
}

CrashHelper::~CrashHelper() {
    crUninstall();
}

bool CrashHelper::Initialize(const LPCTSTR &confName) {
    TConfParams tparams;

    if (!m_pInstance->GetConfParams(confName, tparams)) {
        return false;
    }

    return m_pInstance->SetInstance(tparams);
}

void CrashHelper::AddFile(const LPCTSTR &strFilePath) {
    std::wstring strPathTemp = strFilePath;
    WIN32_FIND_DATA FindFileData;

    if (INVALID_HANDLE_VALUE == FindFirstFile(strFilePath, &FindFileData)) {
        strPathTemp = m_pInstance->m_strAppPath + strFilePath;

        if (INVALID_HANDLE_VALUE == FindFirstFile(strPathTemp.c_str(), &FindFileData)) {
            return;
        }
    }

    auto nPosA = strPathTemp.rfind('\\');
    auto nPosB = strPathTemp.rfind('/');
    auto nPos = (std::wstring::npos == nPosA ? nPosB : (std::wstring::npos == nPosB ? std::wstring::npos : (nPosA > nPosB ? nPosA : nPosB)));
    auto strName = strPathTemp.substr(nPos + 1);
    crAddFile2(strPathTemp.c_str(), strName.c_str(), _T("Additional files"), CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
}

void CrashHelper::SetCrashCallback(PFNCRASHCALLBACK pfnCallbackFunc, LPVOID lpParam) {
    crSetCrashCallback(pfnCallbackFunc, lpParam);
}

bool CrashHelper::SetInstance(const TConfParams &tconf) {
    std::wstring strAppName = GetAppName();

    //�����趨
    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);

    //��������
    info.pszAppName = strAppName.c_str();
    info.pszAppVersion = _T("1.0.0.0");
    info.pszErrorReportSaveDir = tconf.dumpDir.c_str();
    info.pszCustomSenderIcon = tconf.icon.c_str();
    info.pszPrivacyPolicyURL = tconf.privacy.c_str();  //��˽����url

    //�����ʼ�����
    info.pszEmailTo = tconf.emailSendto.c_str();
    info.pszEmailSubject = tconf.emailSubject.c_str();
    info.pszEmailText = tconf.emailText.c_str();

    //�������ȼ�
    info.uPriorities[CR_HTTP] = 3;  //����ʹ��htpp
    info.uPriorities[CR_SMTP] = 2;  //���ʹ��smtp
    info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; //��ʹ�õ������ʼ��ͻ���

    //smtp��������
    info.wSmtpSecurity = tconf.wSmtpSecurity;       //smtp���ܷ�ʽ
    info.pszSmtpProxy = tconf.smtpProxy.c_str();    //smtp��ַ�˿�(mail.example.com:25)
    info.pszSmtpLogin = tconf.smtpUser.c_str();     //smtp��¼�˺�
    info.pszSmtpPassword = tconf.smtpPwd.c_str();   //smtp��¼����

    //http��̨���շ���
    info.pszUrl = tconf.httpUrl.c_str();

    //��ϱ�־λ
    info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;  //��װ���п��õ��쳣����
    info.dwFlags |= CR_INST_AUTO_THREAD_HANDLERS;   //�Զ����߳��а�װ�쳣����
    info.dwFlags |= CR_INST_SEND_MANDATORY;         //ǿ�Ʒ��ͱ���(����ʾ�رհ�ť)
    info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;   //http����ʹ�ö�����

    if (tconf.isRestart) {
        info.dwFlags |= CR_INST_APP_RESTART;            //���������������
        info.nRestartTimeout = tconf.nRestartTimeout;   //������ʱʱ��(s)
    }

    //minidump�ļ�����
    info.uMiniDumpType = (MINIDUMP_TYPE)( // MiniDumpWithPrivateReadWriteMemory |
                             MiniDumpWithDataSegs |
                             MiniDumpWithHandleData |
                             // The following parameters are supported by 6.2 and later
                             MiniDumpWithIndirectlyReferencedMemory |
                             MiniDumpWithFullMemoryInfo |
                             MiniDumpWithThreadInfo |
                             MiniDumpWithUnloadedModules
                         );

    //��װ������
    int nResult = crInstall(&info);

    if (nResult != 0) {
        TCHAR szErrorMsg[512] = _T("");
        crGetLastErrorMsg(szErrorMsg, 512);
        return false;
    }

    //��Ļ��ͼ
    crAddScreenshot2(CR_AS_VIRTUAL_SCREEN | CR_AS_USE_JPEG_FORMAT, 95);
    //��Ļ¼��(��������Ӱ��,�ӳ���������ʼ�ͻ�¼��,���ս�ȡĩβ����)
    //crAddVideo(CR_AV_MAIN_WINDOW | CR_AV_QUALITY_LOW | CR_AV_NO_GUI, 1000 * 5, 1000, NULL, NULL);

    return true;
}

std::wstring CrashHelper::GetAppPath(void) {
    TCHAR szAppPath[MAX_PATH];
    GetModuleFileName(NULL, szAppPath, MAX_PATH);
    (_tcsrchr(szAppPath, '\\'))[1] = 0;
    return std::wstring(szAppPath);
}

std::wstring CrashHelper::GetAppName(void) {
    TCHAR szAppPath[MAX_PATH];
    GetModuleFileName(NULL, szAppPath, MAX_PATH);
    std::wstring strAppName(_tcsrchr(szAppPath, _T('\\')));
    return strAppName.substr(1, strAppName.rfind('.') - 1);
}

WORD CrashHelper::GetSmtpSecurity(const LPCTSTR &str) {
    if (0 == _tcscmp(str, _T("tls"))) {
        return CR_SMTP_SECURITY_TLS;
    } else if (0 == _tcscmp(str, _T("ssl"))) {
        return CR_SMTP_SECURITY_SSL;
    } else {
        return CR_SMTP_SECURITY_NO;
    }
}

bool CrashHelper::GetConfParams(const LPCTSTR &confName, TConfParams &tconf) {
    m_strAppPath = GetAppPath();
    m_strConfPath = confName;
    //WIN32_FIND_DATA FindFileData;

    //if (INVALID_HANDLE_VALUE == FindFirstFile(confName, &FindFileData)) {
        m_strConfPath = m_strAppPath + confName;
    //    if (INVALID_HANDLE_VALUE == FindFirstFile(m_strConfPath.c_str(), &FindFileData)) {
    //        return false;
    //    }
    //}

    tconf.isRestart = (0 == GetConfValue(_T("COMMON"), _T("restart")).compare(_T("true")));
    tconf.nRestartTimeout = _ttoi(GetConfValue(_T("COMMON"), _T("restarttimeout")).c_str());
    tconf.wSmtpSecurity = GetSmtpSecurity(GetConfValue(_T("SMTP"), _T("security")).c_str());
    tconf.dumpDir = m_strAppPath + GetConfValue(_T("COMMON"), _T("dumpdir"));
    tconf.icon = m_strAppPath + GetConfValue(_T("COMMON"), _T("icon"));
    tconf.privacy = GetConfValue(_T("COMMON"), _T("privacy"));
    tconf.smtpProxy = GetConfValue(_T("SMTP"), _T("proxy"));
    tconf.smtpUser = GetConfValue(_T("SMTP"), _T("user"));
    tconf.smtpPwd = GetConfValue(_T("SMTP"), _T("password"));
    tconf.httpUrl = GetConfValue(_T("HTTP"), _T("url"));
    tconf.emailSendto = GetConfValue(_T("EMAIL"), _T("sendto"));
    tconf.emailSubject = GetConfValue(_T("EMAIL"), _T("subject"));
    tconf.emailText = GetConfValue(_T("EMAIL"), _T("text"));
    return true;
}

inline std::wstring CrashHelper::GetConfValue(const LPCTSTR &location, const LPCTSTR &name) {
    TCHAR szValue[MAX_PATH];
    GetPrivateProfileString(location, name, _T(""), szValue, MAX_PATH, m_strConfPath.c_str());
    return std::wstring(szValue);
}
