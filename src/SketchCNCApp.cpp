/*
 Copyright (c) 2016, Craig Pickard - All rights reserved.
 
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "PlotBot.hpp"
#include "ImageProcessor.hpp"

#include "CodeTools.h"
#include "SketchTools.hpp"
#include "Canvas.hpp"
#include "CinderImGui.h"

#include "cinder/Surface.h"
#include "cinder/Capture.h"
#include "cinder/Camera.h"

#define STAGE_WIDTH 1300
#define STAGE_HEIGHT 800

#define TOOLBAR_WIDTH 200
#define TOOLBAR_HEIGHT 600
#define TOOLBAR_PADDING 40

#define PEN_CONTROL_WIDTH 200
#define PEN_CONTROL_HEIGHT 90

#define CONTROL_WINDOW_WIDTH 200
#define CONTROL_WINDOW_HEIGHT 200

#define PEN_CONFIG_WIDTH 200
#define PEN_CONFIG_HEIGHT 200

#define PORTRAIT_WIDTH 200
#define PORTRAIT_HEIGHT 200

#define NUM_CIRCLES 24

using namespace ci;
using namespace ci::app;
using namespace std;

/****************************************************
 
 TO DO LIST:
 
 - Move to origin method
 - Fix UI:
    - Display current tool
    - Add cursors
 - Cable Management
 - Add Geometry
    - square
    - triangle
 
 
 
 ****************************************************/

class SketchCNCApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void keyDown (KeyEvent event) override;
	void update() override;
	void draw() override;
    
    void createGenerative();
    
    void initGUI();
    void displayGUI();
    
    ImageProcessorRef   mImageProcessor;
    
    PlotBotRef          mPlotter;
    
    SketchTools::Tool   mTool;
    
    //textures to store the icon images
    gl::Texture2dRef                mPencilIcon,
                                    mLineIcon,
                                    mHomeIcon,
                                    mSetPenIcon,
                                    mUpIcon,
                                    mDownIcon,
                                    mLeftIcon,
                                    mRightIcon,
                                    mBlankIcon,
                                    mDownLeftIcon,
                                    mUpLeftIcon,
                                    mUpRightIcon,
                                    mDownRightIcon,
                                    mPauseIcon,
                                    mPenUpIcon,
                                    mPenDownIcon,
                                    mCircleIcon;
    
    //enum to determine which sections of UI are being displayed
    enum UImode { TOOLBAR, PEN_CONFIG };
    UImode mUImode;
    
    //method that returns an ivec2 that contains the width and height of the GUI window (including padding)
    ci::ivec2 getDimensions();
    
    //Rectf that represents the digital/physical drawing surface
    ci::Rectf canvas;
    
    bool showToolbar, creativeGenerative;
    
    //booleans for loading images / camera for portraits
    bool imageLoaded, usingCamera;
    
};

void SketchCNCApp::setup()
{
    mPlotter = PlotBot::create();
    mPlotter->init();
    
    mImageProcessor = ImageProcessor::create();
    
    
    initGUI();
    
    mTool = SketchTools::PENCIL_TOOL;
    
    creativeGenerative = false;
    
    imageLoaded = false;
    usingCamera = false;
}

void SketchCNCApp::mouseDown( MouseEvent event )
{
    mPlotter->createTempFeature(mTool, event.getPos());
}

void SketchCNCApp::mouseDrag( MouseEvent event )
{
    mPlotter->updateTempFeature(mTool, event.getPos());
}

void SketchCNCApp::mouseUp( MouseEvent event )
{
    mPlotter->setTempFeatureEndPoint(mTool, event.getPos());
}

void SketchCNCApp::keyDown ( KeyEvent event )
{
    //  cout << event.getCode() << endl;
    
    switch (event.getChar())
    {
        case 'm':
            createGenerative();
            break;
            
        default:
            break;
    }
    
    switch (event.getCode())
    {

        case 273:
            cout << "Adding move up" << endl;
            mPlotter->mBoard->sendCommand("SM,300,0,-2000\r");
            break;
            
            
        case 274:
            cout << "Adding move down" << endl;
            mPlotter->mBoard->sendCommand("SM,300,0,2000\r");
            
            break;
            
        case 275:
            cout << "Adding move right" << endl;
            //            mBoard->addCommand("SM,1000,2000,0\r");
            mPlotter->mBoard->sendCommand("SM,1000,2000,0\r");
            break;
            
        case 276:
            cout << "Adding move left" << endl;
            mPlotter->mBoard->sendCommand("SM,1000,-2000,0\r");
            break;
            
        default:
            break;
    }
    
}

