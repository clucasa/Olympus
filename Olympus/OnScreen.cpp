#include "OnScreen.h"
#include "FontSheet.h"
#include "Effects.h"
#include <cassert>
using namespace std;

OnScreen::OnScreen() :
	mInitialized(false),
	mVB(0),
	mIB(0),
	mInputLayout(0),
	mScreenWidth(0.0f),
	mScreenHeight(0.0f),
	mBatchTexSRV(0),
	mTexWidth(0),
	mTexHeight(0)
{
}

OnScreen::~OnScreen()
{
	//ReleaseCOM(mVB);
	//ReleaseCOM(mIB);
	//ReleaseCOM(mInputLayout);
}

HRESULT OnScreen::Initialize(ID3D11Device* device)
{
	// Prevent double Init.
	assert(!mInitialized);

	HRESULT hr = S_OK;

	mSpriteList.reserve(128);

	vector<USHORT> indices(BatchSize*6);
	for(UINT i = 0; i < BatchSize; ++i)
	{
		indices[i*6+0] = i*4+0;
		indices[i*6+1] = i*4+1;
		indices[i*6+2] = i*4+2;

		indices[i*6+3] = i*4+0;
		indices[i*6+4] = i*4+2;
		indices[i*6+5] = i*4+3;
	}

	D3D11_SUBRESOURCE_DATA indexData = {0};
	indexData.pSysMem = &indices[0];

	ID3DBlob *VS, *PS;
    D3DX11CompileFromFile("OnScreen.hlsl", 0, 0, "VS", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile("OnScreen.hlsl", 0, 0, "PS", "ps_5_0", 0, 0, 0, &PS, 0, 0);
	int taco =  VS->GetBufferSize();

	hr = device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &opVS);
    if( FAILED(hr) )
		return hr;
	device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &opPS);
	if( FAILED(hr) )
		return hr;

	
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};	
	hr = device->CreateInputLayout(layoutDesc, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &mInputLayout);



	

	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = BatchSize*4*sizeof(SpriteVertex);
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	hr = device->CreateBuffer(&vbd, 0, &mVB);
	if(FAILED(hr))
		return hr;

	// Index buffer is immutable.
	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = BatchSize*6*sizeof(USHORT);
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	hr = device->CreateBuffer(&ibd, &indexData, &mIB);
	if(FAILED(hr))
		return hr;

	mInitialized = true;

	return hr;
}

void OnScreen::BeginBatch(ID3D11ShaderResourceView* texSRV)
{
	assert(mInitialized);

	mBatchTexSRV = texSRV;
	mBatchTexSRV->AddRef();

	// Get the size of the texture
	ID3D11Resource* resource = 0;
	mBatchTexSRV->GetResource(&resource);
	ID3D11Texture2D* tex = reinterpret_cast<ID3D11Texture2D*>(resource);

	D3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

//	ReleaseCOM(resource);

	mTexWidth  = texDesc.Width; 
	mTexHeight = texDesc.Height;

	mSpriteList.clear();
}

void OnScreen::EndBatch(ID3D11DeviceContext* dc)
{
	assert(mInitialized);

	UINT viewportCount = 1;
	D3D11_VIEWPORT vp;
	dc->RSGetViewports(&viewportCount, &vp);

	mScreenWidth  = vp.Width;
	mScreenHeight = vp.Height;

	UINT stride = sizeof(SpriteVertex);
	UINT offset = 0;
	dc->IASetInputLayout(mInputLayout);
	dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R16_UINT, 0);
	dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Effects::SpriteFX->SetSpriteMap(mBatchTexSRV);
	
	//ID3DX11EffectPass* pass = Effects::SpriteFX->SpriteTech->GetPassByIndex(0);
	//pass->Apply(0, dc);

	UINT spritesToDraw = mSpriteList.size();
	UINT startIndex = 0;

	while( spritesToDraw > 0 )
	{
		if( spritesToDraw <= BatchSize )
		{
			DrawBatch(dc, startIndex, spritesToDraw);
			spritesToDraw = 0;
		}
		else
		{
			DrawBatch(dc, startIndex, BatchSize);
			startIndex += BatchSize;
			spritesToDraw -= BatchSize;
		}
	}

