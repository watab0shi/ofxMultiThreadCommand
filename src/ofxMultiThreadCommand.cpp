#include "ofxMultiThreadCommand.h"


// ofxCommandThread
//--------------------------------------------------------------------------------
ofxCommandThread::ofxCommandThread()
: numQueue( 0 )
, callback( nullptr )
{
  startThread();
}


// ~ofxCommandThread
//----------------------------------------
ofxCommandThread::~ofxCommandThread()
{
  queue_before.close();
  queue_after.close();
  waitForThread( true );
}


// setCallback
//----------------------------------------
void ofxCommandThread::setCallback( const std::shared_ptr< Callback > _callback )
{
  callback = _callback;
}


// update
//----------------------------------------
void ofxCommandThread::update()
{
  ofxCommandThread::Arg a;
  while( queue_after.tryReceive( a ) )
  {
    if( callback ) callback->operator()( a );
    
    --numQueue;
  }
}


// add
//----------------------------------------
void ofxCommandThread::add( const Arg& _a )
{
  queue_before.send( _a );
  ++numQueue;
}


// getNumQueue
//----------------------------------------
int ofxCommandThread::getNumQueue()
{
  return numQueue;
}


// CALL THIS DIRECTLY FOR BLOCKING COMMAND
// thanks to: http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
// exec
//----------------------------------------
std::string ofxCommandThread::exec( char* _cmd )
{
#ifdef TARGET_OSX
  FILE* pipe = popen( _cmd, "r" );
#endif
#ifdef TARGET_WIN32
  FILE* pipe = _popen( _cmd, "r" );
#endif
  if( !pipe ) return "ERROR";
  char buffer[ 128 ];
  std::string result = "";
  while( !feof( pipe ) )
  {
    if( fgets( buffer, 128, pipe ) != NULL ) result += buffer;
  }
#ifdef TARGET_OSX
  pclose( pipe );
#endif
#ifdef TARGET_WIN32
  _pclose( pipe );
#endif
  return result;
}


// threadedFunction
//----------------------------------------
void ofxCommandThread::threadedFunction()
{
  Arg a;
  while( queue_before.receive( a ) )
  {
    a.result = exec( ( char* )a.cmd.c_str() );
    queue_after.send( a );
  }
}



// ofxMultiThreadCommand
//--------------------------------------------------------------------------------
ofxMultiThreadCommand::ofxMultiThreadCommand()
{
  callback = std::make_shared< ofxCommandThread::Callback >( [ this ]( ofxCommandThread::Arg& _a ){
    ofNotifyEvent( commandComplete, _a, this );
  } );
  for( auto& t : threads ) t.setCallback( callback );
}


// getNextThreadIndex
//----------------------------------------
int ofxMultiThreadCommand::getNextThreadIndex()
{
  if( NUM_THREADS == 1 )
  {
    return 0;
  }
  else
  {
    int idx = 0;
    int max = 999;
    for( int i = 0; i < NUM_THREADS; ++i )
    {
      int nQ = threads[ i ].getNumQueue();
      if( nQ < max )
      {
        max = nQ;
        idx = i;
      }
    }
    return idx;
  }
}


// add
//----------------------------------------
void ofxMultiThreadCommand::add( const ofxCommandThread::Arg& _a )
{
  threads[ getNextThreadIndex() ].add( _a );
}

void ofxMultiThreadCommand::add( ofxCommandThread::Arg&& _a )
{
  threads[ getNextThreadIndex() ].add( _a );
}


// update
//----------------------------------------
void ofxMultiThreadCommand::update()
{
  for( auto& t : threads ) t.update();
}