void SketchCNCApp::update()
{
    mPlotter->update();
    displayGUI();
    
    mImageProcessor->update();

}

void SketchCNCApp::draw()
{
    gl::clear( Color( 0.2, 0.2, 0.2 ) );
    mPlotter->drawCanvas();
    
    mImageProcessor->renderImage();
    
    
}

//------------------------------------------------------------ U I ------------------------------------------------------------

void SketchCNCApp::displayGUI()
{
    if (showToolbar)
    {
        {
        ui::ScopedWindow window( "DRAWING TOOLS", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );
        
        ui::SetWindowPos(ci::ivec2(ci::app::getWindowWidth() - TOOLBAR_WIDTH,0));
            ui::SetWindowSize(ci::ivec2(TOOLBAR_WIDTH, ci::app::getWindowHeight() - CONTROL_WINDOW_HEIGHT - PEN_CONTROL_HEIGHT - PORTRAIT_HEIGHT));
        

        ui::SameLine();
        if (ui::ImageButton(mPencilIcon, ci::ivec2(50,50)))
        {
            std::cout << "pencil selected" << std::endl;
            mTool = SketchTools::PENCIL_TOOL;
            std::cout << SketchTools::mTool << std::endl;
        }
        ui::SameLine();
        if (ui::ImageButton(mLineIcon, ci::ivec2(50,50))) std::cout << "dragline selected" << std::endl, mTool = SketchTools::LINE_TOOL,std::cout << SketchTools::mTool << std::endl;;
        
        ui::Spacing();
            
            if (ui::ImageButton(mCircleIcon, ci::vec2(50,50))) mTool = SketchTools::CIRCLE_TOOL;
        

        
        ui::Spacing();
        
        }
        
        //PORTRAIT OPTIONS
        
        {
            ui::ScopedWindow utilities("PORTRAIT OPTIONS", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ui::SetWindowPos(ci::ivec2(ci::app::getWindowWidth() - PORTRAIT_WIDTH,ci::app::getWindowHeight()- CONTROL_WINDOW_HEIGHT - PEN_CONTROL_HEIGHT - PORTRAIT_HEIGHT));
            ui::SetWindowSize(ci::ivec2(PORTRAIT_WIDTH,PORTRAIT_HEIGHT));
            

            {
                ui::Text("Select image source:");
                ui::Spacing();
                
                if (ui::Button("Load from Disk")) mImageProcessor->loadImage(), mImageProcessor->displayPixels(), imageLoaded = true;
                ui::SameLine();
                if (ui::Button("Use Camera"))
                {
                    std::cout << "Using Camera" << std::endl;
                    mImageProcessor->initCamera();
                    mImageProcessor->enableCapture();
                    imageLoaded = true;
                    usingCamera = true;
                }
                
                if (imageLoaded)
                {
                    int* mThresh = mImageProcessor->getThreshold();
                    float *mScale = mImageProcessor->getScale();
                    //int* mPixelSize = mImageProcessor->getPixelSize();
                    
                    if( usingCamera) if (ui::Button("Take Picture")) mImageProcessor->displayPixels();
                    if (ui::Button("Invert Image")) mImageProcessor->invertImage();
                    
                    ImGui::SliderInt("Threshold", mThresh, 0, 255);
                   // ImGui::SliderFloat("Scale", mScale, 0.0f, 2.0f);
    //                ImGui::SliderInt("Pixel Spacing", mPixelSize, 1, 10);
                    
                    if(ui::Button("Print")) mPlotter->createPixelImage(mImageProcessor->getPixelLocations());
                }
                
            }
        }

        //PLOTTER CONTROLS

        {
            ui::ScopedWindow utilities("UTILITIES", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ui::SetWindowPos(ci::ivec2(ci::app::getWindowWidth() - PEN_CONTROL_WIDTH,ci::app::getWindowHeight()- CONTROL_WINDOW_HEIGHT - PEN_CONTROL_HEIGHT));
            ui::SetWindowSize(ci::ivec2(PEN_CONTROL_WIDTH,PEN_CONTROL_HEIGHT));
            
            ImGui::BeginGroup();
            {
                if (ui::ImageButton(mPenUpIcon, ci::ivec2(50,50))) mPlotter->mBoard->sendCommand(mPlotter->penUp());
                ui::SameLine();
                if (ui::ImageButton(mPenDownIcon, ci::ivec2(50,50))) mPlotter->mBoard->sendCommand(mPlotter->penDown());
                ui::SameLine();
                if (ui::ImageButton(mPauseIcon, ci::ivec2(50,50))) mPlotter->pauseSystem(), ImGui::OpenPopup("System Paused");
                
                if (ImGui::BeginPopupModal("System Paused"))
                {
                    ImGui::Text("The System is now paused:");
                    ImGui::Text("Click 'Resume' to continue");
                    
                    if (ImGui::Button("Resume")) ImGui::CloseCurrentPopup(), mPlotter->runSystem();
                    
                    ui::Spacing();
                    ui::Spacing();
                    
                    ImGui::EndPopup();
                }
                
                
            }
            ImGui::EndGroup();
        }
        
        {
            ui::ScopedWindow utilities("CONTROLS", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ui::SetWindowPos(ci::ivec2(ci::app::getWindowWidth() - CONTROL_WINDOW_WIDTH,ci::app::getWindowHeight()- CONTROL_WINDOW_HEIGHT));
            ui::SetWindowSize(ci::ivec2(CONTROL_WINDOW_WIDTH, CONTROL_WINDOW_HEIGHT));
            
            ImGui::BeginGroup();
            {
                //row 1
                ImGui::BeginGroup();
                if(ui::ImageButton(mUpLeftIcon, ci::ivec2(50,50))) mPlotter->jogUpLeft();
                ui::SameLine();
                if(ui::ImageButton(mUpIcon, ci::ivec2(50,50))) mPlotter->jogUp();
                ui::SameLine();
                if(ui::ImageButton(mUpRightIcon, ci::ivec2(50,50)))mPlotter->jogUpRight();
                ImGui::EndGroup();
                
                
                //row 2
                ImGui::BeginGroup();
                if(ui::ImageButton(mLeftIcon, ci::ivec2(50,50))) mPlotter->jogLeft();
                ui::SameLine();
                if(ui::ImageButton(mHomeIcon, ci::ivec2(50,50))) mPlotter->zeroAxes();
                ui::SameLine();
                if(ui::ImageButton(mRightIcon, ci::ivec2(50,50))) mPlotter->jogRight();
                ImGui::EndGroup();
                
                //row 3
                ImGui::BeginGroup();
                if(ui::ImageButton(mDownLeftIcon, ci::ivec2(50,50))) mPlotter->jogDownLeft();
                ui::SameLine();
                if(ui::ImageButton(mDownIcon, ci::ivec2(50,50))) mPlotter->jogDown();
                ui::SameLine();
                if(ui::ImageButton(mDownRightIcon, ci::ivec2(50,50))) mPlotter ->jogDownRight();
                ImGui::EndGroup();
                
            }
            ImGui::EndGroup();
        }
        
    } //end of showToolbar
}


void SketchCNCApp::createGenerative()
{
    float rad = 70.;
    float theta = 0;
    int offset = 20;
    ci::ivec2 c = ci::ivec2(CANVAS_WIDTH/2 - 50, CANVAS_HEIGHT/2 - 50);
    for (int i = 0; i < NUM_CIRCLES; i++)
    {
        SketchTools::CircleRef tempCircle = SketchTools::Circle::create(c + ci::ivec2(rad *cos(toRadians(theta)), rad * sin(toRadians(theta))));
        tempCircle->setRadius(tempCircle->getCentrePos() + ci::vec2(rad + offset,0));
        mPlotter->createCircle(tempCircle);
        mPlotter->mDigitalCanvas->addCircle(tempCircle);
        theta += 360/NUM_CIRCLES;
        offset += 5;
    }
}


//initialises the ImGui instance and loads all the GUI assets into memory
void SketchCNCApp::initGUI()
{
    ui::initialize();
    
    showToolbar = true;
    //load the pencil icon image, create a texture and bind it
    auto pencilIcon = ci::loadImage(ci::app::loadAsset("pencil_icon_square_100x100.png"));
    mPencilIcon = ci::gl::Texture2d::create(pencilIcon);
    mPencilIcon->bind(0);
    
    auto lineIcon = ci::loadImage(ci::app::loadAsset("dragline_icon_square_100x100.png"));
    mLineIcon = ci::gl::Texture2d::create(lineIcon);
    mLineIcon->bind(1);
    
    auto setPenIcon = ci::loadImage(ci::app::loadAsset("pen_down_icon_100x100.png"));
    mSetPenIcon = ci::gl::Texture2d::create(setPenIcon);
    mSetPenIcon->bind(2);
    
    auto upIcon = ci::loadImage(ci::app::loadAsset("up_100x100.png"));
    mUpIcon = ci::gl::Texture2d::create(upIcon);
    mUpIcon->bind(3);
    
    auto downIcon = ci::loadImage(ci::app::loadAsset("down_100x100.png"));
    mDownIcon = ci::gl::Texture2d::create(downIcon);
    mDownIcon->bind(4);
    
    auto leftIcon = ci::loadImage(ci::app::loadAsset("left_100x100.png"));
    mLeftIcon = ci::gl::Texture2d::create(leftIcon);
    mLeftIcon->bind(5);
    
    auto rightIcon = ci::loadImage(ci::app::loadAsset("right_100x100.png"));
    mRightIcon = ci::gl::Texture2d::create(rightIcon);
    mRightIcon->bind(6);
    
    auto blankIcon = ci::loadImage(loadAsset("blank.png"));
    mBlankIcon = ci::gl::Texture2d::create(blankIcon);
    mBlankIcon->bind(7);
    
    auto downLeftIcon = ci::loadImage(loadAsset("downLeft_100x100.png"));
    mDownLeftIcon = ci::gl::Texture2d::create(downLeftIcon);
    mDownLeftIcon->bind(8);
    
    auto upLeftIcon = ci::loadImage(loadAsset("upLeft_100x100.png"));
    mUpLeftIcon = ci::gl::Texture2d::create(upLeftIcon);
    mUpLeftIcon->bind(9);

    auto upRightIcon = ci::loadImage(loadAsset("upRight_100x100.png"));
    mUpRightIcon = ci::gl::Texture2d::create(upRightIcon);
    mUpRightIcon->bind(10);

    auto downRightIcon = ci::loadImage(loadAsset("downRight_100x100.png"));
    mDownRightIcon = ci::gl::Texture2d::create(downRightIcon);
    mDownRightIcon->bind(11);
    
    auto homeIcon = ci::loadImage(loadAsset("home_100x100.png"));
    mHomeIcon = ci::gl::Texture2d::create(homeIcon);
    mHomeIcon->bind(12);
    
    auto pauseIcon = ci::loadImage(loadAsset("pause_100x100.png"));
    mPauseIcon = ci::gl::Texture2d::create(pauseIcon);
    mPauseIcon->bind(13);
    
    auto penUpIcon = ci::loadImage(loadAsset("penUp_100x100.png"));
    mPenUpIcon = ci::gl::Texture2d::create(penUpIcon);
    mPenUpIcon->bind(14);
    
    auto penDownIcon = ci::loadImage(loadAsset("penDown_100x100.png"));
    mPenDownIcon = ci::gl::Texture2d::create(penDownIcon);
    mPenDownIcon->bind(15);
    
    auto circleIcon = ci::loadImage(loadAsset("circle_100x100.png"));
    mCircleIcon = ci::gl::Texture2d::create(circleIcon);
    mCircleIcon->bind(16);
}



//--------------------------------------------- S E T T I N G S ------------------------------------------------------------

CINDER_APP( SketchCNCApp, RendererGl(RendererGl::Options().msaa(16)),
           [&](App::Settings *settings){
               //settings->setHighDensityDisplayEnabled();
               settings->setWindowSize(STAGE_WIDTH, STAGE_HEIGHT);
               settings->setTitle("OSTK presents SketchCNC :: By Craig Pickard :: ITP Thesis Project 2016");
           })
