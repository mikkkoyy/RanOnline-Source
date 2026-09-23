#pragma once

class IRenderer
{
public:
	virtual ~IRenderer () {}

	virtual void Clear ( D3DCOLOR Color ) = 0;
	virtual bool BeginScene () = 0;
	virtual void EndScene () = 0;
	virtual bool IsValid () const = 0;
};