//	ReleaseCOM(mBatchTexSRV);
}

void OnScreen::Draw(const POINT& position, XMCOLOR color)
{
	Sprite sprite;
	sprite.SrcRect  = CD3D11_RECT(0, 0, mTexWidth, mTexHeight);
	sprite.DestRect = CD3D11_RECT(position.x, position.y, position.x + mTexWidth, position.y + mTexHeight);
	sprite.Color    = color;

	mSpriteList.push_back(sprite);
}

void OnScreen::Draw(const POINT& position, const CD3D11_RECT& sourceRect, XMCOLOR color)
{
	int srcWidth  = sourceRect.right - sourceRect.left;
	int srcHeight = sourceRect.bottom - sourceRect.top;

	Sprite sprite;
	sprite.SrcRect  = sourceRect;
	sprite.DestRect = CD3D11_RECT(position.x, position.y, position.x + srcWidth, position.y + srcHeight);
	sprite.Color    = color;

	mSpriteList.push_back(sprite);
}

void OnScreen::Draw(const POINT& position, const CD3D11_RECT& sourceRect, XMCOLOR color,
	                   float z, float angle, float scale)
{
	int srcWidth  = sourceRect.right - sourceRect.left;
	int srcHeight = sourceRect.bottom - sourceRect.top;

	Sprite sprite;
	sprite.SrcRect = sourceRect;
	sprite.DestRect = CD3D11_RECT(position.x, position.y, position.x + srcWidth, position.y + srcHeight);
	sprite.Color  = color;
	sprite.Z      = z;
	sprite.Angle  = angle;
	sprite.Scale  = scale;

	mSpriteList.push_back(sprite);
}

void OnScreen::Draw(const CD3D11_RECT& destinationRect, XMCOLOR color)
{
	Sprite sprite;
	sprite.SrcRect  = CD3D11_RECT(0, 0, mTexWidth, mTexHeight);
	sprite.DestRect = destinationRect;
	sprite.Color    = color;

	mSpriteList.push_back(sprite);
}

void OnScreen::Draw(const CD3D11_RECT& destinationRect, const CD3D11_RECT& sourceRect, XMCOLOR color)
{
	Sprite sprite;
	sprite.SrcRect  = sourceRect;
	sprite.DestRect = destinationRect;
	sprite.Color    = color;

	mSpriteList.push_back(sprite);
}

void OnScreen::Draw(const CD3D11_RECT& destinationRect, const CD3D11_RECT& sourceRect, XMCOLOR color,
	                    float z, float angle, float scale)
{
	Sprite sprite;
	sprite.SrcRect  = sourceRect;
	sprite.DestRect = destinationRect;
	sprite.Color    = color;
	sprite.Z        = z;
	sprite.Angle    = angle;
	sprite.Scale    = scale;

	mSpriteList.push_back(sprite);
}

void OnScreen::DrawString(ID3D11DeviceContext* dc, FontSheet& fs, std::string text, 
							 const POINT& pos, XMCOLOR color)
{
			dc->VSSetShader(opVS, 0, 0);
		dc->PSSetShader(opPS, 0, 0);

		dc->IASetInputLayout(mInputLayout);
		
		dc->PSSetShaderResources(0, 1, &mBatchTexSRV );


	BeginBatch(fs.GetFontSheetSRV());
	
	UINT length = text.length();

	int posX = pos.x;
	int posY = pos.y;

	// For each character in the string...
	for(UINT i = 0; i < length; ++i)
	{
		WCHAR character = text[i];

		// Is the character a space char?
		if(character == ' ') 
		{
			posX += fs.GetSpaceWidth();
		}
		// Is the character a newline char?
		else if(character == '\n')        
		{
			posX  = pos.x;
			posY += fs.GetCharHeight();
		}
		else
		{
			// Get the bounding rect of the character on the fontsheet.
			const CD3D11_RECT& charRect = fs.GetCharRect(character);

			int width = charRect.right - charRect.left;
			int height = charRect.bottom - charRect.top;

			// Draw the character sprite.
			Draw(CD3D11_RECT(posX, posY, posX + width, posY + height), charRect, color);

			// Move to the next character position.
			posX += width + 1;
      }
   }

	EndBatch(dc);
}

