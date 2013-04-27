///
/// OnScreen.h
///
#pragma once
#ifndef ONSCREEN_H
#define ONSCREEN_H

#include "TextHelper.h"
#include <string>

using namespace std;

class FontSheet;

///
/// Batches and draws screenspace rectangles and text.
///
/// Only one instance of OnScreen is needed per application.
/// To minimize draw calls, the preferred way to rendering sprites with
/// different textures is to use a texture atlas, and the Draw() function 
/// has overloads that take a sourceRect parameter.  
///
class OnScreen
{
public:
	OnScreen();
	~OnScreen();





	ID3D11Device *odev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *odevcon;           // the pointer to our Direct3D device context

	///
	/// Initializes a OnScreen object.
	/// 
	HRESULT Initialize(ID3D11Device* device);

	///
	/// Begins a new sprite batch.  All sprites in the batch will use the given texture.
	/// All Draw() calls add a sprite to the batch.
	///
	void BeginBatch(ID3D11ShaderResourceView* texSRV);

	///
	/// Draw the current sprite batch and empties the internal sprite batch list.
	///
	void EndBatch(ID3D11DeviceContext* dc);

	///
	/// Adds a sprite to the sprite batch.  This call is undefined if not
	///  called within a BeginBatch()/EndBatch().
	/// \param position The screen space position (upper-left corner) of the sprite to draw.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// 
	void Draw(const POINT& position, XMCOLOR color);

	///
	/// \brief Adds a sprite to the sprite batch.  This call is undefined if not
	///  called within a BeginBatch()/EndBatch().
	/// \param position The screen space position (upper-left corner) of the sprite to draw.
	/// \param sourceRect Subrectangle relative to the batch texture that specifies
	///  the atlas image to use for this sprite.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// 
	void Draw(const POINT& position, const CD3D11_RECT& sourceRect,	XMCOLOR color);

	///
	/// \brief Adds a sprite to the sprite batch.  This call is undefined if not
	///  called within a BeginBatch()/EndBatch().
	/// \param position The screen space position (upper-left corner) of the sprite to draw.
	/// \param sourceRect Subrectangle relative to the batch texture that specifies
	///  the atlas image to use for this sprite.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// \param z The z-order of the sprite in NDC space [0,1].  The standard depth test
	///  must be enabled for this to sort sprites by depth.
	/// \param angle Rotation angle of the sprite.  The sprite center point is the pivot point.
	/// \param scale Uniform sprite scale factor.
	///
	void Draw(const POINT& position, const CD3D11_RECT& sourceRect, XMCOLOR color,
		float z, float angle, float scale);

	///
	/// \brief Adds a sprite to the sprite batch.  This call is undefined if not
	///  called within a BeginBatch()/EndBatch().
	/// \param destinationRect Rectangle relative to the screen specifying the 
	///  screen destination to draw the sprite.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// 
	void Draw(const CD3D11_RECT& destinationRect, XMCOLOR color);

	///
	/// \brief Adds a sprite to the sprite batch.  This call is undefined if not
	///  called within a BeginBatch()/EndBatch().
	/// \param destinationRect Rectangle relative to the screen specifying the 
	///  screen destination to draw the sprite.
	/// \param sourceRect Subrectangle relative to the batch texture that specifies
	///  the atlas image to use for this sprite.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// 
	void Draw(const CD3D11_RECT& destinationRect, const CD3D11_RECT& sourceRect, XMCOLOR color);

	///
	/// \brief Adds a sprite to the sprite batch.  This call is undefined if not
	///  called within a BeginBatch()/EndBatch().
	/// \param destinationRect Rectangle relative to the screen specifying the 
	///  screen destination to draw the sprite.
	/// \param sourceRect Subrectangle relative to the batch texture that specifies
	///  the atlas image to use for this sprite.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// \param z The z-order of the sprite in NDC space [0,1].  The standard depth test
	///  must be enabled for this to sort sprites by depth.
	/// \param angle Rotation angle of the sprite.  The sprite center point is the pivot point.
	/// \param scale Uniform sprite scale factor.
	/// 
	void Draw(const CD3D11_RECT& destinationRect, const CD3D11_RECT& sourceRect, XMCOLOR color,
		float z, float angle, float scale);

	///
	/// \brief Draws a string to the screen.
	///
	/// DrawString should not be called inside BeginBatch()/EndBatch().  Internally,
	/// DrawString calls BeginBatch()/EndBatch() to draw the string characters as a
	/// single batch.
	///
	/// \param fs The font sheet describing the font to use to draw the string.
	/// \param text The string to draw.
	/// \param position The screen space position (upper-left corner) of the string to draw.
	/// \param color The color of the sprite modulated with the texture color.  Specify 
	///  white to keep the texture color unchanged.
	/// 
	void DrawString(ID3D11DeviceContext* dc, FontSheet& fs, std::string text, const POINT& pos, XMCOLOR color);

private:
	OnScreen(const OnScreen& rhs);
	OnScreen& operator=(const OnScreen& rhs);

		ID3D10Blob *oVS, *oPS;
			ID3D11VertexShader *opVS;               // the pointer to the vertex shader
	ID3D11PixelShader *opPS;               // the pointer to the vertex shader


	struct SpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMCOLOR Color;
	};

	struct Sprite
	{
		Sprite() :
			Color(1.0f, 1.0f, 1.0f, 1.0f),
			Z(0.0f),
			Angle(0.0f),
			Scale(1.0f)
		{
		}
 
		CD3D11_RECT SrcRect;
		CD3D11_RECT DestRect;
		XMCOLOR Color;
		float Z;
		float Angle;
		float Scale;
	};

	///
	/// Helper method for drawing a subset of sprites in the batch.
	///
	void DrawBatch(ID3D11DeviceContext* dc, UINT startSpriteIndex, UINT spriteCount);

	///
	/// Convert screen space point to NDC space.
	///
	XMFLOAT3 PointToNdc(int x, int y, float z);

	/// 
	/// Generates quad for the given sprite. 
	///
	void BuildSpriteQuad(const Sprite& sprite, SpriteVertex v[4]);
 
private:
	static const int BatchSize = 512;

	bool mInitialized;

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3D11InputLayout* mInputLayout;

	float mScreenWidth;
	float mScreenHeight;

	// Texture to use for current batch.
	ID3D11ShaderResourceView* mBatchTexSRV;

	UINT mTexWidth;
	UINT mTexHeight;

	// List of sprites to draw using the current batch texture.
	vector<Sprite> mSpriteList;
};

#endif // SPRITE_BATCH_H