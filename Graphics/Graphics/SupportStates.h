#pragma once
#include "State.h"

class SelfMaintain : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	const char* getName() override { return "SelfMaintain"; }
};

class MoveToClient : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	const char* getName() override { return "MoveToClient"; }
};

class ServeClient : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	const char* getName() override { return "ServeClient"; }
};

class IdleSupport : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	const char* getName() override { return "IdleSupport"; }
};

class FleeState : public State
{
public:
	void OnEnter(BaseAgent* agent) override;
	void Execute(BaseAgent* agent) override;
	void OnExit(BaseAgent* agent) override;
	const char* getName() override { return "FleeState"; }
};
