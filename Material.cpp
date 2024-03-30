#include "Material.h"
#include "DX12Helper.h"

Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, 
	DirectX::XMFLOAT3 colorTint, MaterialType type, float roughness, DirectX::XMFLOAT2 uvScale, 
	DirectX::XMFLOAT2 uvOffset) : pipelineState(pipelineState),
	colorTint(colorTint), type(type), roughness(roughness), uvScale(uvScale), uvOffset(uvOffset),
	finalized(false)
{
	finalGPUHandleForSRVs = {};
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot)
{
	if (finalized || slot < 0 || slot >= 4) return;

	textureSRVsBySlot[slot] = srv;
}

void Material::FinalizeMaterial()
{
	if (finalized) return;

	DX12Helper& dx12Helper = DX12Helper::GetInstance();

	for (int i = 0; i < 4; i++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
			dx12Helper.CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(textureSRVsBySlot[i], 1);

		if (i == 0) { finalGPUHandleForSRVs = gpuHandle; }
	}

	finalized = true;
}

// Getters
DirectX::XMFLOAT3 Material::GetColorTint() { return colorTint; }
DirectX::XMFLOAT2 Material::GetUVScale() { return uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return uvOffset; }
Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState() { return pipelineState; }
D3D12_GPU_DESCRIPTOR_HANDLE Material::GetFinalGPUHandleForSRVs() { return finalGPUHandleForSRVs; }
MaterialType Material::GetType() { return type; }
float Material::GetRoughness() { return roughness; }

// Setters
void Material::SetColorTint(DirectX::XMFLOAT3 colorTint) { this->colorTint = colorTint; }
void Material::SetUVScale(DirectX::XMFLOAT2 uvScale) { this->uvScale = uvScale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset) { this->uvOffset = uvOffset; }
void Material::SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState) { this->pipelineState = pipelineState; }
void Material::SetType(MaterialType type) { this->type = type; }
void Material::SetRoughness(float roughness) { this->roughness = roughness; }