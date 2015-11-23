
#ifndef _MORPHSNAKES_H
#define _MORPHSNAKES_H

namespace morphsnakes
{

namespace detail
{

static const int curv_operator_2d[4][2] = {{0, 8},
                                           {1, 7},
                                           {2, 6},
                                           {3, 5}};

}

template<class T, int D>
class ndimage
{
public:
    T* data;
    int shape[D];
    int stride[D];
};

// Narrow band cell
template<int D>
class cell
{
public:
    const int position[D];
    
private:
    ndimage<bool, D>* image;
    
    bool toggle;
    int boundaries;
    bool dirty;
};

template<int D>
class embedding_function;

template<>
class embedding_function<2>
{
    static const int num_neighbors = 9;
    
private:
    ndimage<bool, 2>* image;
    int neighborhood[num_neighbors];
    
public:
    embedding_function(ndimage<bool, 2>* _image)
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
