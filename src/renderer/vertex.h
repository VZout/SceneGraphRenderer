#pragma once

#include "math\vec.hpp"
#include <functional>
#include <d3d12.h>

namespace rlr {

	struct Vertex {
		fm::vec3 m_pos;
		fm::vec3 m_normal;
		fm::vec2 m_texCoord;
		//fm::vec3 m_tangent;
		//fm::vec3 m_bitangent;
		float weight[4] = { 0, 0, 0, 0 };
		uint32_t id[4] = { 0, 0, 0, 0};

#ifdef USE_D3D12
	static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
		std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, m_pos),      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, m_normal),   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, m_texCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(Vertex, m_tangent),   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "TANGENT", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(Vertex, m_bitangent),   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, weight), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(Vertex, id),   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		};

		return layout;
	}
#endif

	bool operator==(const Vertex& other) const {
		return m_pos == other.m_pos && m_normal == other.m_normal && m_texCoord == other.m_texCoord;
	}
};

} /* fr */

namespace std {
	template<> struct hash<fm::vec3> {
		size_t operator()(fm::vec3 const& v) const {
			return ((hash<float>()(v.x) ^
				(hash<float>()(v.y) << 1)) >> 1) ^
				(hash<float>()(v.z) << 1);
		}
	};

	template<> struct hash<fm::vec2> {
		size_t operator()(fm::vec2 const& v) const {
			return ((hash<float>()(v.x) ^
				(hash<float>()(v.y) << 1)) >> 1);
		}
	};

	template<> struct hash<rlr::Vertex> {
		size_t operator()(rlr::Vertex const& vertex) const {
			return ((hash<fm::vec3>()(vertex.m_pos) ^
				(hash<fm::vec3>()(vertex.m_normal) << 1)) >> 1) ^
				(hash<fm::vec2>()(vertex.m_texCoord) << 1);
		}
	};
}
