#pragma once
#include "ofThread.h"
#include "ofThreadChannel.h"
#include "ofEvent.h"
#include "ofEventUtils.h"


// ofxCommandThread
//--------------------------------------------------------------------------------
class ofxCommandThread : private ofThread
{
public:
  ofxCommandThread();
  ~ofxCommandThread();
  
  struct Arg
  {
    string timestamp;
    string cmd;
    string result;
  };
  
  using Callback = std::function< void( ofxCommandThread::Arg& ) >;
  
  void setCallback( const shared_ptr< Callback > _callback );
  
  void update();
  
  void add( const Arg& _a );
  
  int  getNumQueue();
  
  string exec( char* _cmd );
  
private:
  void threadedFunction();
  
  ofThreadChannel< Arg > queue_before;
  ofThreadChannel< Arg > queue_after;
  int                    numQueue;
  
  shared_ptr< Callback > callback;
};


// ofxMultiThreadCommand
//--------------------------------------------------------------------------------
class ofxMultiThreadCommand
{
public:
  ofxMultiThreadCommand();
  
  int  getNextThreadIndex();
  
  void add( const ofxCommandThread::Arg& _a );
  void add( ofxCommandThread::Arg&& _a );
  
  void update();
  
  ofEvent< ofxCommandThread::Arg >         commandComplete;
  
private:
  const static int                         NUM_THREADS = 2;
  ofxCommandThread                         threads[ NUM_THREADS ];
  shared_ptr< ofxCommandThread::Callback > callback = nullptr;
};