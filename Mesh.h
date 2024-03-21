#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <string>

#include "Vertex.h"

struct MeshRaytracingData
{
	D3D12_GPU_DESCRIPTOR_HANDLE IndexbufferSRV { };
	D3D12_GPU_DESCRIPTOR_HANDLE VertexBufferSRV { };
	Microsoft::WRL::ComPtr<ID3D12Resource> BLAS;
	unsigned int HitGroupIndex = 0;
};

class Mesh
{
public:
	Mesh(Vertex* vertArray, size_t numVerts, unsigned int* indexArray, size_t numIndices);
	Mesh(const std::wstring& objFile);
	~Mesh();

	// Getters for mesh data
	D3D12_VERTEX_BUFFER_VIEW GetVBView();
	D3D12_INDEX_BUFFER_VIEW GetIBView();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount() { return numVertices; }

	Microsoft::WRL::ComPtr<ID3D12Resource> GetVBResource() { return vb; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIBResource() { return ib; }

	MeshRaytracingData GetRaytracingData() { return raytracingData; }

private:
	// D3D buffers
	Microsoft::WRL::ComPtr<ID3D12Resource> vb;
	Microsoft::WRL::ComPtr<ID3D12Resource> ib;

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};

	// Total indices in this mesh
	unsigned int numIndices;
	unsigned int numVertices;

	// Helper for creating buffers (in the event we add more constructor overloads)
	void CreateBuffers(Vertex* vertArray, size_t numVerts, unsigned int* indexArray, size_t numIndices);
	void CalculateTangents(Vertex* verts, size_t numVerts, unsigned int* indices, size_t numIndices);

	MeshRaytracingData raytracingData;
};

