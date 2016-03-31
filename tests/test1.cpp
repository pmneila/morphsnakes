
#include <algorithm>
#include <iostream>

#include "morphsnakes.h"

std::ostream& operator<<(std::ostream& ostr, const std::array<int, 2>& arr)
{
    ostr << "(" << arr[0] << ", " << arr[1] << ")";
    return ostr;
}

int main()
{
    int data[100];
    std::fill(data, data+100, 0);
    
    std::array<int, 2> shape = {10, 10};
    std::array<int, 2> strides = {sizeof(int) * 10, sizeof(int)};
    
    data[45] = 1;
    
    morphsnakes::NDImage<int, 2> image(data, shape, strides);
    
    for(auto aux : image)
    {
        std::cout << aux.offset << " " << aux.coord << " " << image[aux.offset] << std::endl;
        if(morphsnakes::isBoundary<2>(aux, image.shape))
            continue;
        for(auto n : image.neighborhood(aux))
        {
            std::cout << "\t" << n.offset << " " << n.coord << " " << image[n] << std::endl;
        }
        // for(auto n : image.getNeighbors(aux))
        //     std::cout << aux << " " << n << std::endl;
    }
    
    // morphsnakes::CellMap cellMap = createCellMap(image);
    // std::cout << cellMap << std::endl;
    morphsnakes::NarrowBand<2> narrowBand(image);
    // narrowBand.toggleCell(180);
    // for(auto c : narrowBand.getCellMap())
    // {
    //     std::cout << c.first << " " << c.second.toggle << std::endl;
    // }
    // narrowBand.flush();
    // for(auto c : narrowBand.getCellMap())
    // {
    //     std::cout << c.first << " " << c.second.toggle << std::endl;
    // }
    
    return 0;
}
