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
#include <cpp3ds/Graphics/RenderTarget.hpp>
#include <cpp3ds/Graphics/Drawable.hpp>
#include <cpp3ds/Graphics/Shader.hpp>
#include <cpp3ds/Graphics/Texture.hpp>
#include <cpp3ds/Graphics/VertexArray.hpp>
#include <cpp3ds/OpenGL.hpp>
#include <cpp3ds/System/Err.hpp>
#include "CitroHelpers.hpp"

namespace
{
    // Convert an cpp3ds::BlendMode::Factor constant to the corresponding ctrulib constant.
    GPU_BLENDFACTOR factorToGlConstant(cpp3ds::BlendMode::Factor blendFactor)
    {
        switch (blendFactor)
        {
            default:
            case cpp3ds::BlendMode::Zero:             return GPU_ZERO;
            case cpp3ds::BlendMode::One:              return GPU_ONE;
            case cpp3ds::BlendMode::SrcColor:         return GPU_SRC_COLOR;
            case cpp3ds::BlendMode::OneMinusSrcColor: return GPU_ONE_MINUS_SRC_COLOR;
            case cpp3ds::BlendMode::DstColor:         return GPU_DST_COLOR;
            case cpp3ds::BlendMode::OneMinusDstColor: return GPU_ONE_MINUS_DST_COLOR;
            case cpp3ds::BlendMode::SrcAlpha:         return GPU_SRC_ALPHA;
            case cpp3ds::BlendMode::OneMinusSrcAlpha: return GPU_ONE_MINUS_SRC_ALPHA;
            case cpp3ds::BlendMode::DstAlpha:         return GPU_DST_ALPHA;
            case cpp3ds::BlendMode::OneMinusDstAlpha: return GPU_ONE_MINUS_DST_ALPHA;
        }
    }


    // Convert an cpp3ds::BlendMode::BlendEquation constant to the corresponding ctrulib constant.
    GPU_BLENDEQUATION equationToGlConstant(cpp3ds::BlendMode::Equation blendEquation)
    {
        switch (blendEquation)
        {
            default:
            case cpp3ds::BlendMode::Add:             return GPU_BLEND_ADD;
            case cpp3ds::BlendMode::Subtract:        return GPU_BLEND_SUBTRACT;
        }
    }

}


