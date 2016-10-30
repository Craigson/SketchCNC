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
#include "PlotBot.hpp"


PlotBot::PlotBot():
mBoard(EiBotBoard::create()),
mSetupState(INACTIVE),
mOperationMode(SETUP),
mPulleyDiameter(16.1798),
//mPulleyDiameter(15.8798),
mVelocity(40.0),
mPenPixelPosition(ci::ivec2(0,0)),
mDigitalCanvas(Canvas::create()),
mGlobalTime(0.),
mLastRead(0.),
previousCommandDuration(0),
mTempDragLine(nullptr),
mTempPencilLine(nullptr),
jogDistance(60),
systemPaused(false),
xHome(false),
yHome(false),
mAxisState(Y_AXIS),
mHomingMode(REQUEST_LIMIT_STATE_Y)
{
    mPulleyRadius = mPulleyDiameter / 2.;
    mTravelPerRotation = 2 * M_PI * mPulleyRadius;
    mFullStepDist = mTravelPerRotation / 200.;
    mSixteenthStepDist = mFullStepDist / 16.;
    mEighthStepDist = mFullStepDist / 8.;
    mQuarterStepDist = mFullStepDist / 4.;
    mHalfStepDist = mHalfStepDist /2.;
}

PlotBot::~PlotBot(){}

void PlotBot::init()
{
    mBoard->init();
    SketchTools::setRatio(CANVAS_WIDTH , PHYSICAL_STAGE_WIDTH , CANVAS_HEIGHT, PHYSICAL_STAGE_HEIGHT);
}



/************************************************************************
 *
 *                          U P D A T E
 *
 ************************************************************************/

void PlotBot::update()
{
    switch (mOperationMode)
    {
    
        case SETUP:
            
            switch (mSetupState)
            {
                case INACTIVE:
                    if (ci::app::getElapsedSeconds() > 1) mSetupState = ESTABLISHING_CONNECTION;
                    break;
                    
                case ESTABLISHING_CONNECTION:
                    std::cout << ">> Attempting to establish connection << " << std::endl;
                    establishConnection();
                    break;
                    
                case LIMIT_SWITCH_SETUP:
                    setLimitSwitchPins();
                    break;
                    
                case PEN_SERVO_SETUP:
                    setServo();
                    break;
                    
                case SETUP_COMPLETE:
                    mSetupState = SEND_HOME;
                    std::cout << ">>> SETUP IS COMPLETE, SWITCHING MODE TO NORMAL_OPERATION <<<" << std::endl;
                    break;
                    
                case SEND_HOME:
                    if (ci::app::getElapsedFrames() > 240) moveToOrigin();
                    break;
                    
                default:
                    break;
            }
            break;
            
            
        case NORMAL_OPERATION:
            
            if(!systemPaused) sendTimedPackets();
            
            break;
            
        case SETTING_PEN_HEIGHT:
            
            penSetup();
            
            break;
            
            
            
        case HOMING:
            //std::cout << "Moving to Home" << std::endl;
            moveToOrigin();
            break;
            
            
            
        default:
            break;
    }
    
}



/************************************************************************
 *
 *                  E Sx T A B L I S H  C O N N E C T I O N
 *
 ************************************************************************/

void PlotBot::establishConnection()
{
    if (mBoard->mSerial->getNumBytesAvailable() > 0)
    {
        //  std::cout<< "receiving bytes" << std::endl;
        auto numBytes = mBoard->mSerial->getNumBytesAvailable();
        uint8_t buffer[numBytes];
        mBoard->mSerial->readBytes(buffer, numBytes);
        if (numBytes == 15)
        {
            std::string response =  std::to_string(buffer[11]) + std::to_string(buffer[12]);
            
            //THE ASCII CODES FOR 'O' AND 'K' ARE '79' AND '75' RESPECTIVELY
            if (response.compare("7975") == 0) std::cout << "Success: Connection Established" << std::endl;
            std::cout << "Switching mode to SETTING_PINS" << std::endl;
            mSetupState = LIMIT_SWITCH_SETUP;
        }
        mBoard->flushBuffer();
        return;
    }
    
    mBoard->sendCommand("qc\r");
    mBoard->flushBuffer();
}



/************************************************************************
 *
 *                S E T  L I M I T  S W I T C H  P I N S
 *
 ************************************************************************/

void PlotBot::setLimitSwitchPins()
{
    
    if (mBoard->mSerial->getNumBytesAvailable() > 0)
    {
        //  std::cout<< "receiving bytes" << std::endl;
        auto numBytes = mBoard->mSerial->getNumBytesAvailable();
        uint8_t buffer[numBytes];
        mBoard->mSerial->readBytes(buffer, numBytes);
        if (numBytes == 8)
        {
            std::string responseA =  std::to_string(buffer[0]) + std::to_string(buffer[1]);
            std::string responseB =  std::to_string(buffer[4]) + std::to_string(buffer[5]);
            
            //THE ASCII CODES FOR 'O' AND 'K' ARE '79' AND '75' RESPECTIVELY
            if (responseA.compare("7975") == 0 && responseB.compare("7975") == 0)
            {
                std::cout << "Success! Limit switch direction set" << std::endl;
                mSetupState = PEN_SERVO_SETUP;
            }
            
        }
        mBoard->flushBuffer();
        return;
    }
    
    //SET THE PINS FOR THE LIMIT SWITCHES AS INPUTS
    mBoard->sendCommand("PD,A,1,1\r");
    mBoard->sendCommand("PD,A,2,1\r");
    mBoard->flushBuffer();
    
}



/************************************************************************
 *
 *                S E T  S E R V O
 *
 ************************************************************************/

