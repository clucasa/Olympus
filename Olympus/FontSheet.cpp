
#include "FontSheet.h"
#include <Gdiplus.h>
#include <cassert>

using namespace std;
using namespace Gdiplus;
  
FontSheet::FontSheet() :
	mInitialized(false),
	mFontSheetTex(0),
	mFontSheetSRV(0),
	mTexWidth(1024),
	mTexHeight(0), // to be calculated
	mSpaceWidth(0),
	mCharHeight(0)
{

}

FontSheet::~FontSheet()
{
	//ReleaseCOM(mFontSheetTex);
	//ReleaseCOM(mFontSheetSRV);
}

ID3D11ShaderResourceView* FontSheet::GetFontSheetSRV()
{
	assert(mInitialized);
	return mFontSheetSRV;
}

const CD3D11_RECT& FontSheet::GetCharRect(WCHAR c)
{
	assert(mInitialized);
	return mCharRects[c - StartChar];
}

int FontSheet::GetSpaceWidth()
{
	assert(mInitialized);
	return mSpaceWidth;
}

int FontSheet::GetCharHeight()
{
	assert(mInitialized);
	return mCharHeight;
}
 
HRESULT FontSheet::Initialize(ID3D11Device* device, const std::wstring& fontName, 
		   				      float pixelFontSize, FontStyle fontStyle, bool antiAliased)
{
	// Prevent double Init.
	assert(!mInitialized);

	HRESULT hr = S_OK;

	// Init GDI+
	ULONG_PTR token = NULL;
	GdiplusStartupInput startupInput(NULL, TRUE, TRUE);
	GdiplusStartupOutput startupOutput;
	GdiplusStartup(&token, &startupInput, &startupOutput);

	// Create a new scope so all GDI+ objects are destroyed before we shutdown GDI+.
	{
		Font font(fontName.c_str(), pixelFontSize, fontStyle, UnitPixel);

		// The bitmap of antialiased text might look "blocky", but you have to look at the
		// alpha channel which has the smooth edges.  
		TextRenderingHint hint = antiAliased ? TextRenderingHintAntiAlias : TextRenderingHintSystemDefault;

		//
		// Bitmap for drawing a single char.
		//
		int tempSize = static_cast<int>(pixelFontSize * 2);
		Bitmap charBitmap(tempSize, tempSize, PixelFormat32bppARGB);
		Graphics charGraphics(&charBitmap);
		charGraphics.SetPageUnit(UnitPixel);
		charGraphics.SetTextRenderingHint(hint);
		
		// Determine mCharHeight, mSpaceWidth, and mTexHeight.
		MeasureChars(font, charGraphics);

		//
		// Bitmap for storing all the char sprites on a sprite sheet.
		//
		Bitmap fontSheetBitmap(mTexWidth, mTexHeight, PixelFormat32bppARGB);
		Graphics fontSheetGraphics(&fontSheetBitmap);
		fontSheetGraphics.SetCompositingMode(CompositingModeSourceCopy);
		fontSheetGraphics.Clear(Color(0, 0, 0, 0));

		BuildFontSheetBitmap(font, charGraphics, charBitmap, fontSheetGraphics);

//    Uncomment for debugging to save font sheet as .bmp file. TESTED THIS IS NOT MY CURRENT PROBLEM
		//	CLSID clsid;
		//GetEncoderClsid(L"image/bmp", &clsid);
		//fontSheetBitmap.Save(L"font.bmp", &clsid, NULL);

		if(FAILED(BuildFontSheetTexture(device, fontSheetBitmap)))
		{
			GdiplusShutdown(token);
			return hr;
		}
	}

	// Shutdown GDI+: You must delete all of your GDI+ objects 
	// (or have them go out of scope) before you call GdiplusShutdown.
	GdiplusShutdown(token);

	mInitialized = true;

	return hr; 
}

