#pragma once

class BaseAgent;

// FSM state interface
class State
{
public:
	virtual void OnEnter(BaseAgent* agent) = 0;
	virtual void Execute(BaseAgent* agent) = 0;
	virtual void OnExit(BaseAgent* agent) = 0;
	virtual bool requiresFighter() { return false; }
	virtual const char* getName() { return "Unknown"; }
	virtual ~State() {}
};