void PlotBot::setServo()
{
    
    if (mBoard->mSerial->getNumBytesAvailable() > 0)
    {
        //  std::cout<< "receiving bytes" << std::endl;
        auto numBytes = mBoard->mSerial->getNumBytesAvailable();
        uint8_t buffer[numBytes];
        mBoard->mSerial->readBytes(buffer, numBytes);
        if (numBytes == 8)
        {
            std::string responseA =  std::to_string(buffer[0]) + std::to_string(buffer[1]);
            std::string responseB =  std::to_string(buffer[4]) + std::to_string(buffer[5]);
            
            //THE ASCII CODES FOR 'O' AND 'K' ARE '79' AND '75' RESPECTIVELY
            if (responseA.compare("7975") == 0 && responseB.compare("7975") == 0)
            {
                std::cout << "Success! Servo has been calibrated" << std::endl;
                mBoard->sendCommand(penUp());
                mSetupState = SETUP_COMPLETE;
               // if(mPenState == RESET_SERVO) SketchUI::showToolbar = true;
            }
            
        }
        mBoard->flushBuffer();
        return;
    }
    
    //set the values for servo_min and servo_max, these determine the penUp and penDown positions
    mBoard->sendCommand("SC,4," + std::to_string(SERVO_MIN) + "\r");
    mBoard->sendCommand("SC,5," + std::to_string(SERVO_MAX) + "\r");

    mBoard->flushBuffer();
    
}




/************************************************************************
 *
 *                P E N   S E T U P
 *
 ************************************************************************/

void PlotBot::penSetup()
{
    /*
    ui::ScopedWindow penConfig ( "Pen Height Calibration", ImGuiWindowFlags_NoMove);
    ui::SetWindowPos(ci::ivec2(ci::app::getWindowWidth()/2 - PEN_CONFIG_WIDTH, ci::app::getWindowHeight()/2 - PEN_CONFIG_HEIGHT));
    ui::SetWindowSize(ci::ivec2(PEN_CONFIG_WIDTH, PEN_CONFIG_HEIGHT));
    
    
    switch (mPenState)
    {
            
        case BEGIN:
            
            std::cout << ">>> Attempting to set SERVO_MIN to SERVO_CONFIG_MIN" << std::endl;

            if (mBoard->mSerial->getNumBytesAvailable() > 0)
            {
                //  std::cout<< "receiving bytes" << std::endl;
                auto numBytes = mBoard->mSerial->getNumBytesAvailable();
                uint8_t buffer[numBytes];
                mBoard->mSerial->readBytes(buffer, numBytes);
                if (numBytes == 4)
                {
                    std::string responseA =  std::to_string(buffer[0]) + std::to_string(buffer[1]);
                    
                    //THE ASCII CODES FOR 'O' AND 'K' ARE '79' AND '75' RESPECTIVELY
                    if (responseA.compare("7975") == 0)
                    {
                        std::cout << ">>> Success, new servo_min has been set!" << std::endl;
                        mPenState = RAISE_PEN;
                    }
                    
                }
                mBoard->flushBuffer();
            }
            
            mBoard->sendCommand("SC,4," + std::to_string(SERVO_CONFIG_MIN) + "\r");
            mBoard->flushBuffer();
            
            break;
            
            
            
        case RAISE_PEN:
            std::cout << ">>> Attempting to raise pen" << std::endl;
            
            if (mBoard->mSerial->getNumBytesAvailable() > 0)
            {
                //  std::cout<< "receiving bytes" << std::endl;
                auto numBytes = mBoard->mSerial->getNumBytesAvailable();
                uint8_t buffer[numBytes];
                mBoard->mSerial->readBytes(buffer, numBytes);
                if (numBytes == 4)
                {
                    std::string responseA =  std::to_string(buffer[0]) + std::to_string(buffer[1]);
                    
                    //THE ASCII CODES FOR 'O' AND 'K' ARE '79' AND '75' RESPECTIVELY
                    if (responseA.compare("7975") == 0)
                    {
                        std::cout << ">>> Success, pen has been raised!" << std::endl;
                        mPenState = REMOVE_PEN;
                    }
                    
                }
                mBoard->flushBuffer();
            }
            
            mBoard->sendCommand(penUp());
            mBoard->flushBuffer();
            break;
            
            
            
        case REMOVE_PEN:

            ui::Text("Loosen the set screws and remove the pen if it is present.  When you have removed the pen from the receiver, or if it is already empty, click the NEXT button");
            ui::Spacing();
            if (ui::Button("NEXT")) mPenState = LOAD_PEN;
            break;
           
            
            
       case LOAD_PEN:
            ui::Text("Drop the pen into the receiver so that the point rests on the drawing surface.  Tighten the set screws to secure the pen.  When the pen is secured, click the 'SET ZERO' button.");
            ui::Spacing();
            if (ui::Button("SET_ZERO")) mPenState = RESET_SERVO;
           break;
           
        case RESET_SERVO:
            setServo();
            break;
            
        default:
           break;
    }
*/
}


/************************************************************************
 *
 *                P E N  U P
 *
 ************************************************************************/

std::string PlotBot::penUp()
{
    std::cout << "Raising Pen" << std::endl;
    
    std::string upCmd = "SP,0,100\r"; //raise the pen to the height determined by SERVO_MAX, delaying the next move command by 1000ms
    return upCmd;
    
}


/************************************************************************
 *
 *                P E N  D O W N
 *
 ************************************************************************/

std::string PlotBot::penDown()
{
    std::cout << "Lowering Pen" << std::endl;
    std::string upCmd = "SP,1,600\r"; //lower the pen to the height determined by SERVO_MAX, delaying the next move command by 1000ms
    return upCmd;
}




