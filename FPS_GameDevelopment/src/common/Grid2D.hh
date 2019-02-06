#pragma once

#include <vector>

template<typename T>
class Grid2D
{
public:
    using container_type = std::vector<T>;
    using value_type = T;

    explicit Grid2D(std::size_t width, std::size_t height) :
        mWidth(width),
        mHeight(height),
        mData(width * height)
    {
    }

    /// access element
    T& operator()(std::size_t x, std::size_t y)
    {
        return mData[flattenCoord(x, y)];
    }

    /// read element
    T const& operator()(std::size_t x, std::size_t y) const
    {
        return mData[flattenCoord(x, y)];
    }

    std::size_t width() const
    {
        return mWidth;
    }

    std::size_t height() const
    {
        return mHeight;
    }

    /// read underlying container
    container_type const& container() const
    {
        return mData;
    }

protected:
    /// convert coordinate pair to internal (flat) representation
    std::size_t flattenCoord(std::size_t x, std::size_t y) const
    {
        return mWidth * y + x;
    }

    std::size_t mWidth;
    std::size_t mHeight;
    container_type mData;
};
