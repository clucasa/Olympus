///
/// FontSheet.h
///

#ifndef FONTSHEET_H
#define FONTSHEET_H

#include "TextHelper.h"

namespace Gdiplus
{
	class Bitmap;
	class Font;
	class Graphics;
}

///
/// A font sheet is a texture atlas that stores characters for the given font
/// to be used with the OnScreen class for rendering text. 
///
/// There is a fair amount of cost to create the font sheet, as we use GDI+ to 
/// render out each character, and then copy the sheet to a texture.  Therefore, 
/// all the FontSheets an application needs should be created at initialization
/// time.  This could even be moved to a prebuild step if we had a content pipeline.
///
class FontSheet
{
public:

	// Mirrors the Gdiplus FontStyle enum.
	enum FontStyle 
	{
		FontStyleRegular      = 0,
		FontStyleBold         = 1,
		FontStyleItalic       = 2,
		FontStyleBoldItalic   = 3,
		FontStyleUnderline    = 4,
		FontStyleStrikeout    = 8 
	};

public:

	FontSheet();
	~FontSheet();

	///
	/// Gets the SRV to the font sheet texture atlas.
	///
	ID3D11ShaderResourceView* GetFontSheetSRV();

	///
	/// Gets the rectangle on the sprite sheet that bounds the given character.
	///
	const CD3D11_RECT& GetCharRect(WCHAR c);

	///
	/// Gets the width of the "space" character.  This tells the OnScreen
	/// how much space to skip when rendering space characters.
	///
    int GetSpaceWidth();

	///
	/// Returns the character height for the font.  This should be used for
	/// newline characters when rendering text.
	/// 
    int GetCharHeight(); 
 
	///
	/// Initializes a font sheet object.
	///
	HRESULT Initialize(ID3D11Device* device, const std::wstring& fontName, 
		float pixelFontSize, FontStyle fontStyle, bool antiAliased);

private:
	FontSheet(const FontSheet& rhs);
	FontSheet& operator=(const FontSheet& rhs);

	///
	/// Determines mCharHeight, mSpaceWidth, and mTexHeight.
	///
	void MeasureChars(Gdiplus::Font& font, Gdiplus::Graphics& charGraphics);

	///
	/// Draw the characters one-by-one to the charBitmap, and then copy to the sprite sheet.
	///
	void BuildFontSheetBitmap(
		Gdiplus::Font& font,
		Gdiplus::Graphics& charGraphics, 
		Gdiplus::Bitmap& charBitmap, 
		Gdiplus::Graphics& fontSheetGraphics);

	///
	/// Copies the GDI+ font sheet to a d3d11 texture.
	///
	HRESULT BuildFontSheetTexture(ID3D11Device* device, Gdiplus::Bitmap& fontSheetBitmap);

	///
	/// Scans column-by-column to look for the left-most opaque pixel. We cannot
	/// scan row-by-row, as the first opaque pixel we find might not be "left-most".
	///
	int GetCharMinX(Gdiplus::Bitmap& charBitmap);

	///
	/// Scans column-by-column to look for the right-most opaque pixel. We cannot
	/// scan row-by-row, as the first opaque pixel we find might not be "right-most".
	///
	int GetCharMaxX(Gdiplus::Bitmap& charBitmap);

	///
	/// For saving the GDI Bitmap to file (for internal debugging).
	///
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

private:
	// ASCII characters from 33='!' to 127.  
	static const WCHAR StartChar = 33;
	static const WCHAR EndChar = 127;
	static const UINT NumChars = EndChar - StartChar;

	bool mInitialized;

	ID3D11Texture2D* mFontSheetTex;
	ID3D11ShaderResourceView* mFontSheetSRV;

	UINT mTexWidth;
	UINT mTexHeight;

	CD3D11_RECT mCharRects[NumChars];
	int mSpaceWidth;
	int mCharHeight;  
};

#endif // FONTSHEET_H