/************************************************************************
 *
 *                S E T  P E N  C O N F I G  M I N
 *
 ************************************************************************/

void PlotBot::setPenConfigMin()
{
    std::cout << "Setting temporary servo_min value" << std::endl;
    mBoard->sendCommand("SC,4," + std::to_string(SERVO_CONFIG_MIN) + "\r");
}


/************************************************************************
 *
 *                C R E A T E  T E M P  F E A T U R E
 *
 ************************************************************************/
void PlotBot::createTempFeature(SketchTools::Tool _tool,ci::ivec2 _tempBegin)
{
    switch (_tool)
    {
        case SketchTools::LINE_TOOL:
            mTempDragLine = SketchTools::DragLine::create( _tempBegin ); //create a temporary line feature
            addMoveCmd(_tempBegin); //if the beginning point is not the same as the previous pen pos, add a move command
            break;
            
        case SketchTools::PENCIL_TOOL:
            mTempPencilLine = SketchTools::PencilLine::create( _tempBegin);
            addMoveCmd(_tempBegin);
            
        {
            timedPacket penDownPacket = std::make_pair( 01, penDown() );
            mPacketStack.push_back(penDownPacket);
        }
            
            break;
            
        case SketchTools::CIRCLE_TOOL:
            mTempCircle = SketchTools::Circle::create(_tempBegin);
            mTempCircle->setCenter(_tempBegin);
            addMoveCmd(_tempBegin);
            
        {
            timedPacket penDownPacket = std::make_pair( 01, penDown() );
            mPacketStack.push_back(penDownPacket);
        }
            
            break;
            
        default:
            break;
    }
}



/************************************************************************
 *
 *                U P D A T E  T E M P  F E A T U R E
 *
 ************************************************************************/
void PlotBot::updateTempFeature(SketchTools::Tool _tool,ci::ivec2 _tempUpdate)
{
    switch (_tool)
    {
        case SketchTools::LINE_TOOL:
            mTempDragLine->update( _tempUpdate );
            break;
            
        case SketchTools::PENCIL_TOOL:
            
            mTempPencilLine->update(_tempUpdate);
            
            break;
            
        case SketchTools::CIRCLE_TOOL:
            
            mTempCircle -> update(_tempUpdate);
            break;
            
        default:
            break;
    }
}


/************************************************************************
 *
 *                S E T  T E M P  E N D  P O I N T
 *
 ************************************************************************/
void PlotBot::setTempFeatureEndPoint(SketchTools::Tool _tool,ci::ivec2 _tempEnd)
{
    std::cout << SketchTools::mTool << std::endl;
        switch (_tool)
        {
            case SketchTools::LINE_TOOL:
                mTempDragLine->setEndPoint(_tempEnd);
                mDigitalCanvas->addLine(mTempDragLine);
                
                //generate commands for the plotter
                liftPenCmd(mTempDragLine);
                createDrawingFeature(mTempDragLine);
                break;
                
            case SketchTools::PENCIL_TOOL:
                mTempPencilLine->setEndPoint( _tempEnd );
                mDigitalCanvas->addPencilLine(mTempPencilLine);
                
                createGroup(mTempPencilLine);
                
                //createSegment(mTempPencilLine);
                liftPenCmd(mTempPencilLine);
                break;
                
            case SketchTools::CIRCLE_TOOL:
                
                mTempCircle -> setRadius(_tempEnd);
                mDigitalCanvas->addCircle(mTempCircle);
                
                createCircle(mTempCircle);
               // liftPenCmd(mTempPencilLine);
                break;
                
            default:
                break;
        }
}

/************************************************************************
 *
 *                D R A W  C A N V A S
 *
 ************************************************************************/

void PlotBot::drawCanvas()
{
    mDigitalCanvas->render();
    mDigitalCanvas->showPenPosition(mPenPixelPosition);
    //DRAW THE TEMPORARY LINE THAT'S CURRENTLY BEING CREATED
    if (mTempDragLine != nullptr) mTempDragLine->display();
    if (mTempPencilLine != nullptr) mTempPencilLine->display();
    if (mTempCircle != nullptr) mTempCircle->display();
}





/************************************************************************
 *
 *                A D D  P I X E L
 *
 ************************************************************************/

void PlotBot::addPixel(ci::ivec2 _currentPixel)
{
    std::string moveCmd;
    ci::vec2 startPoint, endPoint;
    
    startPoint = mPenPixelPosition; //the start point of the move is the last recorded position of the pen in pixel space
    endPoint = _currentPixel;
    
        int dirX, dirY; //which direction are we moving in?
    
    startPoint.x < endPoint.x ? dirX = 1 : dirX = -1;
    startPoint.y < endPoint.y ? dirY = 1 : dirY = -1;
    
    int pixelDistX = std::abs(startPoint.x - endPoint.x);
    int pixelDistY = std::abs(startPoint.y - endPoint.y);
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * dirX;
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * dirY;
    
    moveCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    std::cout << moveCmd << std::endl;
    
    mPenPixelPosition = endPoint;
    
    timedPacket penUpPacket = std::make_pair(10, penUp()); //add a pen up at the start of the move command
    mPacketStack.push_back(penUpPacket);
    
    timedPacket tempPacket = std::make_pair(moveDurationINmillis, moveCmd);
    
    mPacketStack.push_back(tempPacket);
}




/************************************************************************
 *
 *                G E N E R A T E  M O V E  C O M M A N D
 *
 ************************************************************************/

