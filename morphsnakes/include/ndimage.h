
#ifndef _NDIMAGE_H
#define _NDIMAGE_H

#include <iostream>
#include <array>
#include <numeric>
#include <algorithm>

namespace morphsnakes
{

template<size_t D>
using Shape = std::array<int, D>;

template<size_t D>
using Stride = std::array<size_t, D>;

template<size_t D>
std::array<int, D> operator+(const std::array<int, D>& a, const std::array<int, D>& b)
{
    std::array<int, D> res;
    for(size_t i = 0; i < D; ++i)
        res[i] = a[i] + b[i];
    return res;
}

// typedef int Position;
template<size_t D>
class Position
{
public:
    typedef std::array<int, D> Coord;
    
    Position(const Coord& coords, int offset)
        : offset(offset)
        , coord(coords)
    {}
    
    int offset;
    Coord coord;
    
    bool operator<(const Position& rhs) const
    {
        return offset < rhs.offset;
    }
    
    bool operator==(const Position& rhs) const
    {
        return offset == rhs.offset;
    }
};

std::ostream& operator<<(std::ostream& ostr, const morphsnakes::Position<2>& pos)
{
    ostr << pos.offset << "(" << pos.coord[0] << ", " << pos.coord[1] << ")";
    return ostr;
}

template<size_t D>
bool isBoundary(const std::array<int, D>& coords, const Shape<D>& shape)
{
    for(size_t i = 0; i < D; ++i)
    {
        if(coords[i] == 0 || coords[i] == shape[i]-1)
            return true;
    }
    return false;
}

template<size_t D>
bool isBoundary(const Position<D>& position, const Shape<D>& shape)
{
    return isBoundary<D>(position.coord, shape);
}

template<size_t D>
std::array<int, D> offsetToCoord(int offset, const Stride<D>& stride)
{
    std::array<int, D> res;
    std::transform(stride.begin(), stride.end(), res.begin(),
                    [&](int s){return offset / s;});
    return res;
}

template<size_t D>
int coordToOffset(const std::array<int, D>& coord, const Stride<D>& stride)
{
    return std::inner_product(coord.begin(), coord.end(), stride.begin(), 0);
}

template<size_t D>
class NeighborOffsets;

template<>
class NeighborOffsets<2>
{
public:
    static const int num_neighbors = 9;

    typedef Position<2>::Coord Coord;
    typedef std::array<Coord, num_neighbors> CoordOffsets;
    typedef std::array<int, num_neighbors> LinearOffsets;
    
    static constexpr CoordOffsets coord_offsets = {
         {Coord {-1, -1}, Coord {-1, 0}, Coord {-1, 1},
          Coord {0, -1}, Coord {0, 0}, Coord {0, 1},
          Coord {1, -1}, Coord {1, 0}, Coord {1, 1}}
    };
    LinearOffsets linear_offsets;

public:
    NeighborOffsets(const Stride<2>& stride)
    {
        for(int i = 0; i < num_neighbors; ++i)
            linear_offsets[i] = coord_offsets[i][1] * stride[1] + coord_offsets[i][0] * stride[0];
    }
};

constexpr NeighborOffsets<2>::CoordOffsets NeighborOffsets<2>::coord_offsets;

template<>
class NeighborOffsets<3>
{
public:
    static const int num_neighbors = 27;
    
    typedef Position<3>::Coord Coord;
    typedef std::array<Coord, num_neighbors> CoordOffsets;
    typedef std::array<int, num_neighbors> LinearOffsets;
    
    static constexpr CoordOffsets coord_offsets = {
        {Coord {-1, -1, -1}, Coord {-1, -1, 0}, Coord {-1, -1, 1},
         Coord {-1, 0, -1}, Coord {-1, 0, 0}, Coord {-1, 0, 1},
         Coord {-1, 1, -1}, Coord {-1, 1, 0}, Coord {-1, 1, 1},
         Coord {0, -1, -1}, Coord {0, -1, 0}, Coord {0, -1, 1},
         Coord {0, 0, -1}, Coord {0, 0, 0}, Coord {0, 0, 1},
         Coord {0, 1, -1}, Coord {0, 1, 0}, Coord {0, 1, 1},
         Coord {1, -1, -1}, Coord {1, -1, 0}, Coord {1, -1, 1},
         Coord {1, 0, -1}, Coord {1, 0, 0}, Coord {1, 0, 1},
         Coord {1, 1, -1}, Coord {1, 1, 0}, Coord {1, 1, 1}}
    };
    LinearOffsets linear_offsets;

public:
    NeighborOffsets(const Stride<3>& stride)
    {
        for(int i = 0; i < num_neighbors; ++i)
            linear_offsets[i] = coord_offsets[i][2] * stride[2] +
                                coord_offsets[i][1] * stride[1] +
                                coord_offsets[i][0] * stride[0];
    }
};

constexpr NeighborOffsets<3>::CoordOffsets NeighborOffsets<3>::coord_offsets;

template<class T, size_t D>
class NDImage;

template<size_t D>
class NDImageIterator
{
public:
    NDImageIterator(const Shape<D>& shape,
                    const Stride<D>& stride,
                    bool atEnd=false)
        : shape(shape)
        , stride(stride)
        , position(std::array<int, D>(), 0)
    {
        position.coord.fill(0);
        position.offset = atEnd ? -1 : 0;
    }
    
