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
#pragma once
#include <stdio.h>
#include "EiBotBoard.hpp"
#include "SketchTools.hpp"
#include "Canvas.hpp"

#define PHYSICAL_STAGE_WIDTH 385.0
#define PHYSICAL_STAGE_HEIGHT 300.0
#define SERVO_MIN 14800
#define SERVO_MAX 23000
#define SERVO_CONFIG_MIN 15000

typedef std::shared_ptr<class PlotBot>          PlotBotRef;

//a timed packet contains the command, as well as the duration in millis (stored as an int) for how long the move will take
typedef std::pair<int, std::string>           timedPacket;

//container for all the commandPackets
typedef std::vector<timedPacket>              PacketStack;

class PlotBot
{
public:
    
    static PlotBotRef create()
    {
        return PlotBotRef(new PlotBot());
    }
    
    PlotBot();
    ~PlotBot();
    
    void init();
    void update();
    
    void setPenConfigMin();
    
    void createTempFeature(SketchTools::Tool _tool, ci::ivec2 _tempBegin);
    void updateTempFeature(SketchTools::Tool _tool,ci::ivec2 _tempUpdate);
    void setTempFeatureEndPoint(SketchTools::Tool _tool,ci::ivec2 _tempEnd);
    void drawCanvas();
    void sendPackets();
    
    friend class SketchCNCApp;
    
protected:
    
    void establishConnection();
    void setLimitSwitchPins();
    void createDrawingFeature(SketchTools::DragLineRef _thisLine);
    void liftPenCmd(SketchTools::DragLineRef _thisLine);
    void liftPenCmd(SketchTools::PencilLineRef  _thisPencilLine);
    void createGroup(SketchTools::PencilLineRef _thisPencilLine);
    void createCircle(SketchTools::CircleRef _thisCircle);
    void createPixelImage(std::vector<ci::vec2> _points);
    void setServo();
    void penSetup();
    void sendTimedPackets(); //uses a timer to send packets to the board so that the buffer doesn't overflow
    
    EiBotBoardRef   mBoard; //create an instance of an EiBotBoard object
    
    PacketStack     mPacketStack; //global container for keeping all PacketStacks
    
    CanvasRef       mDigitalCanvas;
    
    CanvasRef       getCanvas();
    
    std::string penUp();
    std::string penDown();
    void addMoveCmd(ci::ivec2 _featureStart);
    void addPixel(ci::ivec2 _currentPixel);
    void generateDrawCmd();
    
    void moveToOrigin();
    void jogRight();
    void jogLeft();
    void jogUp();
    void jogDown();
    void jogUpLeft();
    void jogUpRight();
    void jogDownRight();
    void jogDownLeft();
    
    void runSystem();
    void pauseSystem();
    
    void zeroAxes();
    
    //create an enum Mode that helps setup the board and connection in sequential steps
    enum SetupState { INACTIVE, ESTABLISHING_CONNECTION, LIMIT_SWITCH_SETUP, PEN_SERVO_SETUP, SETUP_COMPLETE, SEND_HOME };
    enum OperationMode { SETUP, NORMAL_OPERATION, SETTING_PEN_HEIGHT, HOMING };
    enum PenConfigState { BEGIN, RAISE_PEN, REMOVE_PEN, LOAD_PEN, RESET_SERVO };
    enum HomingMode { CHECK_LIMIT_RESPONSE_X, CHECK_LIMIT_RESPONSE_Y, CHECK_MOTOR_RESPONSE_X, CHECK_MOTOR_RESPONSE_Y, MOVE_MOTOR_X, MOVE_MOTOR_Y, REQUEST_LIMIT_STATE_X, REQUEST_LIMIT_STATE_Y };
    enum AxisState { X_AXIS, Y_AXIS };

    SetupState      mSetupState;
    OperationMode   mOperationMode;
    PenConfigState  mPenState;
    HomingMode      mHomingMode;
    AxisState       mAxisState;
    
private:
    double  mPulleyDiameter,
            mPulleyRadius,
            mTravelPerRotation,
            mFullStepDist,
            mSixteenthStepDist,
            mEighthStepDist,
            mQuarterStepDist,
            mHalfStepDist;
   
    double mVelocity;
    
    double mGlobalTime, mLastRead;
    
    int previousCommandDuration;
    
    bool systemPaused, xHome, yHome;
    
    int jogDistance; //distance the gantry moves when manually moved.
    
    ci::vec2    mPenPixelPosition;
    
    //create temporary objects that will be used to create commands during mouseEvents
    
    SketchTools::DragLineRef        mTempDragLine;
    SketchTools::PencilLineRef      mTempPencilLine;
    SketchTools::CircleRef          mTempCircle;
    
};