void PlotBot::addMoveCmd(ci::ivec2 _featureStart)
{
    std::string moveCmd;
    ci::vec2 startPoint, endPoint;
    
    startPoint = mPenPixelPosition; //the start point of the move is the last recorded position of the pen in pixel space
    endPoint = _featureStart;
    
    ci::vec2 dir = startPoint - endPoint;
    float dist = length(dir);
    
    if (dist > 5)
    {
        int dirX, dirY; //which direction are we moving in?
        
        startPoint.x < endPoint.x ? dirX = 1 : dirX = -1;
        startPoint.y < endPoint.y ? dirY = 1 : dirY = -1;
        
        int pixelDistX = std::abs(startPoint.x - endPoint.x);
        int pixelDistY = std::abs(startPoint.y - endPoint.y);
        
        double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
        double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
        
        //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
        //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
        
        int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));

        double moveDurationINsecs = moveDistanceINmm / mVelocity;
        
        int moveDurationINmillis = moveDurationINsecs * 1000;
        
        int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * dirX;
        int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * dirY;
        
        moveCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
        
        std::cout << moveCmd << std::endl;
       
        mPenPixelPosition = endPoint;
        
        timedPacket penUpPacket = std::make_pair(500, penUp()); //add a pen up at the start of the move command
        mPacketStack.push_back(penUpPacket);
        
        timedPacket tempPacket = std::make_pair(moveDurationINmillis, moveCmd);
        
        mPacketStack.push_back(tempPacket);
    }
}


/************************************************************************
 *
 *               C R E A T E  D R A W I N G  F E A T U R E
 *
 ************************************************************************/
void PlotBot::createDrawingFeature(SketchTools::DragLineRef _thisLine)
{
    
    std::string drawCmd;
    
    ci::vec2 dir = mPenPixelPosition - _thisLine->getStartPos();
    
    //float dist = length(dir);
    
    ci::vec2 startPoint, endPoint;
    
    startPoint = _thisLine->getStartPos();
    endPoint = _thisLine->getEndPos();
    
    int dirX, dirY; //which direction are we moving in?
    
    startPoint.x < endPoint.x ? dirX = 1 : dirX = -1;
    startPoint.y < endPoint.y ? dirY = 1 : dirY = -1;
    
    int pixelDistX = std::abs(startPoint.x - endPoint.x);
    int pixelDistY = std::abs(startPoint.y - endPoint.y);
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * dirX;
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * dirY;
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    //add pen down at the beginning of move.
    int penDownTime = 1000;
    timedPacket penDownTimed = std::make_pair(penDownTime, penDown());
    mPacketStack.push_back(penDownTimed);
    
    std::cout << "adding pen down command " << std::endl;
    
    //add the move command
    timedPacket drawPacket = std::make_pair(moveDurationINmillis, drawCmd);
    mPacketStack.push_back(drawPacket);
    
    std::cout << "adding draw command: " << drawCmd << " - which will take " << moveDurationINmillis << "ms to execute and will travel a distance of " << moveDistanceINmm << "mm on the page" << std::endl;
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = _thisLine->getEndPos();
    
}

/************************************************************************
 *
 *               L I F T  P E N  C O M M A N D
 *
 ************************************************************************/

void PlotBot::liftPenCmd(SketchTools::DragLineRef _thisLine)
{
    //check to see whether this line is a continuation of the previous line, if so, don't create the pen lift cmd
    
    ci::vec2 dir = _thisLine->getStartPos() - mPenPixelPosition;
    float dist = length(dir);
    
    if (dist > 3)
    {
        timedPacket liftPacket = std::make_pair(500, penUp());
        mPacketStack.push_back(liftPacket);
        std::cout << "Adding Pen Up Command" << std::endl;
        
    } else {
        std::cout << " >>> line is continues from previous line, no PEN UP cmd required <<< " << std::endl;
    }
}



void PlotBot::liftPenCmd(SketchTools::PencilLineRef _thisPencilLine)
{
    //check to see whether this line is a continuation of the previous line, if so, don't create the pen lift cmd
    
    ci::vec2 dir = _thisPencilLine->getStartPos() - mPenPixelPosition;
    float dist = length(dir);
    
    if (dist > 3)
    {
        timedPacket liftPacket = std::make_pair(500, penUp());
        mPacketStack.push_back(liftPacket);
        std::cout << "Adding Pen Up Command" << std::endl;
        
    } else {
        std::cout << " >>> line is continues from previous line, no PEN UP cmd required <<< " << std::endl;
    }
}




/************************************************************************
 *
 *               S E N D  T I M E D   P A C K E T S
 *
 ************************************************************************/

void PlotBot::sendTimedPackets()
{
    //check the current time
    double now = ci::app::getElapsedSeconds();
    double nowMillis = now * 1000.;
    
    double deltaTime = nowMillis - mGlobalTime;
    mGlobalTime = nowMillis;
    mLastRead += deltaTime;
    
    /* NEED TO CHANGE THIS TO REFLECT THE PREVIOUS COMMAND'S TIME, NOT THE CURRENT - IE. THE CURRENT COMMANDS TIME NEEDS TO BE RECORDED FOR LAST TIME */
    
    if (mPacketStack.size() > 0)
    {
        //we don't want to flood the board's buffer and cause it to lose commands.  To ensure we don't we create a timer that keeps track of the last time a command was sent.  The conditional statement constantly checks the running count (since the last command was sent) against the time value for the move.
        if( mLastRead > previousCommandDuration - 500 )	{
            
//            std::cout << " <<<<<<< >>>>>>>" << std::endl;
//            std::cout << "now: " << now << std::endl;
//            std::cout << "nowMillis: " << nowMillis << std::endl;
//            std::cout << "deltaTime: " << deltaTime << std::endl;
//            std::cout << "globalTime: " << mGlobalTime << std::endl;
//            std::cout << "mLastRead: " << mLastRead << std::endl;
            
            mBoard->sendCommand(mPacketStack[0].second);
            
            if (mPacketStack.size() == 1) mBoard->sendCommand(penUp()); //if it's the last command in the stack, make sure it's followed by a penUp
            
            previousCommandDuration = mPacketStack[0].first; //record the current commands duration for the next iteration of the loop
            
            auto p = mPacketStack.begin(); //create an iterator that represents the first element in the stack - ie. the bottom
            mPacketStack.erase(p); //remove the command from the bottom of the stack
            
            
            
            std::cout << "sending and erasing cmd: " << mPacketStack[0].second << std::endl;
            mLastRead = 0.0;
            
        }

    }
    
}





