#pragma once

#include <EditorTest/EditorTestPCH.h>

#include "../TestClass/TestClass.h"

class ezDocument;

class ezEditorTestGenerateCompile : public ezEditorTest
{
public:
  using SUPER = ezEditorTest;

  virtual const char* GetTestName() const override;

private:
  enum SubTests
  {
    GenerateAndCompile,
  };

  virtual void SetupSubTests() override;
  virtual ezResult InitializeTest() override;
  virtual ezResult DeInitializeTest() override;
  virtual ezTestAppRun RunSubTest(ezInt32 iIdentifier, ezUInt32 uiInvocationCount) override;

  virtual ezResult InitializeSubTest(ezInt32 iIdentifier) override;
  virtual ezResult DeInitializeSubTest(ezInt32 iIdentifier) override;

  ezDocument* m_pDocument = nullptr;
};
