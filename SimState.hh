#pragma once
#ifndef _SIMSTATE_HH_
#define _SIMSTATE_HH_

#include "predefs.hh"


// SimState - interface for states.
class SimState
{
  public:
	virtual ~SimState();

	virtual void Create( SimApp& ) = 0;
	virtual void Destroy() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
};

#endif