/************************************************************************
 *
 *               C R E A T E  G R O U P
 *
 ************************************************************************/
void PlotBot::createGroup(SketchTools::PencilLineRef _thisPencilLine)
{
    
    std::vector<ci::ivec2> tempPoints = _thisPencilLine->getPoints();
    
    
    ci::vec2 prevPoint, currentPoint;
    
    timedPacket penDownPacket = std::make_pair(10, penDown());
    mPacketStack.push_back(penDownPacket);
    
    //cycle through the array of points and check the distance between neighbours - if the distance is too small, remove it from the container

    std::cout << " >>>> BEFORE <<<<< " <<std::endl;
    for(auto i : tempPoints) std::cout << i << std::endl;
    std::cout << std::endl;
    
    for (int i = 1; i < tempPoints.size();)
    {
        currentPoint = tempPoints[i];
        prevPoint = tempPoints[i-1];
        
        ci::vec2 dir = currentPoint - prevPoint;
        float dist = length(dir);
        std::cout << "distance: " << dist << std::endl;
        
        if (dist < 3) tempPoints.erase(tempPoints.begin() + i);
        else i++; //only increment through the loop if we didn't have to erase the current element
    }
    
    std::cout << " >>>> AFTER <<<<< " <<std::endl;
    for(auto i : tempPoints) std::cout << i << std::endl;
    std::cout << std::endl;
    
    
    
    
    for (int i = 1; i < tempPoints.size(); i++)
    {
        
        
        //create a draw command between these two points
        
        std::string drawCmd;
        
        int dirX, dirY; //which direction are we moving in?
        
        currentPoint = tempPoints[i];
        prevPoint = tempPoints[i-1];
        
        ci::vec2 dir = currentPoint - prevPoint;
        float dist = length(dir);
        
        prevPoint.x < currentPoint.x ? dirX = 1 : dirX = -1;
        prevPoint.y < currentPoint.y ? dirY = 1 : dirY = -1;
        
        int pixelDistX = std::abs(prevPoint.x - currentPoint.x);
        int pixelDistY = std::abs(prevPoint.y - currentPoint.y);
        
        double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
        double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
        
        //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
        //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
        
        int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
        
        double moveDurationINsecs = moveDistanceINmm / mVelocity;
        
        int moveDurationINmillis = moveDurationINsecs * 1000;
        
        int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * dirX;
        int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * dirY;
        
        drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
        
        //add the move command
        timedPacket drawPacket = std::make_pair(moveDurationINmillis, drawCmd);
        if (moveDurationINmillis != 0) mPacketStack.push_back(drawPacket);
        
        std::cout << "adding draw command: " << drawCmd << " - which will take " << moveDurationINmillis << "ms to execute and will travel a distance of " << moveDistanceINmm << "mm on the page" << std::endl;
        
        
        
    }
    



    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = tempPoints[tempPoints.size()-1];
     
    
    
}





/************************************************************************
 *
 *               C R E A T E  C I R C L E
 *
 ************************************************************************/
void PlotBot::createCircle(SketchTools::CircleRef _thisCircle)
{
 
    std::vector<ci::ivec2> tempPoints = _thisCircle->getPoints();
    
    
    ci::vec2 prevPoint, currentPoint;
    
    addMoveCmd(tempPoints[0]);
    
    timedPacket penDownPacket = std::make_pair(10, penDown());
    mPacketStack.push_back(penDownPacket);
    

    for (int i = 1; i < tempPoints.size(); i++)
    {
        
        
        //create a draw command between these two points
        
        std::string drawCmd;
        
        int dirX, dirY; //which direction are we moving in?
        
        currentPoint = tempPoints[i];
        prevPoint = tempPoints[i-1];
        
        ci::vec2 dir = currentPoint - prevPoint;
        float dist = length(dir);
        
        prevPoint.x < currentPoint.x ? dirX = 1 : dirX = -1;
        prevPoint.y < currentPoint.y ? dirY = 1 : dirY = -1;
        
        int pixelDistX = std::abs(prevPoint.x - currentPoint.x);
        int pixelDistY = std::abs(prevPoint.y - currentPoint.y);
        
        double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
        double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
        
        //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
        //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
        
        int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
        
        double moveDurationINsecs = moveDistanceINmm / mVelocity;
        
        int moveDurationINmillis = moveDurationINsecs * 1000;
        
        int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * dirX;
        int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * dirY;
        
        drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
        
        //add the move command
        timedPacket drawPacket = std::make_pair(moveDurationINmillis, drawCmd);
        if (moveDurationINmillis != 0) mPacketStack.push_back(drawPacket);
        
        std::cout << "adding draw command: " << drawCmd << " - which will take " << moveDurationINmillis << "ms to execute and will travel a distance of " << moveDistanceINmm << "mm on the page" << std::endl;
        
        
        
    }
    
    
    //connect the last point to the first, thus closing the circle
    {
        std::string drawCmd;
        
        int dirX, dirY; //which direction are we moving in?
        
        ci::vec2 endPoint, currPoint;
        endPoint = tempPoints[0];
        currPoint = tempPoints[tempPoints.size() - 1];
        
        currPoint.x < endPoint.x ? dirX = 1 : dirX = -1;
        currPoint.y < endPoint.y ? dirY = 1 : dirY = -1;
        
        int pixelDistX = std::abs(currPoint.x - endPoint.x);
        int pixelDistY = std::abs(currPoint.y - endPoint.y);
        
        double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
        double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
        
        //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
        //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
        
        int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
        
        double moveDurationINsecs = moveDistanceINmm / mVelocity;
        
        int moveDurationINmillis = moveDurationINsecs * 1000;
        
        int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * dirX;
        int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * dirY;
        
        drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
        
        //add the move command
        timedPacket drawPacket = std::make_pair(moveDurationINmillis, drawCmd);
        if (moveDurationINmillis != 0) mPacketStack.push_back(drawPacket);
        
        std::cout << "adding draw command: " << drawCmd << " - which will take " << moveDurationINmillis << "ms to execute and will travel a distance of " << moveDistanceINmm << "mm on the page" << std::endl;
        
        //update pen's position (in pixel space) that will be used by the next command
        mPenPixelPosition = endPoint;
    }
    
    
    

    
}

