// This file is part of Tasks.
//
// Tasks is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tasks is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Tasks.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

// includes
// Eigen
#include <Eigen/Core>

// Tasks
#include "Tasks.h"
#include "QPSolver.h"

// forward declaration
// RBDyn
namespace rbd
{
class MultiBody;
class MultiBodyConfig;
}

namespace tasks
{

namespace qp
{


class SetPointTask : public Task
{
public:
	SetPointTask(const rbd::MultiBody& mb, HighLevelTask* hlTask,
		double stiffness, double weight);

	double stiffness() const
	{
		return stiffness_;
	}

	void stiffness(double stiffness);

	virtual std::pair<int, int> begin() const
	{
		return std::make_pair(0, 0);
	}

	virtual void updateNrVars(const rbd::MultiBody& /* mb */,
		const SolverData& /* data */) {}
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

private:
	HighLevelTask* hlTask_;

	double stiffness_, stiffnessSqrt_;

	Eigen::MatrixXd Q_;
	Eigen::VectorXd C_;
	Eigen::VectorXd alphaVec_;
};



class TargetObjectiveTask : public Task
{
public:
	TargetObjectiveTask(const rbd::MultiBody& mb, HighLevelTask* hlTask,
		double timeStep, double duration, const Eigen::VectorXd& objDot,
		double weight);

	TargetObjectiveTask(const rbd::MultiBody& mb, HighLevelTask* hlTask,
		double timeStep, double duration, const Eigen::VectorXd& objDot,
		const Eigen::VectorXd& dimWeight, double weight);

	double t0() const
	{
		return t0_;
	}
	void t0(double t)
	{
		t0_ = t;
	}

	double tf() const
	{
		return tf_;
	}
	void tf(double t)
	{
		tf_ = t;
	}

	const Eigen::VectorXd& objDot() const
	{
		return objDot_;
	}
	void objDot(const Eigen::VectorXd& o)
	{
		objDot_ = o;
	}

	const Eigen::VectorXd& dimWeight() const
	{
		return dimWeight_;
	}
	void dimWeight(const Eigen::VectorXd& o)
	{
		dimWeight_ = o;
	}

	const Eigen::VectorXd& phi() const
	{
		return phi_;
	}
	const Eigen::VectorXd& psi() const
	{
		return psi_;
	}


	virtual std::pair<int, int> begin() const
	{
		return std::make_pair(0, 0);
	}

	virtual void updateNrVars(const rbd::MultiBody& /* mb */,
		const SolverData& /* data */) {}
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

private:
	HighLevelTask* hlTask_;

	double t0_, tf_;
	double dt_;
	Eigen::VectorXd objDot_;
	Eigen::VectorXd curObjDot_;
	Eigen::ArrayXd dimWeight_;

	Eigen::VectorXd phi_, psi_;

	Eigen::MatrixXd Q_;
	Eigen::VectorXd C_;
	Eigen::VectorXd alphaVec_;
};



class QuadraticTask : public Task
{
public:
	QuadraticTask(const rbd::MultiBody& mb, HighLevelTask* hlTask,
		double weight);

	virtual std::pair<int, int> begin() const
	{
		return std::make_pair(0, 0);
	}

	virtual void updateNrVars(const rbd::MultiBody& /* mb */,
		const SolverData& /* data */) {}
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

private:
	HighLevelTask* hlTask_;

	Eigen::MatrixXd Q_;
	Eigen::VectorXd C_;
	Eigen::VectorXd alphaVec_;
};



class LinWeightTask : public Task
{
public:
	LinWeightTask(Task* t, double step, double objWeight);

	virtual void weight(double w);

	virtual std::pair<int, int> begin() const;
	virtual void updateNrVars(const rbd::MultiBody& mb,
		const SolverData& data);
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

private:
	Task* task_;

	double step_;
	double objWeight_;
};



class PostureTask : public Task
{
public:
	PostureTask(const rbd::MultiBody& mb, std::vector<std::vector<double>> q,
		double stiffness, double weight);

	tasks::PostureTask& task()
	{
		return pt_;
	}

	void posture(std::vector<std::vector<double>> q)
	{
		pt_.posture(q);
	}

	const std::vector<std::vector<double>> posture() const
	{
		return pt_.posture();
	}

	double stiffness() const
	{
		return stiffness_;
	}

	void stiffness(double stiffness);

	virtual std::pair<int, int> begin() const
	{
		return std::make_pair(0, 0);
	}

	virtual void updateNrVars(const rbd::MultiBody& /* mb */,
		const SolverData& /* data */) {}
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

	const Eigen::VectorXd& eval() const;

private:
	tasks::PostureTask pt_;

	double stiffness_;
	double stiffnessSqrt_;

	Eigen::MatrixXd Q_;
	Eigen::VectorXd C_;
	Eigen::VectorXd alphaVec_;
};



class PositionTask : public HighLevelTask
{
public:
	PositionTask(const rbd::MultiBody& mb, int bodyId, const Eigen::Vector3d& pos,
		const Eigen::Vector3d& bodyPoint=Eigen::Vector3d::Zero());

	tasks::PositionTask& task()
	{
		return pt_;
	}

	void position(const Eigen::Vector3d& pos)
	{
		pt_.position(pos);
	}

	const Eigen::Vector3d& position() const
	{
		return pt_.position();
	}

	void bodyPoint(const Eigen::Vector3d& point)
	{
		pt_.bodyPoint(point);
	}

	const Eigen::Vector3d& bodyPoint() const
	{
		return pt_.bodyPoint();
	}

