#pragma once

#include "Qbit.h"

#include "Pendulum.h"
#include "Cloth.h"
#include "Rope.h"

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

	QP::Pendulum pendulum1 = { 1.0, 1.0 }; // length 1m, mass 1kg
	QP::Pendulum pendulum2 = { 1.0, 1.0 }; // length 1m, mass 1kg

	QP::Parameters params = { 9.81 };
	
	QP::State state = { M_PI / 2, M_PI / 2, 0.0, 0.0 };

	QP::Cloth cloth;

	QP::Rope rope;
};

