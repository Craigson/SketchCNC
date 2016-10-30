//
//  ImageProcessor.cpp
//  SketchCNC
//
//  Created by Craig Pickard on 5/6/16.
//
//

#include "ImageProcessor.hpp"

ImageProcessor::ImageProcessor():
threshold(128),
numX(0),
numY(0),
mScale(1.f),
pixelSize(5),
capturingImage(false),
imageWidth(640),
imageHeight(480),
showPixels(false),
mCanvasPos(ci::ivec2(50,50)),
displayLoaded(false)
{}

ImageProcessor::~ImageProcessor(){}


void ImageProcessor::loadImage()
{
    try {
        ci::fs::path path = ci::app::getOpenFilePath("");
        
        
        auto img = ci::loadImage(ci::loadFile(path));
        //SPLIT THE STRING GENERATED FROM THE CSV FILE INTO LINES
        
        mPixels = ci::Surface(ci::loadImage(ci::loadFile(path)));
        imageWidth = mPixels.getWidth();
        imageHeight = mPixels.getHeight();
        
        numX = mPixels.getWidth() / pixelSize;
        numY = mPixels.getHeight() / pixelSize;
        
        mTexture->create(mPixels);
        
        for (int i = 0; i < mPixels.getWidth(); i += pixelSize)
        {
            for (int j = 0; j < mPixels.getHeight(); j += pixelSize)
            {
                mThresholdValues.push_back(getAverage(&mPixels, ci::vec2(i,j)));
                mPixelPositions.push_back(ci::ivec2(i,j));
            }
        }
        
        fileLoaded = true;
    }
    catch (ci::Exception &exc) {
        CI_LOG_EXCEPTION("failed to load image file", exc);
    }
}

void ImageProcessor::initCamera()
{
    try {
        mCapture = ci::Capture::create( 640, 480 );
        mCapture->start();
    }
    catch( ci::CaptureExc &exc ) {
        CI_LOG_EXCEPTION( "failed to initialize the Capture: ", exc );
    }
    

    
    numX = mCapture->getWidth() / pixelSize;
    numY = mCapture->getHeight() / pixelSize;
    
    imageWidth = mCapture->getWidth();
    imageHeight = mCapture->getHeight();
}




int ImageProcessor::getAverage( ci::Surface* _surface, ci::vec2 _cornerPoint)
{
    
    
    ci::Area area = ci::Area(_cornerPoint.x, _cornerPoint.y, _cornerPoint.x + pixelSize, _cornerPoint.y + pixelSize);
    
    int32_t sumR,sumG,sumB;
    
    sumR = 0;
    sumG = 0;
    sumB = 0;
    
    int count = 0;
    
    ci::Surface::Iter iter = _surface->getIter( area );
    
    while( iter.line() ) {
        while( iter.pixel() ) {
            
            sumR += (int)iter.r();
            sumG += (int)iter.g();
            sumB += (int)iter.b();
            
            count++;
        }
    }
    
    int totalR = sumR / count;
    int totalG = sumG / count;
    int totalB = sumB / count;
    
    int avg = (totalR + totalG + totalB)/3;
    
    return avg;
    
    //return Color8u(sumR/count, sumG/count, sumB/count); //should presumably return avg color!?
}


void ImageProcessor::displayPixels()
{
    showPixels = true;
    capturingImage = false;
    displayLoaded = false;
}

ci::ivec2 ImageProcessor::getCameraSize()
{
    return ci::vec2(mCapture->getWidth(), mCapture->getHeight());
}

void ImageProcessor::showCapture(ci::ivec2 _loc)
{

    
}

void ImageProcessor::update()
{
    
    if (capturingImage && mCapture && mCapture->checkNewFrame())
    {
        mThresholdValues.clear();
        mPixels = *mCapture->getSurface();
        
        if( ! mTexture ) {
            // Capture images come back as top-down, and it's more efficient to keep them that way
            mTexture = ci::gl::Texture::create( *mCapture->getSurface(), ci::gl::Texture::Format().loadTopDown() );
        }
        else {
            mTexture->update( *mCapture->getSurface() );
        }
        
        for (int i = 0; i < mPixels.getWidth(); i += pixelSize)
        {
            for (int j = 0; j < mPixels.getHeight(); j += pixelSize)
            {
                mThresholdValues.push_back(getAverage(&mPixels, ci::vec2(i,j)));
                mPixelPositions.push_back(ci::ivec2(i,j));
            }
        }
    }
    
}

void ImageProcessor::renderImage()
{
    
    if (capturingImage || displayLoaded) ci::gl::color(ci::ColorAf(1.0,1.,1.,1.)), ci::gl::draw(mTexture);
    
    if (showPixels && mThresholdValues.size() != 0)
    {
        //std::cout << "rendering image" << std::endl;
        int index = 0;
        
        
        for (int i = 0; i < mPixels.getWidth(); i += pixelSize)
        {
            for (int j = 0; j < mPixels.getHeight(); j += pixelSize)
            {
                ci::gl::color(ci::Color::black());
                
                if (mThresholdValues[index] < threshold) ci::gl::drawSolidEllipse(ci::vec2(mCanvasPos.x + (i + pixelSize/2), mCanvasPos.y + (j + pixelSize/2)), 1, 1);
                index++;
            }
        }
    }
}

void ImageProcessor::enableCapture()
{
    capturingImage = true;
}

int* ImageProcessor::getThreshold()
{
    return &threshold;
}

float* ImageProcessor::getScale()
{
    return &mScale;
}

int* ImageProcessor::getPixelSize()
{
    return &pixelSize;
}


 std::vector<ci::vec2>  ImageProcessor::getPixelLocations()
{
    std::vector<ci::vec2>   tempPoints;
    int index = 0;
    
    for (int i = 0; i < mPixels.getWidth(); i += pixelSize)
    {
        for (int j = 0; j < mPixels.getHeight(); j += pixelSize)
        {
            if (mThresholdValues[index] < threshold) tempPoints.push_back(ci::vec2(mCanvasPos.x + (i + pixelSize/2), mCanvasPos.y + (j + pixelSize/2)));
            index++;
        }
    }
    
    return tempPoints;
}


void ImageProcessor::invertImage()
{
    for ( auto p : mThresholdValues) p = 255 - p;
}