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

//回调示例
//int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO *pInfo) {
//    switch (pInfo->nStage) {
//    case CR_CB_STAGE_PREPARE:   //异常捕获前
//        //处理事务(如关闭日志以解除文件占用)
//        break;
//
//    case CR_CB_STAGE_FINISH:    //异常捕获结束
//        break;
//    }
//
//    //进入下一阶段
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

    //参数设定
    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);

    //公共参数
    info.pszAppName = strAppName.c_str();
    info.pszAppVersion = _T("1.0.0.0");
    info.pszErrorReportSaveDir = tconf.dumpDir.c_str();
    info.pszCustomSenderIcon = tconf.icon.c_str();
    info.pszPrivacyPolicyURL = tconf.privacy.c_str();  //隐私策略url

    //设置邮件内容
    info.pszEmailTo = tconf.emailSendto.c_str();
    info.pszEmailSubject = tconf.emailSubject.c_str();
    info.pszEmailText = tconf.emailText.c_str();

    //发送优先级
    info.uPriorities[CR_HTTP] = 3;  //优先使用htpp
    info.uPriorities[CR_SMTP] = 2;  //其次使用smtp
    info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; //不使用第三方邮件客户端

    //smtp服务配置
    info.wSmtpSecurity = tconf.wSmtpSecurity;       //smtp加密方式
    info.pszSmtpProxy = tconf.smtpProxy.c_str();    //smtp地址端口(mail.example.com:25)
    info.pszSmtpLogin = tconf.smtpUser.c_str();     //smtp登录账号
    info.pszSmtpPassword = tconf.smtpPwd.c_str();   //smtp登录密码

    //http后台接收服务
    info.pszUrl = tconf.httpUrl.c_str();

    //组合标志位
    info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;  //安装所有可用的异常处理
    info.dwFlags |= CR_INST_AUTO_THREAD_HANDLERS;   //自动在线程中安装异常处理
    info.dwFlags |= CR_INST_SEND_MANDATORY;         //强制发送报告(不显示关闭按钮)
    info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;   //http发送使用二进制

    if (tconf.isRestart) {
        info.dwFlags |= CR_INST_APP_RESTART;            //允许程序重新启动
        info.nRestartTimeout = tconf.nRestartTimeout;   //重启超时时间(s)
    }

    //minidump文件参数
    info.uMiniDumpType = (MINIDUMP_TYPE)( // MiniDumpWithPrivateReadWriteMemory |
                             MiniDumpWithDataSegs |
                             MiniDumpWithHandleData |
                             // The following parameters are supported by 6.2 and later
                             MiniDumpWithIndirectlyReferencedMemory |
                             MiniDumpWithFullMemoryInfo |
                             MiniDumpWithThreadInfo |
                             MiniDumpWithUnloadedModules
                         );

    //安装捕获器
    int nResult = crInstall(&info);

    if (nResult != 0) {
        TCHAR szErrorMsg[512] = _T("");
        crGetLastErrorMsg(szErrorMsg, 512);
        return false;
    }

    //屏幕截图
    crAddScreenshot2(CR_AS_VIRTUAL_SCREEN | CR_AS_USE_JPEG_FORMAT, 95);
    //屏幕录制(对性能有影响,从程序启动开始就会录制,最终截取末尾部分)
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
