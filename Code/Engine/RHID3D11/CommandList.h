#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/CommandList.h>

class spCommandListD3D11 : public spCommandList
{
public:
  void Reset() {}
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spCommandListD3D11);
