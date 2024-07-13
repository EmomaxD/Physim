#pragma once

#include "Qbit.h"

#include "Pendulum.h"
#include "Cloth.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class QbitAppLayer : public Qbit::Layer
{
public:
	QbitAppLayer();
	virtual ~QbitAppLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Qbit::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Qbit::Event& e) override;
private:
	Qbit::OrthographicCameraController m_CameraController;

	Pendulum pendulum1 = { 1.0, 1.0 }; // length 1m, mass 1kg
	Pendulum pendulum2 = { 1.0, 1.0 }; // length 1m, mass 1kg

	// Define the parameters
	Parameters params = { 9.81 }; // acceleration due to gravity

	// Define the initial state
	State state = { M_PI / 2, M_PI / 2, 0.0, 0.0 }; // initial angles and angular velocities

	Cloth cloth;
};

