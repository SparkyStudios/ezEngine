#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Allocator.h>

#include <spark/include/sqrat.h>
#include <spark/include/squirrel.h>

// clang-format off
EZ_IMPLEMENT_SINGLETON(ezSparkLangAllocator);
// clang-format on

ezSparkLangAllocator::ezSparkLangAllocator()
  : ezAllocator("SparkLangAllocator")
  , m_SingletonRegistrar(this)
{
}

void sq_vm_init_alloc_context(SQAllocContext* pContext)
{
  *pContext = new SQAllocContextT();

  if (auto* const pAllocator = ezSparkLangAllocator::GetSingleton(); pAllocator == nullptr)
  {
    (*pContext)->m_pAllocator = new ezSparkLangAllocator();
    (*pContext)->m_bIsOwned = true;
  }
  else
  {
    (*pContext)->m_pAllocator = pAllocator;
    (*pContext)->m_bIsOwned = false;
  }
}

void sq_vm_destroy_alloc_context(SQAllocContext* pContext)
{
  if ((*pContext)->m_bIsOwned)
  {
    delete (*pContext)->m_pAllocator;
  }

  (*pContext)->m_pAllocator = nullptr;
  delete *pContext;
}

void* sq_vm_malloc(SQAllocContext pContext, SQUnsignedInteger size)
{
  return pContext->m_pAllocator->Allocate(size, SQ_ALIGNMENT);
}

void* sq_vm_realloc(SQAllocContext pContext, void* p, SQUnsignedInteger oldSize, SQUnsignedInteger size)
{
  if (p == nullptr)
    return pContext->m_pAllocator->Allocate(size, SQ_ALIGNMENT);

  return pContext->m_pAllocator->Reallocate(p, oldSize, size, SQ_ALIGNMENT);
}

void sq_vm_free(SQAllocContext pContext, void* p, SQUnsignedInteger size)
{
  pContext->m_pAllocator->Deallocate(p);
}
