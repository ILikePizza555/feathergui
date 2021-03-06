// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "fgDirect2D.h"

#include <d2d1effecthelpers.h>
#include <initguid.h>
#include "fgCircle.h"
#include "fgDirect2D_fgCircle.h"
#include "bss-util/defines.h"
#include <utility>
#include <memory>

#define XML(x) TEXT(#x)

fgCircle::fgCircle() {}
fgCircle::~fgCircle()
{
}
IFACEMETHODIMP fgCircle::Initialize(_In_ ID2D1EffectContext* pContextInternal, _In_ ID2D1TransformGraph* pTransformGraph)
{
  HRESULT hr = pContextInternal->LoadPixelShader(CLSID_fgCirclePixelShader, fgCircle_main, sizeof(fgCircle_main));

  if(SUCCEEDED(hr))
    hr = pTransformGraph->SetSingleTransformNode(this);

  return hr;
}
HRESULT fgCircle::Register(_In_ ID2D1Factory1* pFactory)
{
  PCWSTR pszXml =
    XML(
      <?xml version='1.0'?>
      <Effect>
        <!-- System Properties -->
        <Property name = 'DisplayName' type = 'string' value = 'Circle' />
        <Property name = 'Author' type = 'string' value = 'Black Sphere Studios' />
        <Property name = 'Category' type = 'string' value = 'Vector' />
        <Property name = 'Description' type = 'string' value = 'Draws a circle with an outline.' />
        <Inputs>
        </Inputs>
        <Property name='Rect' type='vector4'>
            <Property name='DisplayName' type='string' value='Output Rect'/>
            <Property name='Default' type='vector4' value='(0.0, 0.0, 0.0, 0.0)' />
        </Property>
        <Property name='Arcs' type='vector4'>
            <Property name='DisplayName' type='string' value='Arcs'/>
            <Property name='Default' type='vector4' value='(0.0, 0.0, 0.0, 0.0)' />
        </Property>
        <Property name='Color' type='uint32'>
            <Property name='DisplayName' type='string' value='Color'/>
            <Property name='Default' type='uint32' value='0' />
        </Property>
        <Property name='OutlineColor' type='uint32'>
            <Property name='DisplayName' type='string' value='Outline Color'/>
            <Property name='Default' type='uint32' value='0' />
        </Property>
        <Property name='Outline' type='float'>
            <Property name='DisplayName' type='string' value='Outline Width'/>
            <Property name='Min' type='float' value='0.0' />
            <Property name='Default' type='float' value='0.0' />
        </Property>
      </Effect>
      );

  // This defines the bindings from specific properties to the callback functions
  // on the class that ID2D1Effect::SetValue() & GetValue() will call.
  const D2D1_PROPERTY_BINDING bindings[] =
  {
    D2D1_VALUE_TYPE_BINDING(L"Rect", &SetRect, &GetRect),
    D2D1_VALUE_TYPE_BINDING(L"Arcs", &SetCorners, &GetCorners),
    D2D1_VALUE_TYPE_BINDING(L"Color", &SetColor, &GetColor),
    D2D1_VALUE_TYPE_BINDING(L"OutlineColor", &SetOutlineColor, &GetOutlineColor),
    D2D1_VALUE_TYPE_BINDING(L"Outline", &SetOutline, &GetOutline),
  };

  return pFactory->RegisterEffectFromString(
    CLSID_fgCircle,
    pszXml,
    bindings,
    ARRAYSIZE(bindings),
    CreateEffect
  );
}

HRESULT __stdcall fgCircle::CreateEffect(_Outptr_ IUnknown** ppEffectImpl)
{
  // This code assumes that the effect class initializes its reference count to 1.
  *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new fgCircle());

  if(*ppEffectImpl == nullptr)
    return E_OUTOFMEMORY;
  return S_OK;
}

IFACEMETHODIMP fgCircle::SetDrawInfo(_In_ ID2D1DrawInfo* pRenderInfo)
{
  _drawInfo = pRenderInfo;
  _drawInfo->AddRef();
  return _drawInfo->SetPixelShader(CLSID_fgCirclePixelShader);
}