#pragma once
//#include <wchar.h>
#include <d3d11.h>
#include <exception>
#include <stdio.h>
//#include <cstddef>
//#include <cstdint>
//#include <exception>
#include <fstream>
//#include <stdexcept>
//#include <system_error>
//#include <vector>
#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;

template <typename T>
void SAFE_RELEASE(T* p)
{
    if (p)
    {
        p->Release();
        p = nullptr;
    }
}

template <typename T>
void SAFE_DELETE(T* p)
{
    if (p)
    {
        delete p;
        p = nullptr;
    }
}


LPCWSTR GetComErrorString(HRESULT hr);





// Helper class for COM exceptions
class com_exception : public std::exception
{
public:
    com_exception(HRESULT hr) : result(hr) {}

    const char* what() const noexcept override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X",
            static_cast<unsigned int>(result));
        return s_str;
    }

private:
    HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void HR_T(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw com_exception(hr);
    }
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, D3D10_SHADER_MACRO macros[]);

HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** textureView);