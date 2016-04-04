
#include <algorithm>
#include <iostream>

#include "morphsnakes.h"

#define cimg_display 0
#include "CImg.h"

namespace ms = morphsnakes;
using namespace cimg_library;

CImg<double> rgb2gray(const CImg<double>& img)
{
    return 0.2989*img.get_channel(0) + 0.587*img.get_channel(1) + 0.114*img.get_channel(2);
}

CImg<unsigned char> circle_levelset(int height, int width,
                                    const std::array<int, 2>& center,
                                    double radius,
                                    double scalerow=1.0)
{
    CImg<unsigned char> res(width, height);
    for(int i = 0; i < height; ++i)
    {
        for(int j = 0; j < width; ++j)
        {
            int diffy = (i - center[0]);
            int diffx = (j - center[1]);
            res(j, i) = (radius*radius - (diffx*diffx + diffy*diffy)) > 0;
        }
    }
    
    return res;
}

template<class T>
ms::NDImage<T, 2> cimg2ndimage(CImg<T> img)
{
    ms::Shape<2> shape = {img.height(), img.width()};
    ms::Stride<2> stride = {img.width() * sizeof(T), sizeof(T)};
    
    return ms::NDImage<T, 2>(img.data(), shape, stride);
}

int main()
{
    CImg<double> img = rgb2gray(CImg<double>("../testimages/lakes3.jpg")) / 255.0;
    auto embedding = circle_levelset(img.height(), img.width(), {80, 170}, 25);
    
    (embedding * 100).save_png("test1.png");
    ms::MorphACWE<double, 2> macwe(cimg2ndimage(embedding), cimg2ndimage(img), 3);
    
    for(int i = 0; i < 2; ++i)
    {
        // macwe.step();
    }
    (embedding * 100).save_png("test2.png");
    
    return 0;
}
