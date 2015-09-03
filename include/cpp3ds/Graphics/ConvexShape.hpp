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

#ifndef CPP3DS_CONVEXSHAPE_HPP
#define CPP3DS_CONVEXSHAPE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cpp3ds/Graphics/Shape.hpp>
#include <vector>


namespace cpp3ds
{
////////////////////////////////////////////////////////////
/// \brief Specialized shape representing a convex polygon
///
////////////////////////////////////////////////////////////
class ConvexShape : public Shape
{
public :

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \param pointCount Number of points of the polygon
    ///
    ////////////////////////////////////////////////////////////
    explicit ConvexShape(unsigned int pointCount = 0);

    ////////////////////////////////////////////////////////////
    /// \brief Set the number of points of the polygon
    ///
    /// \a count must be greater than 2 to define a valid shape.
    ///
    /// \param count New number of points of the polygon
    ///
    /// \see getPointCount
    ///
    ////////////////////////////////////////////////////////////
    void setPointCount(unsigned int count);

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of points of the polygon
    ///
    /// \return Number of points of the polygon
    ///
    /// \see setPointCount
    ///
    ////////////////////////////////////////////////////////////
    virtual unsigned int getPointCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the position of a point
    ///
    /// Don't forget that the polygon must remain convex, and
    /// the points need to stay ordered!
    /// setPointCount must be called first in order to set the total
    /// number of points. The result is undefined if \a index is out
    /// of the valid range.
    ///
    /// \param index Index of the point to change, in range [0 .. getPointCount() - 1]
    /// \param point New position of the point
    ///
    /// \see getPoint
    ///
    ////////////////////////////////////////////////////////////
    void setPoint(unsigned int index, const Vector3f& point);

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of a point
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    /// The result is undefined if \a index is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    ///
    /// \return Position of the index-th point of the polygon
    ///
    /// \see setPoint
    ///
    ////////////////////////////////////////////////////////////
    virtual Vector3f getPoint(unsigned int index) const;

private :

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::vector<Vector3f> m_points; ///< Points composing the convex polygon
};

} // namespace cpp3ds


#endif


////////////////////////////////////////////////////////////
/// \class cpp3ds::ConvexShape
/// \ingroup graphics
///
/// This class inherits all the functions of cpp3ds::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of cpp3ds::Shape (outline, color, texture, ...).
///
/// It is important to keep in mind that a convex shape must
/// always be... convex, otherwise it may not be drawn correctly.
/// Moreover, the points must be defined in order; using a random
/// order would result in an incorrect shape.
///
/// Usage example:
/// \code
/// cpp3ds::ConvexShape polygon;
/// polygon.setPointCount(3);
/// polygon.setPoint(0, cpp3ds::Vector2f(0, 0));
/// polygon.setPoint(1, cpp3ds::Vector2f(0, 10));
/// polygon.setPoint(2, cpp3ds::Vector2f(25, 5));
/// polygon.setOutlineColor(cpp3ds::Color::Red);
/// polygon.setOutlineThickness(5);
/// polygon.setPosition(10, 20);
/// ...
/// window.draw(polygon);
/// \endcode
///
/// \see cpp3ds::Shape, cpp3ds::RectangleShape, cpp3ds::CircleShape
///
////////////////////////////////////////////////////////////
