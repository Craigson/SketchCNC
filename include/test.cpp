//
//  test.cpp
//  SketchCNC
//
//  Created by itp student on 4/29/16.
//
//

#include "test.hpp"

/*

typedef std::shared_ptr<class Something> SomethingRef;
typedef std::shared_ptr<const class Something> SomethingConstRef;

class Something : public std::enable_shared_from_this<Something>
{
    Something()
    {
        // THIS WILL FAIL: getRef();
    }
    
    void initialize()
    {
        // THIS IS OK: getRef();
    }
    
    SomethingRef getRef()
    {
        return shared_from_this();
    }
    
    SomethingConstRef getRef() const
    {
        return shared_from_this();
    }
    
    static SomethingRef create()
    {
        SomethingRef r = std::make_shared<Something>();
        r->initialize();
        return r;
    }
};

*/