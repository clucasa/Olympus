#pragma once

#ifndef VERTEX_H
#define VERTEX_H

struct Vertex
{
	Vertex(){}
	Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
		: Pos(p), Normal(n), Tangent(t), Tex(uv){}
	Vertex(
		float px, float py, float pz, 
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v)
		: Pos(px,py,pz), Normal(nx,ny,nz),
			Tangent(tx, ty, tz), Tex(u,v){}

	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	int 	 texNum;
	XMFLOAT3 Tangent;
	XMFLOAT3 BiNormal;
};

#endif