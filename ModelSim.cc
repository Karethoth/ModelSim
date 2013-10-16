#include "SimApp.hh"

#include <exception>
#include <memory>

int main( void )
{
	// This try/catch will catch potential exception launched by ogre or by my program.
	// Ogre can launch 'Ogre::Exception' for example.
	try
	{
		SimApp sim;
		sim.Init();
		
		while( !sim.IsClosed() )
		{
			sim.Update();
		}
		
		sim.Unload();
	}
	catch( std::exception &e )
	{
		std::cout << "Exception: " << e.what() << std::endl;
		system( "pause" );
	}

    return 0;
}