namespace cpp3ds
{
////////////////////////////////////////////////////////////
RenderTarget::RenderTarget() :
m_defaultView(),
m_view       (),
m_cache      ()
{
	m_cache.vertexCache = new Vertex[StatesCache::VertexCacheSize];
	m_cache.glStatesSet = false;
}


////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget()
{
	delete[] m_cache.vertexCache;
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color& color)
{
    if (activate(true))
    {
        u32 clearColor = (((color.r)&0xFF)<<24) | (((color.g)&0xFF)<<16) | (((color.b)&0xFF)<<8) | (((color.a)&0xFF)<<0);
        C3D_RenderTargetSetClear(m_target, C3D_CLEAR_ALL, clearColor, 0);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::setView(const View& view)
{
    m_view = view;
    m_cache.viewChanged = true;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getView() const
{
    return m_view;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getDefaultView() const
{
    return m_defaultView;
}


////////////////////////////////////////////////////////////
IntRect RenderTarget::getViewport(const View& view) const
{
    float width  = static_cast<float>(getSize().x);
    float height = static_cast<float>(getSize().y);
    const FloatRect& viewport = view.getViewport();

    return IntRect(static_cast<int>(0.5f + width  * viewport.left),
                   static_cast<int>(0.5f + height * viewport.top),
                   static_cast<int>(0.5f + width  * viewport.width),
                   static_cast<int>(0.5f + height * viewport.height));
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(const Vector2i& point) const
{
    return mapPixelToCoords(point, getView());
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(const Vector2i& point, const View& view) const
{
    // First, convert from viewport coordinates to homogeneous coordinates
    Vector2f normalized;
    IntRect viewport = getViewport(view);
    normalized.x = -1.f + 2.f * (point.x - viewport.left) / viewport.width;
    normalized.y =  1.f - 2.f * (point.y - viewport.top)  / viewport.height;

    // Then transform by the inverse of the view matrix
    return view.getInverseTransform().transformPoint(normalized);
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f& point) const
{
    return mapCoordsToPixel(point, getView());
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f& point, const View& view) const
{
    // First, transform the point by the view matrix
    Vector2f normalized = view.getTransform().transformPoint(point);

    // Then convert to viewport coordinates
    Vector2i pixel;
    IntRect viewport = getViewport(view);
    pixel.x = static_cast<int>(( normalized.x + 1.f) / 2.f * viewport.width  + viewport.left);
    pixel.y = static_cast<int>((-normalized.y + 1.f) / 2.f * viewport.height + viewport.top);

    return pixel;
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Drawable& drawable, const RenderStates& states)
{
    drawable.draw(*this, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Vertex* vertices, unsigned int vertexCount,
                        PrimitiveType type, const RenderStates& states)
{
    // Nothing to draw?
    if (!vertices || (vertexCount == 0))
        return;

	// Vertices allocated in the stack (common) can't be converted to physical address
	if (osConvertVirtToPhys(vertices) == 0)
	{
		err() << "RenderTarget::draw() called with vertex array in inaccessible memory space." << std::endl;
		return;
	}

    if (activate(true))
    {
        // First set the persistent OpenGL states if it's the very first call
        if (!m_cache.glStatesSet)
            resetGLStates();

        // Check if the vertex count is low enough so that we can pre-transform them
        bool useVertexCache = (vertexCount <= StatesCache::VertexCacheSize);
        if (useVertexCache)
        {
            // Pre-transform the vertices and store them into the vertex cache
            for (unsigned int i = 0; i < vertexCount; ++i)
            {
                Vertex& vertex = m_cache.vertexCache[i];
                vertex.position = states.transform * vertices[i].position;
                vertex.color = vertices[i].color;
                vertex.texCoords = vertices[i].texCoords;
            }

            // Since vertices are transformed, we must use an identity transform to render them
            if (!m_cache.useVertexCache)
                applyTransform(Transform::Identity);
        }
        else
        {
            applyTransform(states.transform);
        }

        // Apply the view
        if (m_cache.viewChanged)
            applyCurrentView();

        // Apply the blend mode
        if (states.blendMode != m_cache.lastBlendMode)
            applyBlendMode(states.blendMode);

        // Apply the texture
        Uint64 textureId = states.texture ? states.texture->m_cacheId : 0;
        if (textureId != m_cache.lastTextureId)
            applyTexture(states.texture);

        // Apply the shader
        if (states.shader)
            applyShader(states.shader);

        // If we pre-transform the vertices, we must use our internal vertex cache
        if (useVertexCache)
        {
            // ... and if we already used it previously, we don't need to set the pointers again
            if (!m_cache.useVertexCache)
                vertices = m_cache.vertexCache;
            else
                vertices = NULL;
        }

        // Setup the pointers to the vertices' components
        if (vertices)
        {
            C3D_BufInfo* bufInfo = C3D_GetBufInfo();
            BufInfo_Init(bufInfo);
            BufInfo_Add(bufInfo, vertices, sizeof(Vertex), 3, 0x210);
        }

        // Find the OpenGL primitive type
        static const GPU_Primitive_t modes[] = {GPU_TRIANGLES, GPU_TRIANGLE_STRIP, GPU_TRIANGLE_FAN, GPU_GEOMETRY_PRIM};
        GPU_Primitive_t mode = modes[type];

        CitroUpdateMatrixStacks();

        // Draw the primitives
        C3D_DrawArrays(mode, 0, vertexCount);

        // Unbind the shader, if any
        if (states.shader)
            applyShader(NULL);

        // Update the cache
        m_cache.useVertexCache = useVertexCache;
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::pushGLStates()
{
	if (activate(true))
    {
        // TODO: implement pushGlStates
    }
    resetGLStates();
}


////////////////////////////////////////////////////////////
void RenderTarget::popGLStates()
{
    if (activate(true))
    {
        // TODO: implement popGLStates
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::resetGLStates()
{
	// Check here to make sure a context change does not happen after activate(true)
    bool shaderAvailable = Shader::isAvailable();

    if (activate(true))
    {
        m_cache.glStatesSet = true;

        // Apply the default SFML states
        applyBlendMode(BlendAlpha);
        applyTransform(Transform::Identity);
        applyTexture(NULL);
        if (shaderAvailable)
            applyShader(NULL);

        m_cache.useVertexCache = false;

        // Set the default view
        setView(getView());
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::initialize()
{
    // Setup the default and current views
    m_defaultView.reset(FloatRect(0, 0, static_cast<float>(getSize().x), static_cast<float>(getSize().y)));
    m_view = m_defaultView;

    // Set GL states only on first draw, so that we don't pollute user's states
    m_cache.glStatesSet = false;
}


////////////////////////////////////////////////////////////
C3D_RenderTarget* RenderTarget::getCitroTarget()
{
    return m_target;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyCurrentView()
{
	// Set the viewport
    IntRect viewport = getViewport(m_view);
    int top = getSize().y - (viewport.top + viewport.height);
    C3D_SetViewport(viewport.left, top, viewport.height, viewport.width);

	// Set the projection matrix
    memcpy(MtxStack_Cur(CitroGetProjectionMatrix())->m, m_view.getTransform().getMatrix(), sizeof(C3D_Mtx));

    m_cache.viewChanged = false;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyBlendMode(const BlendMode& mode)
{
    // Apply the blend mode
    C3D_AlphaBlend(equationToGlConstant(mode.colorEquation),
                   equationToGlConstant(mode.alphaEquation),
                   factorToGlConstant(mode.colorSrcFactor),
                   factorToGlConstant(mode.colorDstFactor),
                   factorToGlConstant(mode.alphaSrcFactor),
                   factorToGlConstant(mode.alphaDstFactor));

    m_cache.lastBlendMode = mode;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyTransform(const Transform& transform)
{
    memcpy(MtxStack_Cur(CitroGetModelviewMatrix())->m, transform.getMatrix(), sizeof(C3D_Mtx));

}


////////////////////////////////////////////////////////////
void RenderTarget::applyTexture(const Texture* texture)
{
    Texture::bind(texture, Texture::Pixels);

    m_cache.lastTextureId = texture ? texture->m_cacheId : 0;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyShader(const Shader* shader)
{
    Shader::bind(shader);
}

} // namespace cpp3ds


////////////////////////////////////////////////////////////
// Render states caching strategies
//
// * View
//   If SetView was called since last draw, the projection
//   matrix is updated. We don't need more, the view doesn't
//   change frequently.
//
// * Transform
//   The transform matrix is usually expensive because each
//   entity will most likely use a different transform. This can
//   lead, in worst case, to changing it every 4 vertices.
//   To avoid that, when the vertex count is low enough, we
//   pre-transform them and therefore use an identity transform
//   to render them.
//
// * Blending mode
//   Since it overloads the == operator, we can easily check
//   whether any of the 6 blending components changed and,
//   thus, whether we need to update the blend mode.
//
// * Texture
//   Storing the pointer or OpenGL ID of the last used texture
//   is not enough; if the cpp3ds::Texture instance is destroyed,
//   both the pointer and the OpenGL ID might be recycled in
//   a new texture instance. We need to use our own unique
//   identifier system to ensure consistent caching.
//
// * Shader
//   Shaders are very hard to optimize, because they have
//   parameters that can be hard (if not impossible) to track,
//   like matrices or textures. The only optimization that we
//   do is that we avoid setting a null shader if there was
//   already none for the previous draw.
//
////////////////////////////////////////////////////////////
