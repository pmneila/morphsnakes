
#ifndef _MORPHSNAKES_H
#define _MORPHSNAKES_H

#include <map>

#include "ndimage.h"

namespace morphsnakes
{

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
