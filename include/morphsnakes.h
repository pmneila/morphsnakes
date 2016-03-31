
#ifndef _MORPHSNAKES_H
#define _MORPHSNAKES_H

#include <map>
#include <array>
#include <vector>
#include <numeric>

namespace morphsnakes
{

namespace detail
{

static const int curv_operator_2d[4][2] = {{0, 8},
                                           {1, 7},
                                           {2, 6},
                                           {3, 5}};

}

// typedef int Position;
template<int D>
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

template<int D>
bool isBoundary(const std::array<int, D>& coords, const std::array<int, D>& shape)
{
    for(int i = 0; i < D; ++i)
    {
        if(coords[i] == 0 || coords[i] == shape[i]-1)
            return true;
    }
    return false;
}

template<int D>
bool isBoundary(const Position<D>& position, const std::array<int, D>& shape)
{
    return isBoundary<D>(position.coord, shape);
}

template<int D>
std::array<int, D> offsetToCoord(int offset, const std::array<int, D>& stride)
{
    std::array<int, D> res;
    std::transform(stride.begin(), stride.end(), res.begin(),
                    [&](int s){return offset / s;});
    return res;
}

template<int D>
int coordToOffset(const std::array<int, D>& coord, const std::array<int, D>& stride)
{
    return std::inner_product(coord.begin(), coord.end(), stride.begin(), 0);
}

template<int D>
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
            linear_offsets[i] = coord_offsets[i][0] * stride[1] + coord_offsets[i][1] * stride[0];
    }
};

constexpr NeighborOffsets<2>::CoordOffsets NeighborOffsets<2>::coord_offsets;

template<class T, int D>
class NDImage;

template<int D>
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

template<int D>
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
        Coord new_coord;
        
        const Coord& coord_offset = *coord_iterator;
        for(int i = 0; i < D; ++i)
        {
            new_coord[i] = center.coord[i] + coord_offset[i];
        }
        
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

template<int D>
class Neighborhood
{
public:
    Neighborhood(const Position<D>& center, const NeighborOffsets<D>& offsets)
        : center(center)
        , offsets(offsets)
    {}
    
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

template<class T, int D>
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

template<int D>
class NarrowBand
{
public:
    
    typedef std::map<Position<D>, Cell> CellMap;
    
    template<class T>
    CellMap createCellMap(const NDImage<T, D>& image)
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
            
            auto neighbors = _image.neighborhood.getNeighbors(position);
            for(auto n : neighbors)
                updatedCells[n] = Cell();
        }
        
        _cells.insert(updatedCells.begin(), updatedCells.end());
    }
    
    void prune()
    {
        auto cellIt = _cells.begin();
        for(; cellIt != _cells.end(); ++cellIt)
        {
            const Position<D>& position = cellIt->first;
            auto val = _image[position];
            
            // for(auto n : _image.getNeighbors(position))
            // {
            //     if(_image[n] != val)
            //         toRemove.insert(position);
            // }
        }
    }
    
    const CellMap& getCellMap() const {return _cells;}
    
private:
    Image& _image;
    CellMap _cells;
};

template<int D>
class embedding_function;

template<>
class embedding_function<2>
{
    static const int num_neighbors = 9;
    
private:
    NDImage<bool, 2>* image;
    int neighborhood[num_neighbors];
    
public:
    embedding_function(NDImage<bool, 2>* _image)
        : image(_image)
    {
        static const int offsets[][2] = {{-1, -1}, {-1, 0}, {-1, 1},
                                         {0, -1}, {0, 0}, {0, 1},
                                         {1, -1}, {1, 0}, {1, 1}};
        
        for(int i = 0; i < num_neighbors; ++i)
            neighborhood[i] = offsets[i][0] * image->stride[1] + offsets[i][1] * image->stride[0];
    }
    
    void curv_op(bool SI = false)
    {
        
        for(int i = 1; i < image->shape[0] - 1; ++i)
        {
            for(int j = 1; j < image->shape[1] - 1; ++j)
            {
                int offset = i * image->stride[1] + j * image->stride[0];
                
                bool patch[num_neighbors];
                for(int p_i = 0; p_i < num_neighbors; ++p_i)
                    patch[p_i] = image->data[offset + neighborhood[p_i]];
                
                
            }
        }
    }

};

}

#endif // _MORPHSNAKES_H
