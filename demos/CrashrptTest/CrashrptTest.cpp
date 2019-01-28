// CrashrptTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO *pInfo) {
    switch (pInfo->nStage) {
    case CR_CB_STAGE_PREPARE:   //异常捕获前
        //处理事务(如关闭日志以解除文件占用)
        break;

    case CR_CB_STAGE_FINISH:    //异常捕获结束
        break;
    }

    //进入下一阶段
    return CR_CB_NOTIFY_NEXT_STAGE;
    //return CR_CB_DODEFAULT;
}

int main() {
    bool bRet = CrashHelper::Initialize(L"crashrpt.conf");

    if (bRet) {
        CrashHelper::SetCrashCallback(CrashCallback, NULL);
        system("pause");

        //模拟崩溃
        int *p = 0;
        *p = 0;
    } else {
        std::cout << "CrashHelper::Initialize Failed" << std::endl;
    }

    system("pause");
}
