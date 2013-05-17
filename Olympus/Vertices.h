#pragma once

#ifndef VERTICES_H
#define VERTICES_H

struct PosNormalTexTan
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT3 TangentU;
	XMFLOAT2 Tex;
};

#endif