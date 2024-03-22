#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"

#include "BufferStructs.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "DX12Helper.h"
#include "Material.h"

#include "RaytracingHelper.h"

// For the DirectX Math library
using namespace DirectX;

#define RandomRange(min, max) (float)rand() / RAND_MAX * (max - min) + min

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true),				// Show extra stats (fps) in title bar?
	ibView{},
	vbView{}
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// We need to wait here until the GPU
	// is actually done with its work
	DX12Helper::GetInstance().WaitForGPU();

	delete& RaytracingHelper::GetInstance();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Attempt to initialize DXR
	RaytracingHelper::GetInstance().Initialize(
		windowWidth,
		windowHeight,
		device,
		commandQueue,
		commandList,
		FixPath(L"Raytracing.cso"));

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateRootSigAndPipelineState();
	CreateBasicGeometry();

	camera = std::make_shared<Camera>(
		XMFLOAT3(0.0f, 0.0f, -20.0f),
		5.0f,
		0.004f,
		XM_PIDIV4,
		windowWidth / (float)windowHeight);

	XMFLOAT3 white = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMFLOAT3 red = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 green = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 blue = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 teal = XMFLOAT3(0.0f, 0.5f, 0.5f);
	XMFLOAT3 purple = XMFLOAT3(0.5f, 0.0f, 0.5f);

	//Set up lights
	Light directional1 = {};
	directional1.Type = LIGHT_TYPE_DIRECTIONAL;
	directional1.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//directional1.Color = XMFLOAT3(0.4f, 0.2f, 0.8f);
	directional1.Color = white;
	directional1.Intensity = 1.0f;

	Light directional2 = {};
	directional2.Type = LIGHT_TYPE_DIRECTIONAL;
	directional2.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	//directional2.Color = XMFLOAT3(1.0f, 0.0f, 0.5f);
	directional2.Color = red;
	directional2.Intensity = 1.0f;

	Light directional3 = {};
	directional3.Type = LIGHT_TYPE_DIRECTIONAL;
	directional3.Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
	//directional3.Color = XMFLOAT3(1.0f, 0.0f, 0.5f);
	directional3.Color = green;
	directional3.Intensity = 2.0f;

	Light directional4 = {};
	directional4.Type = LIGHT_TYPE_DIRECTIONAL;
	directional4.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	//directional4.Color = XMFLOAT3(1.0f, 0.0f, 0.5f);
	directional4.Color = blue;
	directional4.Intensity = 2.0f;

	Light point1 = {};
	point1.Type = LIGHT_TYPE_POINT;
	point1.Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	//point1.Color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	point1.Color = teal;
	point1.Range = 5.0f;
	point1.Intensity = 1.5f;

	Light point2 = {};
	point2.Type = LIGHT_TYPE_POINT;
	point2.Position = XMFLOAT3(0.0f, 0.0f, -1.0f);
	//point2.Color = XMFLOAT3(0.0f, 0.5f, 1.0f);
	point2.Color = purple;
	point2.Range = 7.5f;
	point2.Intensity = 1.0f;

	Light point3 = {};
	point3.Type = LIGHT_TYPE_POINT;
	point3.Position = XMFLOAT3(3.0f, -1.0f, 0.0f);
	//point3.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	point3.Color = white;
	point3.Range = 5.0f;
	point3.Intensity = 1.0f;

	Light point4 = {};
	point4.Type = LIGHT_TYPE_POINT;
	point4.Position = XMFLOAT3(10.0f, -1.0f, 0.0f);
	//point4.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	point4.Color = red;
	point4.Range = 7.5f;
	point4.Intensity = 3.0f;

	lights.push_back(directional1);
	lights.push_back(directional2);
	lights.push_back(point1);
	lights.push_back(point2);
	lights.push_back(directional3);
	lights.push_back(point3);
	lights.push_back(point4);
	lights.push_back(directional4);

	lightCount = 8;

	commandList->Close();
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - creates multiple entities using loaded meshes
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Loads in all the mesh files and creates entities with each, adding it to the entity list
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str());
	std::shared_ptr<Mesh> quadDSMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str());

	//-- Load Textures --

	//Bronze Texture
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/bronze_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/bronze_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str());

	//Cobblestone Texture
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/cobblestone_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str());

	//Floor Texture
	D3D12_CPU_DESCRIPTOR_HANDLE floorAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/floor_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE floorMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/floor_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE floorNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/floor_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE floorRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/floor_roughness.png").c_str());

	//Paint Texture
	D3D12_CPU_DESCRIPTOR_HANDLE paintAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/paint_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE paintMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/paint_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE paintNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/paint_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE paintRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/paint_roughness.png").c_str());

	//Rough Texture
	D3D12_CPU_DESCRIPTOR_HANDLE roughAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/rough_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE roughMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/rough_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE roughNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/rough_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE roughRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/rough_roughness.png").c_str());

	//Scratched Texture
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/scratched_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/scratched_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str());

	//Wood Texture
	D3D12_CPU_DESCRIPTOR_HANDLE woodAlbedo = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/wood_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE woodMetal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/wood_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE woodNormal = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/wood_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE woodRoughness = DX12Helper::GetInstance().LoadTexture(FixPath(L"../../Assets/Textures/wood_roughness.png").c_str());

	// -- Create Materials --

	//Bronze Material
	std::shared_ptr<Material> bronze = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1), XMFLOAT2(1,1), XMFLOAT2(0,0));
	bronze->AddTexture(bronzeAlbedo, 0);
	bronze->AddTexture(bronzeNormal, 1);
	bronze->AddTexture(bronzeRoughness, 2);
	bronze->AddTexture(bronzeMetal, 3);
	bronze->FinalizeMaterial();

	//Cobblestone Material
	std::shared_ptr<Material> cobblestone = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1), XMFLOAT2(0, 0));
	cobblestone->AddTexture(cobblestoneAlbedo, 0);
	cobblestone->AddTexture(cobblestoneNormal, 1);
	cobblestone->AddTexture(cobblestoneRoughness, 2);
	cobblestone->AddTexture(cobblestoneMetal, 3);
	cobblestone->FinalizeMaterial();

	//Floor Material
	std::shared_ptr<Material> floor = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1));
	floor->AddTexture(floorAlbedo, 0);
	floor->AddTexture(floorNormal, 1);
	floor->AddTexture(floorRoughness, 2);
	floor->AddTexture(floorMetal, 3);
	floor->FinalizeMaterial();

	//Paint Material
	std::shared_ptr<Material> paint = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1), XMFLOAT2(0, 0));
	paint->AddTexture(paintAlbedo, 0);
	paint->AddTexture(paintNormal, 1);
	paint->AddTexture(paintRoughness, 2);
	paint->AddTexture(paintMetal, 3);
	paint->FinalizeMaterial();

	//Rough Material
	std::shared_ptr<Material> rough = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1), XMFLOAT2(0, 0));
	rough->AddTexture(roughAlbedo, 0);
	rough->AddTexture(roughNormal, 1);
	rough->AddTexture(roughRoughness, 2);
	rough->AddTexture(roughMetal, 3);
	rough->FinalizeMaterial();

	//Scratched Material
	std::shared_ptr<Material> scratched = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1), XMFLOAT2(0, 0));
	scratched->AddTexture(scratchedAlbedo, 0);
	scratched->AddTexture(scratchedNormal, 1);
	scratched->AddTexture(scratchedRoughness, 2);
	scratched->AddTexture(scratchedMetal, 3);
	scratched->FinalizeMaterial();

	//Wood Material
	std::shared_ptr<Material> wood = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1), XMFLOAT2(0, 0));
	wood->AddTexture(woodAlbedo, 0);
	wood->AddTexture(woodNormal, 1);
	wood->AddTexture(woodRoughness, 2);
	wood->AddTexture(woodMetal, 3);
	wood->FinalizeMaterial();

	// Floor Cube
	std::shared_ptr<GameEntity> floorEntity = std::make_shared<GameEntity>(cubeMesh, std::make_shared<Material>(pipelineState, XMFLOAT3(0.2f,0.2f,0.2f)));
	floorEntity->GetTransform()->SetScale(100);
	floorEntity->GetTransform()->SetPosition(0, -103, 0);
	entityList.push_back(floorEntity);

	// Torus
	std::shared_ptr<GameEntity> torus = std::make_shared<GameEntity>(torusMesh, std::make_shared<Material>(pipelineState, XMFLOAT3(0.5f,0.2f,0.1f)));
	torus->GetTransform()->SetScale(2);
	torus->GetTransform()->SetPosition(0, 1, 0);
	entityList.push_back(torus);

	// Cylinder
	std::shared_ptr<GameEntity> cylinder = std::make_shared<GameEntity>(cylinderMesh, std::make_shared<Material>(pipelineState, XMFLOAT3(0.1f,0.5f,0.2f)));
	cylinder->GetTransform()->SetScale(1.5f);
	cylinder->GetTransform()->SetPosition(3, 2, 6);
	entityList.push_back(cylinder);

	// Helix
	std::shared_ptr<GameEntity> helix = std::make_shared<GameEntity>(helixMesh, std::make_shared<Material>(pipelineState, XMFLOAT3(0.4f,0.5f,0.2f)));
	helix->GetTransform()->SetScale(0.5f);
	helix->GetTransform()->SetPosition(5, 3, 5);
	entityList.push_back(helix);

	// Spheres
	for (int i = 0; i < 15; i++) {
		std::shared_ptr<Material> randomMat = std::make_shared<Material>(pipelineState, XMFLOAT3(
			RandomRange(0.0f, 1.0f),
			RandomRange(0.0f, 1.0f),
			RandomRange(0.0f, 1.0f)));

		float scale = RandomRange(0.5f, 1.5f);

		std::shared_ptr<GameEntity> entity = std::make_shared<GameEntity>(sphereMesh, randomMat);
		entity->GetTransform()->SetScale(scale);
		entity->GetTransform()->SetPosition(
			RandomRange(-6, 6),
			-2 + scale / 2.0f,
			RandomRange(-6, 6));

		entityList.push_back(entity);
	}

	RaytracingHelper::GetInstance().CreateTopLevelAccelerationStructureForScene(entityList);
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state objects for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		// Create an input layout that describes the vertex format
		// used by the vertex shader we're using
		// - This is used by the pipeline to know how to interpret the raw data
		//    sitting inside a vertex buffer

		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION";			   // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0;					   // This is the first POSITION semantic

		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[1].SemanticName = "NORMAL";
		inputElements[1].SemanticIndex = 0;					   // This is the first NORMAL semantic

		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;    // R32 G32 = float2
		inputElements[2].SemanticName = "TEXCOORD";
		inputElements[2].SemanticIndex = 0;					   // This is the first TEXCOORD semantic

		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0;					   // This is the first TANGENT semantic
	}

	// Root Signature
	{
		// Describe the range of CBVs needed for the vertex shader
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors = 1;
		cbvRangeVS.BaseShaderRegister = 0;
		cbvRangeVS.RegisterSpace = 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Describe the range of CBVs needed for the pixel shader
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors = 1;
		cbvRangePS.BaseShaderRegister = 0;
		cbvRangePS.RegisterSpace = 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create a range of SRV's for textures
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 4;     // Set to max number of textures at once (match pixel shader!)
		srvRange.BaseShaderRegister = 0; // Starts at s0 (match pixel shader!)
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create the root parameters
		D3D12_ROOT_PARAMETER rootParams[3] = {};

		// CBV table param for vertex shader
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;

		// CBV table param for pixel shader
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;

		// SRV table param
		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;

		// Create a single static sampler (available to all pixel shaders at the same slot)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // register (s0)
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe and serialize the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related --
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) --
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();

		// -- Render targets --
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;

		// -- States --
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;

		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc --
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipe state object
		device->CreateGraphicsPipelineState(&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	camera->UpdateProjectionMatrix((float)windowWidth / windowHeight);

	RaytracingHelper::GetInstance().ResizeOutputUAV(windowWidth, windowHeight);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	entityList[1]->GetTransform()->Rotate(
		0.5f * deltaTime,
		0.5f * deltaTime,
		0.5f * deltaTime);

	//for (int i = 2; i < entityList.size(); i++)
	//{
	//	XMFLOAT3 pos = entityList[i]->GetTransform()->GetPosition();

	//	pos.x += deltaTime * RandomRange(-0.5f, 0.5f);
	//	pos.z += deltaTime * RandomRange(-0.5f, 0.5f);

	//	entityList[i]->GetTransform()->SetPosition(pos);
	//}

	for (int i = 2; i < entityList.size(); i++)
	{
		XMFLOAT3 pos = entityList[i]->GetTransform()->GetPosition();

		float dir = -1.0f;
		if (i % 2 == 0) dir = 1.0f;

		pos.x += dir * RandomRange(0.0f, 0.025f) * deltaTime;
		pos.z += -dir * RandomRange(0.0f, 0.025f) * deltaTime;

		entityList[i]->GetTransform()->SetPosition(pos);
		entityList[i]->GetTransform()->Rotate(
			0.25f * deltaTime,
			0.25f * deltaTime,
			0.25f * deltaTime);
	}

	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	//// Grab the current back buffer for this frame
	//Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer = backBuffers[currentSwapBuffer];

	//// Clearing the render target
	//{
	//	// Transition the back buffer from present to render target
	//	D3D12_RESOURCE_BARRIER rb = {};
	//	rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//	rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//	rb.Transition.pResource = currentBackBuffer.Get();
	//	rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//	rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//	rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//	commandList->ResourceBarrier(1, &rb);

	//	// Background color (Cornflower Blue in this case) for clearing
	//	float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };

	//	// Clear the RTV
	//	commandList->ClearRenderTargetView(
	//		rtvHandles[currentSwapBuffer],
	//		color,
	//		0, 0); // No scissor rectangles

	//	// Clear the depth buffer, too
	//	commandList->ClearDepthStencilView(
	//		dsvHandle,
	//		D3D12_CLEAR_FLAG_DEPTH,
	//		1.0f,	// Max depth = 1.0f
	//		0,		// Not clearing stencil, but need a value
	//		0, 0);	// No scissor rects
	//}

	//// Rendering here!
	//{
	//	// Set overall pipeline state
	//	commandList->SetPipelineState(pipelineState.Get());

	//	// Root sig (msut happen before root descriptor table)
	//	commandList->SetGraphicsRootSignature(rootSignature.Get());


	//	DX12Helper& dx12Helper = DX12Helper::GetInstance();

	//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap =
	//		dx12Helper.GetCBVSRVDescriptorHeap();

	//	commandList->SetDescriptorHeaps(1, descriptorHeap.GetAddressOf());

	//	// Set up other commands for rendering
	//	commandList->OMSetRenderTargets(1, &rtvHandles[currentSwapBuffer], true, &dsvHandle);
	//	commandList->RSSetViewports(1, &viewport);
	//	commandList->RSSetScissorRects(1, &scissorRect);
	//	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	for (int i = 0; i < entityList.size(); i++)
	//	{
	//		std::shared_ptr<Material> mat = entityList[i]->GetMaterial();
	//		commandList->SetPipelineState(mat->GetPipelineState().Get());

	//		VertexShaderExternalData vsData = {};
	//		vsData.world = entityList[i]->GetTransform()->GetWorldMatrix();
	//		vsData.worldInverseTranspose = entityList[i]->GetTransform()->GetWorldInverseTransposeMatrix();
	//		vsData.projection = camera->GetProjection();
	//		vsData.view = camera->GetView();

	//		D3D12_GPU_DESCRIPTOR_HANDLE cbHandleVS =
	//			dx12Helper.FillNextConstantBufferAndGetGPUDescriptorHandle(
	//				(void*)(&vsData), sizeof(VertexShaderExternalData));

	//		commandList->SetGraphicsRootDescriptorTable(0, cbHandleVS);

	//		{
	//		PixelShaderExternalData psData = {};
	//		psData.uvScale = mat->GetUVScale();
	//		psData.uvOffset = mat->GetUVOffset();
	//		psData.cameraPosition = camera->GetTransform()->GetPosition();
	//		psData.lightCount = lightCount;
	//		memcpy(psData.lights, &lights[0], sizeof(Light) * MAX_LIGHTS);

	//		D3D12_GPU_DESCRIPTOR_HANDLE cbHandlePS =
	//			dx12Helper.FillNextConstantBufferAndGetGPUDescriptorHandle(
	//				(void*)(&psData), sizeof(PixelShaderExternalData));

	//		commandList->SetGraphicsRootDescriptorTable(1, cbHandlePS);
	//		}

	//		commandList->SetGraphicsRootDescriptorTable(2, mat->GetFinalGPUHandleForSRVs());

	//		D3D12_VERTEX_BUFFER_VIEW evbView = entityList[i]->GetMesh()->GetVBView();
	//		D3D12_INDEX_BUFFER_VIEW eibView = entityList[i]->GetMesh()->GetIBView();

	//		commandList->IASetVertexBuffers(0, 1, &evbView);
	//		commandList->IASetIndexBuffer(&eibView);

	//		// Draw
	//		commandList->DrawIndexedInstanced(entityList[i]->GetMesh()->GetIndexCount(), 1, 0, 0, 0);
	//	}
	//}

	//// Present
	//{
	//	// Transition back to present
	//	D3D12_RESOURCE_BARRIER rb = {};
	//	rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//	rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//	rb.Transition.pResource = currentBackBuffer.Get();
	//	rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//	rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//	rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//	commandList->ResourceBarrier(1, &rb);

	//	// Must occur BEFORE present
	//	DX12Helper::GetInstance().CloseExecuteAndResetCommandList();

	//	// Present the current back buffer
	//	bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
	//		swapChain->Present(
	//			vsyncNecessary ? 1 : 0,
	//			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

	//	// Figure out which buffer is next
	//	currentSwapBuffer++;
	//	if (currentSwapBuffer >= numBackBuffers)
	//			currentSwapBuffer = 0;
	//}

	// Grab the helper
	DX12Helper& dx12Helper = DX12Helper::GetInstance();

	// Reset allocator associated with the current buffer
	// and set up the command list to use that allocator
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), 0);

	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer = backBuffers[currentSwapBuffer];

	// Raytracing
	{
		// Update the raytracing accel structure
		RaytracingHelper::GetInstance().
			CreateTopLevelAccelerationStructureForScene(entityList);

		// Perform raytrace, including execution of command list
		RaytracingHelper::GetInstance().Raytrace(
			camera, 
			backBuffers[currentSwapBuffer]);
	}

	// Present
	{
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Update current swap buffer index
		currentSwapBuffer++;
		if (currentSwapBuffer >= numBackBuffers)
			currentSwapBuffer = 0;

		dx12Helper.WaitForGPU();
	}
}