
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

typedef int Position;

template<int D>
class Neighborhood;

template<>
class Neighborhood<2>
{
public:
    static const int num_neighbors = 9;
    static constexpr int offsets[num_neighbors][2] = {{-1, -1}, {-1, 0}, {-1, 1},
                                                        {0, -1}, {0,0}, {0, 1},
                                                        {1, -1}, {1, 0}, {1, 1}};
    std::array<Position, num_neighbors> neighbors;

public:
    Neighborhood(const std::array<int, 2>& stride)
    {
        for(int i = 0; i < num_neighbors; ++i)
            neighbors[i] = offsets[i][0] * stride[1] + offsets[i][1] * stride[0];
    }
    
    std::array<Position, num_neighbors> getNeighbors(const Position& pos) const
    {
        std::array<Position, num_neighbors> res;
        for(int i = 0; i < num_neighbors; ++i)
            res[i] = neighbors[i] + pos;
        
        return res;
    }
};

constexpr int Neighborhood<2>::offsets[Neighborhood<2>::num_neighbors][2];

template<class T, int D>
class NDImage;

template<class T, int D>
class NDImageIterator
{
public:
    NDImageIterator(const NDImage<T, D>& image, bool atEnd=false)
        : image(image)
        , position(atEnd ? -1 : 0)
    {
        if(!atEnd)
        {
            cursor.fill(1);
            position = std::inner_product(cursor.begin(), cursor.end(), image.stride.begin(), 0);
        }
    }
    
    NDImageIterator& operator++()
    {
        int i;
        for(i = D - 1; i >= 0; --i)
        {
            if(cursor[i] < image.shape[i] - 2)
            {
                ++cursor[i];
                break;
            }
            else
                cursor[i] = 1;
        }
        
        if(i == -1)
            position = -1;
        else
            position = std::inner_product(cursor.begin(), cursor.end(), image.stride.begin(), 0);
        
        return *this;
    }
    
    NDImageIterator operator++(int)
    {
        NDImageIterator aux(*this);
        
        ++(*this);
        
        return aux;
    }
    
    Position operator*() {return position;}
    
    bool operator==(const NDImageIterator& rhs) const
    {
        return position == rhs.position;
    }

    bool operator!=(const NDImageIterator& rhs) const
    {
        return position != rhs.position;
    }

public:
    const NDImage<T, D>& image;
    Position position;
    std::array<int, D> cursor;
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
        , neighborhood(stride)
    {}
    
    T& operator[](const Position& position)
    {
        return *reinterpret_cast<T*>(&data_bytes[position]);
    }
    
    const T& operator[](const Position& position) const
    {
        return *reinterpret_cast<T*>(&data_bytes[position]);
    }
    
    std::array<Position, Neighborhood<D>::num_neighbors> getNeighbors(const Position& position) const
    {
        return neighborhood.getNeighbors(position);
    }
    
    NDImageIterator<T, D> begin() const
    {
        return NDImageIterator<T, D>(*this);
    }
    
    NDImageIterator<T, D> end() const
    {
        return NDImageIterator<T, D>(*this, true);
    }
    
public:
    T* data;
    char* data_bytes;
    std::array<int, D> shape;
    std::array<int, D> stride;
    const Neighborhood<D> neighborhood;
};

// Narrow band cell
class Cell
{
public:
    Cell()
        : toggle(false)
    {}
//    const Position<D> position;
    bool toggle;
//    int boundaries;
//    bool dirty;
};

typedef std::map<Position, Cell > CellMap;

template<class T, int D>
CellMap createCellMap(const NDImage<T, D>& image)
{
    CellMap cellMap;
    
    for(auto position : image)
    {
        const T& val = image[position];
        
        for(auto n : image.getNeighbors(position))
        {
            if(image[n] != val)
            {
                cellMap[position] = Cell();
                break;
            }
        }
    }
    
    return cellMap;
}

template<int D>
class NarrowBand
{
public:
    typedef NDImage<int, D> Image;
    
    NarrowBand(Image& image)
        : _image(image), _cells(createCellMap(image))
    {}
    
    void toggleCell(const Position& position)
    {
        _cells[position].toggle = true;
    }
    
    void flush()
    {
        CellMap updatedCells;
        
        auto cellIt = _cells.begin();
        for(; cellIt != _cells.end(); ++cellIt)
        {
            const Position& position = cellIt->first;
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
            Position position = cellIt->first;
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
