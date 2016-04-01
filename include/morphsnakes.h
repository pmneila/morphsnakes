
#ifndef _MORPHSNAKES_H
#define _MORPHSNAKES_H

#include <map>
#include <array>
#include <vector>
#include <numeric>
#include <iostream>

namespace morphsnakes
{

template<size_t D>
std::array<int, D> operator+(const std::array<int, D>& a, const std::array<int, D>& b)
{
    std::array<int, D> res;
    for(int i = 0; i < D; ++i)
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
        : coord(coords)
        , offset(offset)
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
bool isBoundary(const std::array<int, D>& coords, const std::array<int, D>& shape)
{
    for(int i = 0; i < D; ++i)
    {
        if(coords[i] == 0 || coords[i] == shape[i]-1)
            return true;
    }
    return false;
}

template<size_t D>
bool isBoundary(const Position<D>& position, const std::array<int, D>& shape)
{
    return isBoundary<D>(position.coord, shape);
}

template<size_t D>
std::array<int, D> offsetToCoord(int offset, const std::array<int, D>& stride)
{
    std::array<int, D> res;
    std::transform(stride.begin(), stride.end(), res.begin(),
                    [&](int s){return offset / s;});
    return res;
}

template<size_t D>
int coordToOffset(const std::array<int, D>& coord, const std::array<int, D>& stride)
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
        {{-1, -1}, {-1, 0}, {-1, 1},
        {0, -1}, {0,0}, {0, 1},
        {1, -1}, {1, 0}, {1, 1}}
    };
    LinearOffsets linear_offsets;

public:
    NeighborOffsets(const std::array<int, 2>& stride)
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
        {{-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1},
         {-1, 0, -1}, {-1, 0, 0}, {-1, 0, 1},
         {-1, 1, -1}, {-1, 1, 0}, {-1, 1, 1},
         {0, -1, -1}, {0, -1, 0}, {0, -1, 1},
         {0, 0, -1}, {0, 0, 0}, {0, 0, 1},
         {0, 1, -1}, {0, 1, 0}, {0, 1, 1},
         {1, -1, -1}, {1, -1, 0}, {1, -1, 1},
         {1, 0, -1}, {1, 0, 0}, {1, 0, 1},
         {1, 1, -1}, {1, 1, 0}, {1, 1, 1}}
    };
    LinearOffsets linear_offsets;

public:
    NeighborOffsets(const std::array<int, 2>& stride)
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
    NDImageIterator(const std::array<int, D>& shape,
                    const std::array<int, D>& stride,
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
    const std::array<int, D> shape;
    const std::array<int, D> stride;
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
    NDImage(T* data, const std::array<int, D>& shape, const std::array<int, D>& stride)
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
    const std::array<int, D> shape;
    const std::array<int, D> stride;
    const NeighborOffsets<D> neighbor_offsets;
};

// Narrow band cell
class Cell
{
public:
    Cell()
        : toggle(false)
    {}
    bool toggle;
};

template<size_t D>
class NarrowBand
{
public:
    
    typedef std::map<Position<D>, Cell> CellMap;
    
    template<class T>
    static CellMap createCellMap(const NDImage<T, D>& image)
    {
        CellMap cellMap;
        
        for(auto pixel : image)
        {
            if(isBoundary<D>(pixel, image.shape))
                continue;
            
            const T& val = image[pixel];
            
            for(auto n : image.neighborhood(pixel))
            {
                if(image[n] != val)
                {
                    cellMap[pixel] = Cell();
                    break;
                }
            }
        }
        
        return cellMap;
    }
    
    typedef NDImage<int, D> Image;
    
    NarrowBand(Image& image)
        : _image(image), _cells(createCellMap(image))
    {}
    
    void toggleCell(const Position<D>& position)
    {
        _cells[position].toggle = true;
    }
    
    void flush()
    {
        CellMap updatedCells;
        
        auto cellIt = _cells.begin();
        for(; cellIt != _cells.end(); ++cellIt)
        {
            const Position<D>& position = cellIt->first;
            if(!cellIt->second.toggle)
                continue;
            
            _image[position] = !_image[position];
            cellIt->second.toggle = false;
            
            for(auto n : _image.neighborhood(position))
            {
                // Don't add boundary pixels to the NarrowBand.
                if(isBoundary<D>(n, _image.shape))
                    continue;
                
                updatedCells[n] = Cell();
            }
        }
        
        _cells.insert(updatedCells.begin(), updatedCells.end());
    }
    
