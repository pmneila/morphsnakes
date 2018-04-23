#ifndef _NARROWBAND_H
#define _NARROWBAND_H

#include <map>
#include <unordered_map>

#include "ndimage.h"

namespace std
{
// Hash function for the unordered_map
template<size_t D>
struct hash<morphsnakes::Position<D> >
{
    size_t operator()(const morphsnakes::Position<D>& position) const
    {
        return hash<int>()(position.offset);
    }
};

}

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
using Embedding = NDImage<unsigned char, D>;

template<size_t D>
using CellMap = std::unordered_map<Position<D>, Cell>;

template<size_t D>
class NarrowBand
{
public:
    
    template<class T>
    static CellMap<D> createCellMap(NDImage<T, D>& image)
    {
        CellMap<D> cellMap;
        
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
        : _embedding(embedding), _cells(createCellMap(_embedding))
    {}
    
    virtual ~NarrowBand() {}
    
    void toggleCell(const Position<D>& position)
    {
        _cells[position].toggle = true;
    }
    
    virtual void update()
    {
        CellMap<D> updatedCells;
        
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
    
    const CellMap<D>& getCellMap() const {return _cells;}
    const Embedding<D>& getEmbedding() const {return _embedding;}
    
protected:
    Embedding<D> _embedding;
    CellMap<D> _cells;
};

template<class T, size_t D>
class ACWENarrowBand : public NarrowBand<D>
{
public:
    
    ACWENarrowBand(const Embedding<D>& embedding, const NDImage<T, D>& image)
        : NarrowBand<D>(embedding)
        , _image(image)
    {
        assert(embedding.shape == image.shape);
        
        initAverages(embedding, image);
    }
    
    virtual void update()
    {
        CellMap<D> updatedCells;
        
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
    
    inline double getAverageInside() const
    {
        return sum_in / static_cast<double>(count_in);
    }

    inline double getAverageOutside() const
    {
        return sum_out / static_cast<double>(count_out);
    }
    
    inline int getCountIn() const {return count_in;}
    inline int getCountOut() const {return count_out;}
    inline double getSumIn() const {return sum_in;}
    inline double getSumOut() const {return sum_out;}
    
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

}

#endif // _NARROWBAND_H
