#include <TestFramework/TestFrameworkPCH.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
#  include <TestFramework/Framework/uwp/uwpTestApplication.h>
#  include <TestFramework/Framework/uwp/uwpTestFramework.h>

#  include <Foundation/Logging/Log.h>

ezUwpTestFramework::ezUwpTestFramework(const char* szTestName, const char* szAbsTestDir, const char* szRelTestDataDir, int argc, const char** argv)
  : ezTestFramework(szTestName, szAbsTestDir, szRelTestDataDir, argc, argv)
{
}

ezUwpTestFramework::~ezUwpTestFramework()
{
  RoUninitialize();
}

ezTestAppRun ezUwpTestFramework::RunTests()
{
  ComPtr<ABI::Windows::ApplicationModel::Core::ICoreApplication> coreApplication;
  HRESULT result = ABI::Windows::Foundation::GetActivationFactory(
    HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(), &coreApplication);

  if (FAILED(result))
  {
    std::cout << "Failed to create core application." << std::endl;
  }
  else
  {
    ComPtr<ezUwpTestApplication> application = Make<ezUwpTestApplication>(*this);
    coreApplication->Run(application.Get());
    application.Detach(); // Was already deleted by uwp.
  }

  return ezTestAppRun::Quit;
}

#endif
