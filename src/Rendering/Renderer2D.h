#include "DeviceResources.h"

#pragma once

enum class RootSignatureEntry
{
	WorldTransform,
	ConstantBuffer,
	Texture,
	Size
};



class Renderer2D : public DeviceResources
{
public:
	Renderer2D(HWND hwnd);
	void StartRecording();
	void StopRecording();
	void BindCameraBuffer(D3D12_GPU_VIRTUAL_ADDRESS worldTransformAddres);
	void RenderGraphicalObjects(D3D12_GPU_VIRTUAL_ADDRESS* constBufferTable, 
						D3D12_VERTEX_BUFFER_VIEW** vbViewTable, D3D12_INDEX_BUFFER_VIEW** ibViewTable, int num, UINT indexCount);

	static void Resize(HWND hwnd, void* renderer);
protected:
	void CompileShaders();
	void CreateRootSignature();
	void CreateLocalPipeline();
	void CreateTexture(ID3D12Resource** uploadBuffer, std::wstring path);
private:
	UINT* wtMap;
	ComPtr<ID3D12PipelineState> graphicsPipeline;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3DBlob> vs_shaderBlob;
	ComPtr<ID3DBlob> ps_shaderBlob;
	D3D12_GPU_VIRTUAL_ADDRESS worldTransform;
	ComPtr<ID3D12Resource> test_vb;
	D3D12_RESOURCE_BARRIER transitionTable[4];


	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12DescriptorHeap> textureHeap;
};

