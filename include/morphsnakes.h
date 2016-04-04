
#ifndef _MORPHSNAKES_H
#define _MORPHSNAKES_H

#include <map>
#include <cassert>

#include "ndimage.h"

namespace morphsnakes
{

template<size_t D>
using Embedding = NDImage<unsigned char, D>;

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
    
    NarrowBand(const Embedding<D>& embedding)
        : _embedding(embedding), _cells(createCellMap(embedding))
    {}
    
    void toggleCell(const Position<D>& position)
    {
        _cells[position].toggle = true;
    }
    
    void update()
    {
        CellMap updatedCells;
        
        auto cellIt = _cells.begin();
        for(; cellIt != _cells.end(); ++cellIt)
        {
            const Position<D>& position = cellIt->first;
            if(!cellIt->second.toggle)
                continue;
            
            _embedding[position] = !_embedding[position];
            cellIt->second.toggle = false;
            
            for(auto n : _embedding.neighborhood(position))
            {
                // Don't add boundary pixels to the NarrowBand.
                if(isBoundary<D>(n, _embedding.shape))
                    continue;
                
                updatedCells[n] = Cell();
            }
        }
        
        _cells.insert(updatedCells.begin(), updatedCells.end());
    }
    
    void cleanup()
    {
        auto cellIt = _cells.begin();
        while(cellIt != _cells.end())
        {
            const Position<D>& position = cellIt->first;
            auto val = _embedding[position];
            
            bool shouldDelete = true;
            for(auto n : _embedding.neighborhood(position))
            {
                if(_embedding[n] != val)
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
    const Embedding<D>& getEmbedding() const {return _embedding;}
    
protected:
    Embedding<D> _embedding;
    CellMap _cells;
};

template<class T, size_t D>
class ACWENarrowBand : public NarrowBand<D>
{
public:
    
    typedef typename NarrowBand<D>::CellMap CellMap;
    
    ACWENarrowBand(const Embedding<D>& embedding, const NDImage<T, D>& image)
        : NarrowBand<D>(embedding)
        , _image(image)
    {
        assert(embedding.shape == image.shape);
        
        initAverages(embedding, image);
    }
    
    void update()
    {
        CellMap updatedCells;
        
        auto cellIt = this->_cells.begin();
        for(; cellIt != this->_cells.end(); ++cellIt)
        {
            const Position<D>& position = cellIt->first;
            if(!cellIt->second.toggle)
                continue;
            
            typename Embedding<D>::DataType& val = this->_embedding[position];
            val = !val;
            
            updateAverages(position, val);
            
            // _embedding[position] = !_embedding[position];
            cellIt->second.toggle = false;
            
            for(auto n : this->_embedding.neighborhood(position))
            {
                // Don't add boundary pixels to the NarrowBand.
                if(isBoundary<D>(n, this->_embedding.shape))
                    continue;
                
                updatedCells[n] = Cell();
                // updatedCells.insert(typename CellMap::value_type(n, Cell()));
            }
        }
        
        this->_cells.insert(updatedCells.begin(), updatedCells.end());
    }
    
    double getAverageInside() const
    {
        return sum_in / static_cast<double>(count_in);
    }

    double getAverageOutside() const
    {
        return sum_out / static_cast<double>(count_out);
    }
    
    const NDImage<T, D>& getImage() const {return _image;}
    
private:
    void initAverages(const Embedding<D>& embedding, const NDImage<T, D>& image)
    {
        count_in = 0;
        count_out = 0;
        sum_in = 0.0;
        sum_out = 0.0;
        
        for(auto& position : embedding)
        {
            const auto& embeddingVal = embedding[position];
            // We need position.coord instead of position since
            const auto& imageVal = image[position.coord];
            
            if(embeddingVal == 0)
            {
                ++count_out;
                sum_out += imageVal;
            }
            else
            {
                ++count_in;
                sum_in += imageVal;
            }
        }
    }
    
    void updateAverages(const Position<D>& position, int newValue)
    {
        const auto& imageVal = _image[position.coord];
        
        if(newValue == 0)
        {
            --count_in;
            ++count_out;
            sum_in -= imageVal;
            sum_out += imageVal;
        }
        else
        {
            --count_out;
            ++count_in;
            sum_out -= imageVal;
            sum_in += imageVal;
        }
        
        assert(count_in >= 0 && count_out >= 0);
    }
    
protected:
    const NDImage<T, D> _image;
    int count_in, count_out;
    double sum_in, sum_out;
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
    const Embedding<D>& embedding = narrowBand.getEmbedding();
    
    for(auto& cell : narrowBand.getCellMap())
    {
        auto& val = embedding[cell.first];
        
        // If sup_inf and val is 0 or inf_sup and val is 1, then no change is possible.
        if(val == inf_sup)
            continue;
        
        const Neighborhood<D>& neighborhood = embedding.neighborhood(cell.first);
        bool shouldToggle = true;
        for(auto& elem : op)
        {
            bool active_element = false;
            for(auto& index : elem)
            {
                const Position<D>& n = neighborhood.getNeighbor(index);
                auto& val = embedding[n];
                
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
    
    narrowBand.update();
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

template<class T, size_t D>
void image_attachment_gac(NarrowBand<D>& narrowBand,
                            const std::array<NDImage<T, D>*, D> grads)
{
    const auto& embedding = narrowBand.getEmbedding();
    
    for(auto& cell : narrowBand.getCellMap())
    {
        const auto& position = cell.first;
        
        T dot_product = 0;
        for(int i = 0; i < D; ++i)
        {
            const T& grad_image_i = grads[i][position.coord];
            T u_next = embedding[position.offset + embedding.stride[i]];
            T u_prev = embedding[position.offset - embedding.stride[i]];
            T grad_u_i = u_next - u_prev;
            
            dot_product += grad_image_i * grad_u_i;
        }
        
        const auto& val = embedding[position];
        if((val == 1 && dot_product < 0) || (val == 0 && dot_product > 0))
            narrowBand.toggleCell(position);
    }
    
    narrowBand.update();
}

template<size_t D>
bool has_zero_gradient(const Embedding<D>& embedding,
                            const Position<D>& position)
{
    for(int i = 0; i < D; ++i)
    {
        const auto& u_next = embedding[position.offset + embedding.stride[i]];
        const auto& u_prev = embedding[position.offset - embedding.stride[i]];
        if(u_next != u_prev)
            return false;
    }
    
    return true;
}

template<class T, size_t D>
void image_attachment_acwe(ACWENarrowBand<T, D>& narrowBand, double lambda1, double lambda2)
{
    const auto& embedding = narrowBand.getEmbedding();
    const auto& image = narrowBand.getImage();
    
    double averageIn = narrowBand.getAverageInside();
    double averageOut = narrowBand.getAverageOutside();
    for(auto& cell : narrowBand.getCellMap())
    {
        const auto& position = cell.first;
        
        if(has_zero_gradient(embedding, position))
            continue;
        
        const auto& embeddingVal = embedding[position];
        const T& imageVal = image[position.coord];
        
        double diffIn = imageVal - averageIn;
        double diffOut = imageVal - averageOut;
        double criterion = lambda1 * diffIn * diffIn - lambda2 * diffOut * diffOut;
        if((embeddingVal == 0 && criterion < 0) || (embeddingVal == 1 && criterion > 0))
            narrowBand.toggleCell(position);
    }
    
    narrowBand.update();
}

template<class T, size_t D>
class MorphACWE
{
public:
    MorphACWE(const Embedding<D>& embedding, const NDImage<T, D>& image, int smoothing=1, double lambda1=1.0, double lambda2=1.0)
        : MorphACWE(ACWENarrowBand<T, D>(embedding, image), smoothing, lambda1, lambda2)
    {}
    
    MorphACWE(const ACWENarrowBand<T, D>& narrowBand, int smoothing=1, double lambda1=1.0, double lambda2=1.0)
        : _narrowBand(narrowBand)
        , _smoothing(smoothing)
        , _lambda1(lambda1)
        , _lambda2(lambda2)
        , _curv_is(false)
    {}
    
    void step()
    {
        // Image attachment
        image_attachment_acwe(_narrowBand, _lambda1, _lambda2);
        
        // Smoothing
        // for(int i = 0; i < _smoothing; ++i)
        // {
        //     curv(_curv_is, _narrowBand);
        //     _curv_is = !_curv_is;
        // }
        
        _narrowBand.cleanup();
    }
    
protected:
    ACWENarrowBand<T, D> _narrowBand;
    int _smoothing;
    double _lambda1;
    double _lambda2;
    bool _curv_is;
};

}

#endif // _MORPHSNAKES_H
