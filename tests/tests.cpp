
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

CImg<double> gborders(const CImg<double>& img, double alpha, double sigma)
{
    // Gaussian gradient magnitude
    auto gaussian_blur = img.get_blur(sigma, sigma, sigma, true, true);
    CImgList<double> grads = gaussian_blur.get_gradient("xy", 0);
    auto gaussian_gradient_magnitude = (grads[0].get_sqr() + grads[1].get_sqr()).get_sqrt();
    
    auto res = (1.0 + alpha * gaussian_gradient_magnitude).get_sqrt().get_pow(-1);
    return res;
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
ms::NDImage<T, 2> cimg2ndimage(CImg<T>& img)
{
    ms::Shape<2> shape = {img.height(), img.width()};
    ms::Stride<2> stride = {img.width() * sizeof(T), sizeof(T)};
    
    return ms::NDImage<T, 2>(img.data(), shape, stride);
}

void lakes()
{
    CImg<double> img = rgb2gray(CImg<double>("../testimages/lakes3.jpg")) / 255.0;
    auto embedding = circle_levelset(img.height(), img.width(), {80, 170}, 25);
        
    (embedding * 255).save_png("lakes_begin.png");
    // Morphological ACWE
    ms::MorphACWE<double, 2> macwe(cimg2ndimage(embedding), cimg2ndimage(img), 3);
    for(int i = 0; i < 200; ++i)
        macwe.step();
    // Save results
    (embedding * 255).save_png("lakes_end.png");
}

void starfish()
{
    // Load image
    CImg<double> img = rgb2gray(CImg<double>("../testimages/seastar2.png")) / 255.0;
    auto embedding = circle_levelset(img.height(), img.width(), {163, 137}, 135);
    (embedding * 255).save_png("seastar_begin.png");
    
    // Compute borders and gradients
    auto gimg = gborders(img, 1000.0, 2.0);
    CImgList<double> grad_gimg = gimg.get_gradient("yx", 0);
    std::array<ms::NDImage<double, 2>, 2> grads = {cimg2ndimage(grad_gimg[0]), cimg2ndimage(grad_gimg[1])};
    
    // Morphological GAC
    ms::MorphGAC<double, 2> mgac(cimg2ndimage(embedding), cimg2ndimage(gimg), grads, 2, 0.4, -1);
    for(int i = 0; i < 110; ++i)
        mgac.step();
    
    // Save results
    (embedding * 255).save_png("seastar_end.png");
}

int main()
{
    starfish();
    lakes();
    return 0;
}
