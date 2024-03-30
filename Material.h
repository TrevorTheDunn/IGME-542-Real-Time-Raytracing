#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>

enum MaterialType {
	Normal,
	Refractive
};

#pragma once
class Material
{
public:
	Material(
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState,
		DirectX::XMFLOAT3 colorTint,
		MaterialType type = MaterialType::Normal,
		float roughness = 1.0f,
		DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
		DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));

	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot);

	void FinalizeMaterial();

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();
	D3D12_GPU_DESCRIPTOR_HANDLE GetFinalGPUHandleForSRVs();
	MaterialType GetType();
	float GetRoughness();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 colorTint);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 uvOffset);
	void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState);
	void SetType(MaterialType type);
	void SetRoughness(float roughness);

private:
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	bool finalized;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[4];
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;

	MaterialType type;
	float roughness;
};