    void prune()
    {
        auto cellIt = _cells.begin();
        while(cellIt != _cells.end())
        {
            const Position<D>& position = cellIt->first;
            auto val = _image[position];
            
            bool shouldDelete = true;
            for(auto n : _image.neighborhood(position))
            {
                if(_image[n] != val)
                {
                    shouldDelete = false;
                    break;
                }
            }
            
            if(shouldDelete)
                cellIt = _cells.erase(cellIt);
            else
                ++cellIt;
        }
    }
    
    const CellMap& getCellMap() const {return _cells;}
    const Image& getImage() const {return _image;}
    
private:
    Image& _image;
    CellMap _cells;
};

// Descriptors of morphological operators

template<size_t num_elements, size_t num_neighbors>
using OperatorDescriptor = std::array<std::array<int, num_neighbors>, num_elements>;

template<size_t D>
struct Operator;

template<>
struct Operator<2>
{
    static constexpr OperatorDescriptor<4, 2> curvature  = {
        {{0, 8},
        {1, 7},
        {2, 6},
        {3, 5}}
    };

    static constexpr OperatorDescriptor<1, 8> dilate_erode = {
        {{0, 1, 2, 3, 5, 6, 7, 8}}
    };
};

constexpr OperatorDescriptor<4, 2> Operator<2>::curvature;
constexpr OperatorDescriptor<1, 8> Operator<2>::dilate_erode;

template<>
struct Operator<3>
{
    static constexpr OperatorDescriptor<9, 8> curvature  = {
        {{6, 7, 8, 12, 14, 18, 19, 20},
        {9, 10, 11, 12, 14, 15, 16, 17},
        {0, 1, 2, 12, 14, 24, 25, 26},
        {0, 4, 8, 9, 17, 18, 22, 26},
        {3, 4, 5, 12, 14, 21, 22, 23},
        {2, 4, 6, 11, 15, 20, 22, 24},
        {2, 5, 8, 10, 16, 18, 21, 24},
        {1, 4, 7, 10, 16, 19, 22, 25},
        {0, 3, 6, 10, 16, 20, 23, 26}
        }
    };

    static constexpr OperatorDescriptor<1, 26> dilate_erode = {
        {{0, 1, 2, 3, 4, 5, 6, 7, 8,
          9,10,11,12,14,15,16,17,
         18,19,20,21,22,23,24,25,26}}
    };
};

constexpr OperatorDescriptor<9, 8> Operator<3>::curvature;
constexpr OperatorDescriptor<1, 26> Operator<3>::dilate_erode;

/**
 * Morphological operator acting over a narrow band.
 */
template<class M, size_t D>
void morph_op(const M& op, bool inf_sup, NarrowBand<D>& narrowBand)
{
    typedef typename NarrowBand<D>::CellMap CellMap;
    typedef typename NarrowBand<D>::Image Image;
    
    const CellMap& cellMap = narrowBand.getCellMap();
    const Image& image = narrowBand.getImage();
    
    for(auto& cell : cellMap)
    {
        auto& val = image[cell.first];
        
        // If sup_inf and val is 0 or inf_sup and val is 1, then no change is possible.
        if(val == inf_sup)
            continue;
        
        const Neighborhood<D>& neighborhood = image.neighborhood(cell.first);
        bool shouldToggle = true;
        for(auto& elem : op)
        {
            bool active_element = false;
            for(auto& index : elem)
            {
                const Position<D>& n = neighborhood.getNeighbor(index);
                auto& val = image[n];
                
                if(val == inf_sup)
                {
                    active_element = true;
                    break;
                }
            }
            
            if(!active_element)
            {
                shouldToggle = false;
                break;
            }
        }
        
        if(shouldToggle)
            narrowBand.toggleCell(cell.first);
    }
}

// Common morphological operators: dilation, erosion and curvature

template<size_t D>
void dilate(NarrowBand<D>& narrowBand)
{
    morph_op(Operator<D>::dilate_erode, true, narrowBand);
}

template<size_t D>
void erode(NarrowBand<D>& narrowBand)
{
    morph_op(Operator<D>::dilate_erode, false, narrowBand);
}

template<size_t D>
void curv(bool inf_sup, NarrowBand<D>& narrowBand)
{
    morph_op(Operator<D>::curvature, inf_sup, narrowBand);
}

// Image attachment



}

#endif // _MORPHSNAKES_H