void OnScreen::DrawBatch(ID3D11DeviceContext* dc, UINT startSpriteIndex, UINT spriteCount)
{
	// 
	// Write the quads to the vertex buffer.
	//
	D3D11_MAPPED_SUBRESOURCE mappedData;
	dc->Map(mVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	SpriteVertex* v = reinterpret_cast<SpriteVertex*>(mappedData.pData);

	for(UINT i = 0; i < spriteCount; ++i)
	{
		const Sprite& sprite = mSpriteList[startSpriteIndex + i];

		SpriteVertex quad[4];
		BuildSpriteQuad(sprite, quad);

		v[i*4+0] = quad[0];
		v[i*4+1] = quad[1];
		v[i*4+2] = quad[2];
		v[i*4+3] = quad[3];
	}

	dc->Unmap(mVB, 0);

	dc->DrawIndexed(spriteCount*6, 0, 0);
}

XMFLOAT3 OnScreen::PointToNdc(int x, int y, float z)
{
	XMFLOAT3 p;

	p.x = 2.0f*(float)x/mScreenWidth - 1.0f;
	p.y = 1.0f - 2.0f*(float)y/mScreenHeight;
	p.z = z;

	return p;
}

void OnScreen::BuildSpriteQuad(const Sprite& sprite, SpriteVertex v[4])
{
	const CD3D11_RECT& dest = sprite.DestRect;
	const CD3D11_RECT& src  = sprite.SrcRect;

	// Dest rect defines target in screen space.
	v[0].Pos = PointToNdc(dest.left,  dest.bottom, sprite.Z);
	v[1].Pos = PointToNdc(dest.left,  dest.top,    sprite.Z);
	v[2].Pos = PointToNdc(dest.right, dest.top,    sprite.Z);
	v[3].Pos = PointToNdc(dest.right, dest.bottom, sprite.Z);

	// Source rect defines subset of texture to use from sprite sheet.
	v[0].Tex = XMFLOAT2((float)src.left  / mTexWidth, (float)src.bottom / mTexHeight); 
	v[1].Tex = XMFLOAT2((float)src.left  / mTexWidth, (float)src.top    / mTexHeight); 
	v[2].Tex = XMFLOAT2((float)src.right / mTexWidth, (float)src.top    / mTexHeight); 
	v[3].Tex = XMFLOAT2((float)src.right / mTexWidth, (float)src.bottom / mTexHeight); 

	v[0].Color = sprite.Color;
	v[1].Color = sprite.Color;
	v[2].Color = sprite.Color;
	v[3].Color = sprite.Color;

	// Quad center point.
	float tx = 0.5f*(v[0].Pos.x + v[3].Pos.x);
	float ty = 0.5f*(v[0].Pos.y + v[1].Pos.y);

	XMVECTOR scaling = XMVectorSet(sprite.Scale, sprite.Scale, 1.0f, 0.0f);
	XMVECTOR origin = XMVectorSet(tx, ty, 0.0f, 0.0f);
	XMVECTOR translation = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMMATRIX T = XMMatrixAffineTransformation2D(scaling, origin, sprite.Angle, translation);

	// Rotate and scale the quad in NDC space.
	for(int i = 0; i < 4; ++i)
	{
		XMVECTOR p = XMLoadFloat3(&v[i].Pos);
		p = XMVector3TransformCoord(p, T);
		XMStoreFloat3(&v[i].Pos, p);
	}
}