/************************************************************************
 *
 *               C R E A T E  P I X E L  I M A G E
 *
 ************************************************************************/
void PlotBot::createPixelImage(std::vector<ci::vec2> _points)
{
    std::cout << "creating pixel image" << std::endl;
    
    for (int i = 0; i < _points.size(); i++)
    {
        addPixel(_points[i]);
        std::cout << "adding pixel: " << _points[i] << std::endl;
        timedPacket penDownPacket = std::make_pair(10, penDown());
        mPacketStack.push_back(penDownPacket);
    }
    
    
}



/************************************************************************
 *
 *               J O G  R I G H T
 *
 ************************************************************************/
void PlotBot::jogRight()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = jogDistance;   //<<<<<<<
    int pixelDistY = 0;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue();        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue();         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x + jogDistance, mPenPixelPosition.y);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}



/************************************************************************
 *
 *               J O G  L E F T
 *
 ************************************************************************/
void PlotBot::jogLeft()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
        //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = jogDistance;   //<<<<<<<
    int pixelDistY = 0;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * -1;        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue();         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x - jogDistance, mPenPixelPosition.y);           //<<<<<<<
    
     std::cout << "new pen position is: " << mPenPixelPosition << std::endl;
}



/************************************************************************
 *
 *               J O G  U P
 *
 ************************************************************************/
void PlotBot::jogUp()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = 0;                     //<<<<<<<
    int pixelDistY = jogDistance;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue();        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * -1;         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x, mPenPixelPosition.y - jogDistance);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}



/************************************************************************
 *
 *               J O G  D O W N
 *
 ************************************************************************/

void PlotBot::jogDown()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = 0;                     //<<<<<<<
    int pixelDistY = jogDistance;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue();        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue();         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x, mPenPixelPosition.y + jogDistance);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}

/************************************************************************
 *
 *               J O G  U P  L E F T
 *
 ************************************************************************/
void PlotBot::jogUpLeft()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = jogDistance;           //<<<<<<<
    int pixelDistY = jogDistance;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * -1;        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * -1;         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x - jogDistance, mPenPixelPosition.y - jogDistance);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}

/************************************************************************
 *
 *               J O G  U P  R I G H T
 *
 ************************************************************************/
void PlotBot::jogUpRight()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = jogDistance;   //<<<<<<<
    int pixelDistY = jogDistance;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue();        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue() * -1;         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x + jogDistance, mPenPixelPosition.y - jogDistance);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}

/************************************************************************
 *
 *               J O G  D O W N  R I G H T
 *
 ************************************************************************/
void PlotBot::jogDownRight()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = jogDistance;   //<<<<<<<
    int pixelDistY = jogDistance;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue();        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue();         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x + jogDistance, mPenPixelPosition.y + jogDistance);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}

/************************************************************************
 *
 *               J O G  D O W N  L E F T
 *
 ************************************************************************/
void PlotBot::jogDownLeft()
{
    std::cout << "pen position was: " << mPenPixelPosition << std::endl;
    //UPDATE PEN POSITION
    std::string drawCmd;
    
    int pixelDistX = jogDistance;   //<<<<<<<
    int pixelDistY = jogDistance;             //<<<<<<<
    
    double mmDistX = SketchTools::convertPixelsTOmmX(pixelDistX);
    double mmDistY = SketchTools::convertPixelsTOmmY(pixelDistY);
    
    //WE NEED TO WORK OUT HOW LONG IT WILL TAKE THE MOVE TO COMPLETE: dt = ds / v
    //WHERE WE USE PYTHAGOREAN THEOREM TO WORK OUT THE HYPOTENUSE OF mmDistX and mmDistY
    
    int moveDistanceINmm = sqrt( pow(mmDistX, 2) + pow(mmDistY,2));
    
    double moveDurationINsecs = moveDistanceINmm / mVelocity;
    
    int moveDurationINmillis = moveDurationINsecs * 1000;
    
    int numStepsX = (mmDistX / mFullStepDist) * mBoard->getStepModeValue() * -1;        //<<<<<<<
    int numStepsY = (mmDistY / mFullStepDist) * mBoard->getStepModeValue();         //<<<<<<<
    
    drawCmd = "SM," + std::to_string(moveDurationINmillis) + "," + std::to_string(numStepsX) + "," + std::to_string(numStepsY) + "\r";
    
    mBoard->sendCommand(drawCmd);
    
    //update pen's position (in pixel space) that will be used by the next command
    mPenPixelPosition = ci::ivec2(mPenPixelPosition.x - jogDistance, mPenPixelPosition.y + jogDistance);           //<<<<<<<
    
    std::cout << "new pen position is: " << mPenPixelPosition << std::endl << std::endl;
}