	virtual int dim();
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& jac();
	virtual const Eigen::MatrixXd& jacDot();
	virtual const Eigen::VectorXd& eval();

private:
	tasks::PositionTask pt_;
};



class OrientationTask : public HighLevelTask
{
public:
	OrientationTask(const rbd::MultiBody& mb, int bodyId, const Eigen::Quaterniond& ori);
	OrientationTask(const rbd::MultiBody& mb, int bodyId, const Eigen::Matrix3d& ori);

	tasks::OrientationTask& task()
	{
		return ot_;
	}

	void orientation(const Eigen::Quaterniond& ori)
	{
		ot_.orientation(ori);
	}

	void orientation(const Eigen::Matrix3d& ori)
	{
		ot_.orientation(ori);
	}

	const Eigen::Matrix3d& orientation() const
	{
		return ot_.orientation();
	}

	virtual int dim();
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& jac();
	virtual const Eigen::MatrixXd& jacDot();
	virtual const Eigen::VectorXd& eval();

private:
	tasks::OrientationTask ot_;
};



class CoMTask : public HighLevelTask
{
public:
	CoMTask(const rbd::MultiBody& mb, const Eigen::Vector3d& com);

	tasks::CoMTask& task()
	{
		return ct_;
	}

	void com(const Eigen::Vector3d& com)
	{
		ct_.com(com);
	}

	const Eigen::Vector3d com() const
	{
		return ct_.com();
	}

	virtual int dim();
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& jac();
	virtual const Eigen::MatrixXd& jacDot();
	virtual const Eigen::VectorXd& eval();

private:
	tasks::CoMTask ct_;
};



class ContactTask : public Task
{
public:
	ContactTask(int bodyId, bool min, double weight):
		Task(weight),
		bodyId_(bodyId),
		begin_(0),
		dir_(min ? 1. : -1),
		Q_(),
		C_()
	{}

	virtual std::pair<int, int> begin() const
	{
		return std::make_pair(begin_, begin_);
	}

	virtual void updateNrVars(const rbd::MultiBody& mb,
		const SolverData& data);
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

private:
	int bodyId_;
	int begin_;
	double dir_;

	Eigen::MatrixXd Q_;
	Eigen::VectorXd C_;
};



class GripperTorqueTask : public Task
{
public:
	GripperTorqueTask(int bodyId, const Eigen::Vector3d& origin,
		const Eigen::Vector3d& axis, double weight):
		Task(weight),
		bodyId_(bodyId),
		origin_(origin),
		axis_(axis),
		begin_(0),
		Q_(),
		C_()
	{}

	virtual std::pair<int, int> begin() const
	{
		return std::make_pair(begin_, begin_);
	}

	virtual void updateNrVars(const rbd::MultiBody& mb,
		const SolverData& data);
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& Q() const;
	virtual const Eigen::VectorXd& C() const;

private:
	int bodyId_;
	Eigen::Vector3d origin_;
	Eigen::Vector3d axis_;
	int begin_;

	Eigen::MatrixXd Q_;
	Eigen::VectorXd C_;
};



class LinVelocityTask : public HighLevelTask
{
public:
	LinVelocityTask(const rbd::MultiBody& mb, int bodyId, const Eigen::Vector3d& vel,
		const Eigen::Vector3d& bodyPoint=Eigen::Vector3d::Zero());

	tasks::LinVelocityTask& task()
	{
		return pt_;
	}

	void velocity(const Eigen::Vector3d& s)
	{
		pt_.velocity(s);
	}

	const Eigen::Vector3d& velocity() const
	{
		return pt_.velocity();
	}

	void bodyPoint(const Eigen::Vector3d& point)
	{
		pt_.bodyPoint(point);
	}

	const Eigen::Vector3d& bodyPoint() const
	{
		return pt_.bodyPoint();
	}

	virtual int dim();
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& jac();
	virtual const Eigen::MatrixXd& jacDot();
	virtual const Eigen::VectorXd& eval();

private:
	tasks::LinVelocityTask pt_;
};



class OrientationTrackingTask : public HighLevelTask
{
public:
	OrientationTrackingTask(const rbd::MultiBody& mb, int bodyId,
		const Eigen::Vector3d& bodyPoint, const Eigen::Vector3d& bodyAxis,
		const std::vector<int>& trackingJointsId,
		const Eigen::Vector3d& trackedPoint);

	tasks::OrientationTrackingTask& task()
	{
		return ott_;
	}

	void trackedPoint(const Eigen::Vector3d& tp)
	{
		ott_.trackedPoint(tp);
	}

	const Eigen::Vector3d& trackedPoint() const
	{
		return ott_.trackedPoint();
	}

	void bodyPoint(const Eigen::Vector3d& bp)
	{
		ott_.bodyPoint(bp);
	}

	const Eigen::Vector3d& bodyPoint() const
	{
		return ott_.bodyPoint();
	}

	void bodyAxis(const Eigen::Vector3d& ba)
	{
		ott_.bodyAxis(ba);
	}

	const Eigen::Vector3d& bodyAxis() const
	{
		return ott_.bodyAxis();
	}

	virtual int dim();
	virtual void update(const rbd::MultiBody& mb, const rbd::MultiBodyConfig& mbc);

	virtual const Eigen::MatrixXd& jac();
	virtual const Eigen::MatrixXd& jacDot();
	virtual const Eigen::VectorXd& eval();

private:
	tasks::OrientationTrackingTask ott_;
};

} // namespace qp

} // namespace tasks