    NDImageIterator& operator++()
    {
        int i;
        for(i = D - 1; i >= 0; --i)
        {
            if(position.coord[i] < shape[i] - 1)
            {
                ++position.coord[i];
                break;
            }
            else
                position.coord[i] = 0;
        }
        
        if(i == -1)
            position.offset = -1;
        else
            position.offset = coordToOffset<D>(position.coord, stride);
        
        return *this;
    }
    
    NDImageIterator operator++(int)
    {
        NDImageIterator aux(*this);
        
        ++(*this);
        
        return aux;
    }
    
    const Position<D>& operator*() const
    {return position;}
    
    bool operator==(const NDImageIterator& rhs) const
    {
        return position.offset == rhs.position.offset;
    }

    bool operator!=(const NDImageIterator& rhs) const
    {
        return position.offset != rhs.position.offset;
    }

public:
    const Shape<D> shape;
    const Stride<D> stride;
    Position<D> position;
};

template<size_t D>
class NeighborhoodIterator
{
public:
    
    typedef typename Position<D>::Coord Coord;
    typedef typename NeighborOffsets<D>::CoordOffsets::const_iterator CoordOffsetsIt;
    typedef typename NeighborOffsets<D>::LinearOffsets::const_iterator LinearOffsetsIt;
    
    NeighborhoodIterator(const Position<D>& center,
                         const CoordOffsetsIt& coord_iterator,
                         const LinearOffsetsIt& linear_iterator)
        : center(center)
        , coord_iterator(coord_iterator)
        , linear_iterator(linear_iterator)
    {}
    
    NeighborhoodIterator& operator++()
    {
        ++coord_iterator;
        ++linear_iterator;
        return *this;
    }
    
    NeighborhoodIterator operator++(int)
    {
        NeighborhoodIterator aux(*this);
        
        ++(*this);
        
        return aux;
    }
    
    Position<D> operator*()
    {
        Coord new_coord = center.coord + *coord_iterator;
        int new_offset = center.offset + *linear_iterator;
        return Position<D>(new_coord, new_offset);
    }
    
    bool operator==(const NeighborhoodIterator& rhs) const
    {
        return this->linear_iterator == rhs.linear_iterator;
    }
    
    bool operator!=(const NeighborhoodIterator& rhs) const
    {
        return this->linear_iterator != rhs.linear_iterator;
    }
    
private:
    const Position<D>& center;
    CoordOffsetsIt coord_iterator;
    LinearOffsetsIt linear_iterator;
};

template<size_t D>
class Neighborhood
{
public:
    Neighborhood(const Position<D>& center, const NeighborOffsets<D>& offsets)
        : center(center)
        , offsets(offsets)
    {}
    
    Position<D> getNeighbor(int index) const
    {
        const typename Position<D>::Coord& coord_offset = offsets.coord_offsets[index];
        int linear_offset = offsets.linear_offsets[index];
        
        return Position<D>(center.coord + coord_offset, center.offset + linear_offset);
    }
    
    NeighborhoodIterator<D> begin()
    {
        return NeighborhoodIterator<D>(center, offsets.coord_offsets.begin(), offsets.linear_offsets.begin());
    }
    
    NeighborhoodIterator<D> end()
    {
        return NeighborhoodIterator<D>(center, offsets.coord_offsets.end(), offsets.linear_offsets.end());
    }
    
private:
    const Position<D>& center;
    const NeighborOffsets<D>& offsets;
};

template<class T, size_t D>
class NDImage
{
public:
    
    typedef T DataType;
    
    NDImage(T* data, const Shape<D>& shape, const Stride<D>& stride)
        : data(data)
        , data_bytes(reinterpret_cast<char*>(data))
        , shape(shape)
        , stride(stride)
        , neighbor_offsets(stride)
    {}
    
    T& operator[](int offset)
    {
        return *reinterpret_cast<T*>(&data_bytes[offset]);
    }
    
    const T& operator[](int offset) const
    {
        return *reinterpret_cast<T*>(&data_bytes[offset]);
    }
    
    T& operator[](const Position<D>& position)
    {
        return this->operator[](position.offset);
    }
    
    const T& operator[](const Position<D>& position) const
    {
        return this->operator[](position.offset);
    }
    
    T& operator[](const typename Position<D>::Coord coord)
    {
        int offset = std::inner_product(coord.begin(), coord.end(), stride.begin(), 0);
        return this->operator[](offset);
    }
    
    const T& operator[](const typename Position<D>::Coord coord) const
    {
        int offset = std::inner_product(coord.begin(), coord.end(), stride.begin(), 0);
        return this->operator[](offset);
    }
    
    Neighborhood<D> neighborhood(const Position<D>& center) const
    {
        return Neighborhood<D>(center, neighbor_offsets);
    }
    
    NDImageIterator<D> begin() const
    {
        return NDImageIterator<D>(shape, stride);
    }
    
    NDImageIterator<D> end() const
    {
        return NDImageIterator<D>(shape, stride, true);
    }
    
public:
    T* data;
    char* data_bytes;
    const Shape<D> shape;
    const Stride<D> stride;
    const NeighborOffsets<D> neighbor_offsets;
};

}

#endif // _NDIMAGE_H