void FontSheet::MeasureChars(Font& font, Graphics& charGraphics)
{
	WCHAR allChars[NumChars + 1];
	for(WCHAR i = 0; i < NumChars; ++i)
		allChars[i] = StartChar + i;
	allChars[NumChars] = 0;

	RectF sizeRect;
	charGraphics.MeasureString(allChars, NumChars, &font, PointF(0, 0), &sizeRect);
	mCharHeight = static_cast<int>(sizeRect.Height + 0.5f);  

	// Given the fixed texture width, figure out how many rows we need to draw
	// all the characters, and consequently how much texture height we need.
	int numRows = static_cast<int>(sizeRect.Width / mTexWidth) + 1;  
	mTexHeight  = static_cast<int>(numRows*mCharHeight) + 1;

	// Measure space character (which we do not store in the atlas).
	WCHAR charString[2] = {' ', 0};
	charGraphics.MeasureString(charString, 1, &font, PointF(0, 0), &sizeRect);
	mSpaceWidth = static_cast<int>(sizeRect.Width + 0.5f);  
}

void FontSheet::BuildFontSheetBitmap(Font& font,
												 Graphics& charGraphics, 
												 Bitmap& charBitmap, 
												 Graphics& fontSheetGraphics)
{
	WCHAR charString[2] = {' ', 0};
	SolidBrush whiteBrush(Color(255, 255, 255, 255));
	UINT fontSheetX = 0;
	UINT fontSheetY = 0;
	for(UINT i = 0; i < NumChars; ++i)
	{
		charString[0] = static_cast<WCHAR>(StartChar + i);
		charGraphics.Clear(Color(0, 0, 0, 0));
		charGraphics.DrawString(charString, 1, &font, PointF(0.0f, 0.0f), &whiteBrush);

		// Compute tight char horizontal bounds (ignoring empty space).
		int minX = GetCharMinX(charBitmap);
		int maxX = GetCharMaxX(charBitmap);
		int charWidth = maxX - minX + 1;

		// Move to next row of the font sheet?
		if(fontSheetX + charWidth >= mTexWidth)
		{
			fontSheetX = 0;
			fontSheetY += static_cast<int>(mCharHeight) + 1;
		}

		// Save the rectangle of this character on the texture atlas.
		mCharRects[i] = CD3D11_RECT(fontSheetX, fontSheetY, fontSheetX + charWidth, fontSheetY + mCharHeight);

		// The rectangle subset of the source image to copy.
		fontSheetGraphics.DrawImage(&charBitmap, fontSheetX, fontSheetY, 
			minX, 0, charWidth, mCharHeight, UnitPixel);

		// next char
		fontSheetX += charWidth + 1;
	}
}


HRESULT FontSheet::BuildFontSheetTexture(ID3D11Device* device, Bitmap& fontSheetBitmap)
{
	HRESULT hr = S_OK;

	// Lock the bitmap for direct memory access
	BitmapData bmData;
	fontSheetBitmap.LockBits(&Rect(0, 0, mTexWidth, mTexHeight), 
		ImageLockModeRead, PixelFormat32bppARGB, &bmData);  

	// Copy into a texture.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width  = mTexWidth;
	texDesc.Height = mTexHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;        
	data.pSysMem = bmData.Scan0;
	data.SysMemPitch = mTexWidth * 4;
	data.SysMemSlicePitch = 0;

	hr = device->CreateTexture2D(&texDesc, &data, &mFontSheetTex);
	if(FAILED(hr))
		return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(mFontSheetTex, &srvDesc, &mFontSheetSRV);
	if(FAILED(hr))
		return hr;

	fontSheetBitmap.UnlockBits(&bmData);  

	return hr;
}

int FontSheet::GetCharMinX(Bitmap& charBitmap)
{
	int width  = charBitmap.GetWidth();
	int height = charBitmap.GetHeight();
	
	for(int x = 0; x < width; ++x)
	{
		for(int y = 0; y < height; ++y)
		{
			Color color;
			charBitmap.GetPixel(x, y, &color);
			if(color.GetAlpha() > 0)
			{
				 return x;
			}
		}
	}

	return 0;
}

int FontSheet::GetCharMaxX(Bitmap& charBitmap)
{
	int width  = charBitmap.GetWidth();
	int height = charBitmap.GetHeight();

	for(int x = width-1; x >= 0; --x)
	{
		for(int y = 0; y < height; ++y)
		{
			Color color;
			charBitmap.GetPixel(x, y, &color);
			if(color.GetAlpha() > 0)
			{
				 return x;
			}
		}
	}

	return width-1;
}

int FontSheet::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
