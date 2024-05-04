#include "Renderer2D.h"
#include "d3dx12.h"
#include <d3dcompiler.h>

Renderer2D::Renderer2D(HWND hwnd)
	:
	DeviceResources(hwnd)
{
	ComPtr<ID3D12Resource> uploadBuffer;

	NOT_SUCCEEDED(CommandAllocator->Reset());
	NOT_SUCCEEDED(CommandList->Reset(CommandAllocator.Get(), nullptr));
	CompileShaders();
	CreateRootSignature();
	CreateLocalPipeline();
	//CreateTexture(&uploadBuffer);
	NOT_SUCCEEDED(CommandList->Close());
	ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	this->Synchronize();
}



void Renderer2D::StartRecording()
{
	float color[4] = {0.3, 0.3, 0.7, 1 };
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE depth_hanlde = CurrentDepthBufferView();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = renderTargets[current_frame].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	NOT_SUCCEEDED(CommandAllocator->Reset());
	NOT_SUCCEEDED(CommandList->Reset(CommandAllocator.Get(), graphicsPipeline.Get()));

	CommandList->RSSetViewports(1, &ViewPort);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->ResourceBarrier(1, &barrier);
	CommandList->ClearRenderTargetView(rtv_handle, color, 0, nullptr);
	CommandList->ClearDepthStencilView(depth_hanlde, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1, 0, 0, nullptr);
	CommandList->OMSetRenderTargets(1, &rtv_handle, true, &depth_hanlde);
	CommandList->SetGraphicsRootSignature(rootSignature.Get());
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->SetGraphicsRootConstantBufferView((int)RootSignatureEntry::WorldTransform, worldTransform);
	//CommandList->SetDescriptorHeaps(1, textureHeap.GetAddressOf());
	//CommandList->SetGraphicsRootDescriptorTable(2, textureHeap->GetGPUDescriptorHandleForHeapStart());
}




void Renderer2D::StopRecording()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = renderTargets[current_frame].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CommandList->ResourceBarrier(1, &barrier);
	CommandList->Close();

	ID3D12CommandList* lists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(1, lists);

	SwapChain->Present(0, 0);
	current_frame = SwapChain->GetCurrentBackBufferIndex();
	Synchronize();
}

void Renderer2D::BindCameraBuffer(D3D12_GPU_VIRTUAL_ADDRESS worldTransformAddres)
{
	worldTransform = worldTransformAddres;
}

void Renderer2D::RenderGraphicalObject(Entity& obj)
{
	CommandList->SetGraphicsRootConstantBufferView((int)RootSignatureEntry::ConstantBuffer, obj.getConstantBufferVirtualAddress());
	CommandList->IASetVertexBuffers(0, 1, obj.getVertexBufferView());
	CommandList->IASetIndexBuffer(obj.getIndexBufferView());
	CommandList->DrawIndexedInstanced(INDEX_COUNT, 1, 0, 0, 0);
}

void Renderer2D::RenderGraphicalObjects(Entity* arrayObj, int num)
{
	for (int i = 0; i < num; i++)
	{
		Entity& obj = arrayObj[i];

		CommandList->SetGraphicsRootConstantBufferView((int)RootSignatureEntry::ConstantBuffer, obj.getConstantBufferVirtualAddress());
		CommandList->IASetVertexBuffers(0, 1, obj.getVertexBufferView());
		CommandList->IASetIndexBuffer(obj.getIndexBufferView());
		CommandList->DrawIndexedInstanced(INDEX_COUNT, 1, 0, 0, 0);
	}
}

void Renderer2D::Resize(HWND hwnd, void* renderer)
{
	Renderer2D* renderer2D = (Renderer2D*)renderer;
	renderer2D->DeviceResources::Resize(hwnd);
}

void Renderer2D::CompileShaders()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	CompileShader(&vs_shaderBlob,L"src//shaders//shaders.hlsl", NULL, NULL, "VS", "vs_5_0", compileFlags, 0);

	CompileShader(&ps_shaderBlob, L"src//shaders//shaders.hlsl", NULL, NULL, "PS", "ps_5_0", compileFlags, 0);

}

void Renderer2D::CreateRootSignature()
{

	CD3DX12_DESCRIPTOR_RANGE1 texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootSlots[3];
	rootSlots[(int)RootSignatureEntry::WorldTransform ].InitAsConstantBufferView(0);
	rootSlots[(int)RootSignatureEntry::ConstantBuffer].InitAsConstantBufferView(1);
	rootSlots[(int)RootSignatureEntry::Texture].InitAsDescriptorTable(1, &texTable);

	CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootSlots), rootSlots, 1, &pointWrap,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	NOT_SUCCEEDED(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
	if (error.Get() != nullptr)
	{
		MessageBox(NULL, L"ROOT SIGNATURE ERROR", NULL, MB_OK);
		exit(-1);
	}
	NOT_SUCCEEDED(Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void Renderer2D::CreateLocalPipeline()
{
	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vs_shaderBlob->GetBufferPointer(), vs_shaderBlob->GetBufferSize() };
	psoDesc.PS = { ps_shaderBlob->GetBufferPointer(), ps_shaderBlob->GetBufferSize() };
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.AntialiasedLineEnable = TRUE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = DsvFormat;
	NOT_SUCCEEDED(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&graphicsPipeline)));
}

/*
void Renderer2D::UpdateWorldTransform()
{

	XMMATRIX transformation = XMMatrixScaling(1/AspectRatio(), 1, 1);
	transformation = XMMatrixTranspose(transformation);
	XMFLOAT4X4 transformationMatrix;
	XMStoreFloat4x4(&transformationMatrix, transformation);
	//memcpy(wtMap, &transformationMatrix, sizeof(XMFLOAT4X4));

	float* ptr = (float*)wtMap;
	for (int i = 0; i < 16; i++)
	{
		ptr[i] = ((float*)&transformationMatrix)[i];
	}
}
*/
