// tests_g.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gtest/gtest.h"
#include "service_tests.h"
#include "db_tests.h"
#include "slog_app_tests.h"

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

//#define MAX_PATH 512

std::wstring get_service_path() {
    wchar_t szPath[MAX_PATH] = {0};

    if(!GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        wprintf(_T("Cannot get service path! (%d)\n"), GetLastError());
        return 0;
    }

    std::wstring sPath(szPath);
    int iPos = std::string::npos;
    iPos = sPath.find_last_of('\\');
    if(iPos != std::string::npos) {
        sPath.erase(iPos+1);
        sPath.append(L"slogService.exe");
        return sPath;
    }
    return 0;
}

bool do_service(const wchar_t* action) {

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    wchar_t cmdLine[1024] = {0};

    wsprintf(cmdLine, L"%s -%s\0", get_service_path().c_str(), action);

    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
        cmdLine,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        wprintf( L"CreateProcess for install service failed (%d).\n", GetLastError() );
        return false;
    }
    return true;
}


TEST(install_service, install_srv){
    EXPECT_TRUE(do_service(L"install"));
}

TEST(service_exist) {
    EXPECT_TRUE(service_exist());
}

TEST(slog_app_started) {
    EXPECT_TRUE(slog_app_started());
}

TEST(slog_app_restarted) {

    EXPECT_TRUE(slog_app_restarted());
}

TEST(database_init){
    EXPECT_TRUE(db_init());
}

TEST(crate_database_tables){
    EXPECT_TRUE(create_tables());
}

TEST(skype_api_run) {
    EXPECT_TRUE(skype_api_test_run());
}

TEST(populate_users_table) {
    EXPECT_TRUE(populate_users());
}

TEST(skype_api_stop) {
    EXPECT_TRUE(skype_api_test_stop());
}

TEST(remove_service){
    EXPECT_TRUE(do_service(L"remove"));
}


int _tmain(int argc, _TCHAR* argv[])
{
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

