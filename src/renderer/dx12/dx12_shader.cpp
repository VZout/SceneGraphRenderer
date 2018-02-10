#include "dx12_shader.h"

#include "../interface.h"

#include <D3Dcompiler.h>

namespace rlr {

// TODO: move this to a internal or util file.
namespace internal
{
	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

} /* internal */

void Load(Shader** shader, ShaderType type, std::string const & path) {
	Shader* new_shader = new Shader();

	std::string shader_target;
	switch (type) {
	case ShaderType::VERTEX_SHADER: shader_target = "vs_5_0";
		break;
	case ShaderType::PIXEL_SHADER: shader_target = "ps_5_0";
		break;
	case ShaderType::DOMAIN_SHADER: shader_target = "ds_5_0";
		break;
	case ShaderType::GEOMETRY_SHADER: shader_target = "gs_5_0";
		break;
	case ShaderType::HULL_SHADER: shader_target = "hs_5_0";
		break;
	case ShaderType::DIRECT_COMPUTE_SHADER: shader_target = "cs_5_0";
		break;
	}

	ID3DBlob* error;
	std::wstring wpath = internal::s2ws(path);
	HRESULT hr = D3DCompileFromFile(wpath.c_str(),
		nullptr,
		nullptr,
		"main",
		shader_target.c_str(),
		0,
		0,
		&new_shader->native,
		&error);
	if (FAILED(hr)) {
		throw((char*)error->GetBufferPointer());
	}

	(*shader) = new_shader;
}

void Destroy(Shader* shader) {
	shader->native->Release();
	delete shader;
	shader = 0;
}

} /* rlr */
