////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2014 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cpp3ds/Graphics/Vertex.hpp>
#ifndef EMULATION
#include <3ds.h>
#include <bits/functexcept.h>
#endif


namespace cpp3ds
{
////////////////////////////////////////////////////////////
Vertex::Vertex() :
position (0, 0),
color    (255, 255, 255),
texCoords(0, 0),
normal   (0, 0, 1)
{
}


////////////////////////////////////////////////////////////
Vertex::Vertex(const Vector3f& thePosition) :
position (thePosition),
color    (255, 255, 255),
texCoords(0, 0),
normal   (0, 0, 1)
{
}


////////////////////////////////////////////////////////////
Vertex::Vertex(const Vector3f& thePosition, const Color& theColor) :
position (thePosition),
color    (theColor),
texCoords(0, 0),
normal   (0, 0, 1)
{
}


////////////////////////////////////////////////////////////
Vertex::Vertex(const Vector3f& thePosition, const Vector2f& theTexCoords) :
position (thePosition),
color    (255, 255, 255),
texCoords(theTexCoords),
normal   (0, 0, 1)
{
}


////////////////////////////////////////////////////////////
Vertex::Vertex(const Vector3f& thePosition, const Color& theColor, const Vector2f& theTexCoords) :
position (thePosition),
color    (theColor),
texCoords(theTexCoords),
normal   (0, 0, 1)
{
}


////////////////////////////////////////////////////////////
Vertex::Vertex(const Vector3f& thePosition, const Color& theColor, const Vector2f& theTexCoords, const Vector3f& theNormal) :
position (thePosition),
color    (theColor),
texCoords(theTexCoords),
normal   (theNormal)
{
}


#ifndef EMULATION
////////////////////////////////////////////////////////////
void* Vertex::operator new (std::size_t size)
{
	void *p = linearAlloc(size);
	if (!p)
		std::__throw_bad_alloc();
	return p;
}

////////////////////////////////////////////////////////////
void Vertex::operator delete (void *p)
{
	linearFree(p);
}

////////////////////////////////////////////////////////////
void* Vertex::operator new[] (std::size_t size)
{
	void *p = linearAlloc(size);
	if (!p)
		std::__throw_bad_alloc();
	return p;
}

////////////////////////////////////////////////////////////
void Vertex::operator delete[] (void *p)
{
	linearFree(p);
}
#endif

} // namespace cpp3ds
