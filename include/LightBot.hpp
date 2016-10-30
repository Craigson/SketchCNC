//
//  LightBot.hpp
//  SketchCNC
//
//  Created by itp student on 5/10/16.
//
//

#pragma once
#include <stdio.h>
#include "EiBotBoard.hpp"
#include "Canvas.hpp"


typedef std::shared_ptr<class LightBot>          LightBotRef;

//a timed packet contains the command, as well as the duration in millis (stored as an int) for how long the move will take
typedef std::pair<int, std::string>           timedPacket;

//container for all the commandPackets
typedef std::vector<timedPacket>              PacketStack;

class LightBot
{
public:
    
    static LightBotRef create()
    {
        return LightBotRef(new LightBot());
    }
    
    LightBot();
    ~LightBot();
    
    void init();
    void update();
    
    void establishConnection();
    void setLimitSwitchPins();
    
    
protected:
    
    
    EiBotBoardRef   mBoard; //create an instance of an EiBotBoard object
    
    PacketStack     mPacketStack; //global container for keeping all PacketStacks
    
    CanvasRef       mDigitalCanvas;
    
    CanvasRef       getCanvas();
    
};
    