/************************************************************************
 *
 *               R U N  S Y S T E M
 *
 ************************************************************************/
void PlotBot::runSystem()
{
    systemPaused = false;
    std::cout << " >>>> SYSTEM IS PAUSED <<<<< " << std::endl;
}

/************************************************************************
 *
 *               P A U S E  S Y S T E M
 *
 ************************************************************************/
void PlotBot::pauseSystem()
{
    systemPaused = true;
    std::cout << " >>>>> RESUMING NORMAL OPERATIONS <<<<<< " << std::endl;
}



/************************************************************************
 *
 *               Z E R O   A X E S
 *
 ************************************************************************/
void PlotBot::zeroAxes()
{
    std::cout << "zeroing axes" << std::endl;
    mOperationMode = HOMING;
    xHome = false;
    yHome = false;
}




/************************************************************************
 *
 *               M O V E   T O   O R I G I N
 *
 ************************************************************************/

void PlotBot::moveToOrigin()
{

    std::cout<< std::endl << "----------------------------------------------------------------" << std::endl << std::endl;
    
    std::cout << "frame number: " << ci::app::getElapsedFrames() << std::endl << std::endl;
    
    //IF BOTH THE X AND Y-AXES ARE AT HOME, THEN THERE'S NO LONGER A NEED TO HOMING, SET THE LOCATION TO 0,0
    if (xHome && yHome)
    {
        xHome = false;
        yHome = false;
        mOperationMode = NORMAL_OPERATION;
        mPenPixelPosition = ci::vec2(0);
        return;
    }
    
    //CHECK FOR RESPONSE FROM BOARD FOR BOTH LIMIT SWITCH
    
    if (mBoard->mSerial->getNumBytesAvailable() > 0)
    {
        std::cout<< "CHECK ------ " << std::endl;
        auto numBytes = mBoard->mSerial->getNumBytesAvailable();
        uint8_t packet[numBytes];                               //CHECK HOW MANY BYTES ARE AVAILABLE
        mBoard->mSerial->readBytes(packet, numBytes);                   //READ AVAILABLE BYTES AND PLACE THEM IN 'PACKET' BUFFER
        
        //THE CURRENT SWITCH STATE DEPENDS ON WHICH CORRESPONDING PIN-MOTOR PAIR IS BEING ADDRESSED
        switch (mAxisState) {
                std::cout << "bytes received dawg, now checking which axis is engaged.." << std::endl;
                
            case X_AXIS:
                std::cout << "X-Axis engaged in check mode" << std::endl;
                
                switch (mHomingMode)
            {
                    
                case CHECK_LIMIT_RESPONSE_X:
                    
                    std::cout << "CHECK_LIMIT_RESPONSE_X activated..." << std::endl;
                    
                    if (numBytes == 6)
                    {
                        std::string packetString = std::to_string(packet[3]);    //THIS HANDLES THE REQUEST FOR THE PIN STATE
                        std::cout << "numBytes: " << numBytes << " , packet: " << packetString << std::endl << std::endl;
                        
                        if (packetString.compare("48") == 0)
                        {
                            std::cout << "X-Axis gantry has reached the origin, switching axis state to Y-AXIS and mode to REQUEST_LIMIT_Y" << std::endl << std::endl;
                            xHome = true;
                            mAxisState = Y_AXIS;
                            mHomingMode = REQUEST_LIMIT_STATE_Y;
                        }
                        
                        if (packetString.compare("49") == 0)
                        {
                            std::cout << "We're not home yet, switching mode to MOVE_MOTOR_X" << std::endl;
                            mHomingMode = MOVE_MOTOR_X;
                        }
                        
                    }
                    
                    break;
                    
                case CHECK_MOTOR_RESPONSE_X:
                    
                    std::cout << "CHECK_MOTOR_RESPONSE_X activated..." << std::endl;
                    
                    std::cout << "numBytes: " << numBytes << ", packet: " << packet << std::endl;
                    
                    if (numBytes == 4)
                    {
                        std::string packetString = std::to_string(packet[2]) + std::to_string(packet[3]);
                        
                        std::cout << "packet string is: " << packetString << std::endl;
                        
                        //IF THE RESPONSE PACKET INDICATES THAT THE MOTOR SUCCESSFULLY MOVED THE X-AXIS, IT'S
                        //TIME TO CHECK THE STATE OF THE Y-AXIS LIMIT SWITCH
                        if (packetString.compare("1310") == 0 && !yHome)
                        {
                            std::cout << "motor response for x-axis was good, switching to y-axis and mode to REQUEST_LIMIT_STATE_Y" << std::endl;
                            mAxisState = Y_AXIS;
                            mHomingMode = REQUEST_LIMIT_STATE_Y;
                            
                        } else if (packetString.compare("1310") == 0 && yHome)
                        {
                            std::cout << "motor response for x-axis was good, but the y-axis is home, so let's stay on the X-AXIS and switch mode to REQUEST_LIMIT_STATE_X" << std::endl;
                            mHomingMode = REQUEST_LIMIT_STATE_X;
                            
                        }
                        
                        //                                else {
                        //                                    mHomingMode = REQUEST_LIMIT_STATE_X; //this might need to change to send motor command again
                        //                                }
                        
                    }
                    
                    break;
                    
                default:
                    break;
                    
            } //END OF mHomingMode SWITCH STATEMENT
                
                break;
                
                
            case Y_AXIS:
                std::cout << "Y-Axis engaged in check mode" << std::endl;
                
                
                switch(mHomingMode)
            {
                    
                case CHECK_LIMIT_RESPONSE_Y:
                    
                    if (numBytes == 6)
                    {
                        std::string packetString = std::to_string(packet[3]);    //THIS HANDLES THE REQUEST FOR THE PIN STATE
                        std::cout << "numBytes: " << numBytes << " , packet: " << packetString << std::endl << std::endl;
                        
                        //IF THE Y-AXIS LIMIT-SWITCH STATE IS LOW, THE DRAWING HEAD HAS REACHED THE Y-AXIS ORIGIN
                        if (packetString.compare("48") == 0)
                        {
                            std::cout << "Y-Axis gantry has reached the origin, switching axis state to X-AXIS and mode to REQUEST_LIMIT_X" << std::endl << std::endl;
                            yHome = true;
                            mAxisState = X_AXIS;
                            mHomingMode = REQUEST_LIMIT_STATE_X;
                            
                        }
                        
                        if (packetString.compare("49") == 0)
                        {
                            std::cout << "We're not home yet, switching mode to MOVE_MOTOR_Y" << std::endl;
                            mHomingMode = MOVE_MOTOR_Y;
                        }
                        
                    }
                    
                    break;
                    
                case CHECK_MOTOR_RESPONSE_Y:
                    
                    //IF THE RESPONSE PACKET INDICATES THAT THE MOTOR SUCCESSFULLY MOVED ALONG THE Y-AXIS, IT'S
                    //TIME TO CHECK THE STATE OF THE X-AXIS LIMIT SWITCH
                    if (numBytes == 4)
                    {
                        std::string packetString = std::to_string(packet[2]) + std::to_string(packet[3]);
                        
                        //IF THE RESPONSE PACKET INDICATES THAT THE MOTOR SUCCESSFULLY MOVED THE X-AXIS, IT'S
                        //TIME TO CHECK THE STATE OF THE Y-AXIS LIMIT SWITCH
                        if (packetString.compare("1310") == 0)
                        {
                            std::cout << "motor response was good, x-axis is home though, so no need to switch, let's switch mode to REQUEST_LIMIT_STATE_Y" << std::endl;
                            mHomingMode = REQUEST_LIMIT_STATE_Y;
                        }
                        
                    }
                    
                    break;
                    
                default:
                    break;
                    
            } //END OF mHomingMode SWITCH STATEMENT
                
                
                break;
                
            default:
                break;
                
        }   //END OF AXIS_MODE SWITCH STATEMENT
        
        
        
        
        mBoard->flushBuffer();
        
    } //END OF CHECK BYTES AVAILABLE
    

        std::cout << "SEND ------" << std::endl;
        
        //THE CURRENT SWITCH STATE DEPENDS ON WHICH CORRESPONDING PIN-MOTOR PAIR IS BEING ADDRESSED
        switch (mAxisState) {
                
            case X_AXIS:
                std::cout << std::endl << "we're about to send a command... for the x-axis" << std::endl;
                
                switch (mHomingMode)
            {
                case REQUEST_LIMIT_STATE_X:
                    
                    
                    if(!xHome) //ONLY SEND THE REQUEST IF THE GANTRY IS NOT AT HOME
                    {
                        std::cout << "Sending pin-state request for X and switching mode to CHECK_LIMIT_RESPONSE_X" << std::endl;
                        mBoard->sendCommand("PI,A,2\r");
                        mHomingMode = CHECK_LIMIT_RESPONSE_X;
                    }
                    
                    break;
                    
                case MOVE_MOTOR_X:
                    
                    if (!xHome) //ONLY MOVE THE MOTOR IF THE GANTRY IS NOT AT HOME
                    {
                        std::cout << "Moving x-axis 1mm and switching mode to CHECK_MOTOR_RESPONSE_X" << std::endl;
                        mBoard->sendCommand("SM,50,-100,0\r"); //this moves the x-axis 1mm toward the origin in 30ms
                        mHomingMode = CHECK_MOTOR_RESPONSE_X;
                        std::cout << "homing mode is now: " << mHomingMode << std::endl;
                    }
                    
                default:
                    break;
            }
                
                break;
                
                
            case Y_AXIS:
                
                std::cout << "we're about to send a command... for the y-axis" << std::endl;
                
                switch(mHomingMode)
            {
                    
                case REQUEST_LIMIT_STATE_Y:
                    
                    if(!yHome) //ONLY SEND THE REQUEST IF THE GANTRY IS NOT AT HOME
                    {
                        std::cout << "Sending pin-state request for Y and switching mode to CHECK_LIMIT_RESPONSE_Y" << std::endl;
                        mBoard->sendCommand("PI,A,1\r");
                        mHomingMode = CHECK_LIMIT_RESPONSE_Y;
                    }
                    
                    break;
                    
                case MOVE_MOTOR_Y:
                    
                    if (!yHome) //ONLY MOVE THE MOTOR IF THE GANTRY IS NOT AT HOME
                    {
                        std::cout << "Moving y-axis 1mm and switching Mode to CHECK_MOTOR_RESPONSE_Y" << std::endl;
                        mBoard->sendCommand("SM,50,0,-100\r"); //this moves the x-axis 1mm toward the origin in 30ms
                        mHomingMode = CHECK_MOTOR_RESPONSE_Y;
                    }
                    
                    break;
                    
                default:
                    break;
                    
            } //END OF mHomingMode SWITCH STATEMENT
                
            default:
                break;
                
        } //END OF mAxisMode SWITCH STATEMENT

    
}





/************************************************************************
 *
 *               G E T  C A N V A S
 *
 ************************************************************************/

CanvasRef PlotBot::getCanvas()
{
    return mDigitalCanvas;
}