#include "DeviceResources.h"
#include "../GraphicalObject.h"

#pragma once
class Renderer2D : public DeviceResources
{
public:
	Renderer2D(HWND hwnd);
	void StartRecording();
	void StopRecording();
	void RenderGraphicalObject(GraphicalObject& obj);
	static void Resize(HWND hwnd, void* renderer);
protected:
	void CompileShaders();
	void CreateRootSignature();
	void CreateLocalPipeline();
	//void CreateTexture(ID3D12Resource** uploadBuffer);
private:
	ComPtr<ID3D12PipelineState> graphicsPipeline;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3DBlob> vs_shaderBlob;
	ComPtr<ID3DBlob> ps_shaderBlob;
	ComPtr<ID3D12Resource> test_vb;
	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12DescriptorHeap> textureHeap;
	unsigned char* TextureData;
	D3D12_RESOURCE_BARRIER transitionTable[4];
	D3D12_TEXTURE_COPY_LOCATION textureLocation;
	D3D12_TEXTURE_COPY_LOCATION bufferLocation;
};

