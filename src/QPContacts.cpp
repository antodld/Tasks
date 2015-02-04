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

// associated header
#include "QPContacts.h"

// includes
// std
#include <stdexcept>

// Eigen
#include <Eigen/Geometry>

//boost
#include <boost/math/constants/constants.hpp>


namespace tasks
{

namespace qp
{


/// @throw std::domain_error if points is not a valid points index.
void checkRange(int point, const std::vector<Eigen::Vector3d>& points)
{
	if(point < 0 || point >= static_cast<int>(points.size()))
	{
		std::ostringstream str;
		str << "invalid point index: must be in the range [0," << points.size()
				<< "[";
		throw std::domain_error(str.str());
	}
}



/**
	*													FrictionCone
	*/



FrictionCone::FrictionCone(const Eigen::Matrix3d& frame, int nrGen, double mu,
	double dir):
	generators(nrGen)
{
	Eigen::Vector3d normal(frame.row(2));
	Eigen::Vector3d tan(dir*frame.row(0));
	double angle = std::atan(mu);

	Eigen::Vector3d gen = Eigen::AngleAxisd(angle, tan)*normal;
	double step = (boost::math::constants::pi<double>()*2.)/nrGen;

	for(int i = 0; i < nrGen; ++i)
	{
		generators[i] = Eigen::AngleAxisd(dir*step*i, normal)*gen;
	}
}



/**
	*													ContactId
	*/



ContactId::ContactId():
	r1Index(-1),
	r2Index(-1),
	r1BodyId(-1),
	r2BodyId(-1),
	numSurf(-1)
{}


ContactId::ContactId(int r1I, int r2I, int r1BId, int r2BId, int nSurf):
	r1Index(r1I),
	r2Index(r2I),
	r1BodyId(r1BId),
	r2BodyId(r2BId),
	numSurf(nSurf)
{}


bool ContactId::operator==(const ContactId& cId) const
{
	return r1Index == cId.r1Index && r2Index == cId.r2Index &&
	r1BodyId == cId.r1BodyId && r2BodyId == cId.r2BodyId &&
	numSurf==cId.numSurf;
}


bool ContactId::operator!=(const ContactId& cId) const
{
	return !((*this) == cId);
}


bool ContactId::operator<(const ContactId& cId) const
{
	return r1Index < cId.r1Index ||
		(r1Index == cId.r1Index && r1BodyId < cId.r1BodyId) ||
		(r1Index == cId.r1Index && r1BodyId == cId.r1BodyId &&
		r2Index < cId.r2Index) ||
		(r1Index == cId.r1Index && r1BodyId == cId.r1BodyId &&
		r2Index == cId.r2Index && r2BodyId < cId.r2BodyId) ||
		(r1Index == cId.r1Index && r1BodyId == cId.r1BodyId &&
		r2Index == cId.r2Index && r2BodyId == cId.r2BodyId &&
		numSurf < cId.numSurf);
}



/**
	*													UnilateralContact
	*/



UnilateralContact::UnilateralContact(int r1I, int r2I,
	int r1BId, int r2BId,
	std::vector<Eigen::Vector3d> r1P,
	const Eigen::Matrix3d& r1Frame,
	const sva::PTransformd& Xbb,
	int nrGen, double mu,
	const sva::PTransformd& Xbs, int nSurf):
	contactId(r1I, r2I, r1BId, r2BId, nSurf),
	r1Points(std::move(r1P)),
	r2Points(),
	r1Cone(r1Frame, nrGen, mu),
	r2Cone(),
	X_b1_b2(Xbb),
	X_b1_s1(Xbs)
{
	construct(r1Frame, nrGen, mu);
}


UnilateralContact::UnilateralContact(const ContactId& cId,
	std::vector<Eigen::Vector3d> r1P,
	const Eigen::Matrix3d& r1Frame,
	const sva::PTransformd& Xbb,
	int nrGen, double mu,
	const sva::PTransformd& Xbs):
	contactId(cId),
	r1Points(std::move(r1P)),
	r2Points(),
	r1Cone(r1Frame, nrGen, mu),
	r2Cone(),
	X_b1_b2(Xbb),
	X_b1_s1(Xbs)
{
	construct(r1Frame, nrGen, mu);
}


Eigen::Vector3d UnilateralContact::force(const Eigen::VectorXd& lambda,
	int /* point */, const FrictionCone& cone) const
{
	Eigen::Vector3d F(Eigen::Vector3d::Zero());

	for(std::size_t i = 0; i < cone.generators.size(); ++i)
	{
		F += cone.generators[i]*lambda(i);
	}

	return F;
}


Eigen::Vector3d UnilateralContact::force(const Eigen::VectorXd& lambda,
	const FrictionCone& cone) const
{
	Eigen::Vector3d F(Eigen::Vector3d::Zero());
	int pos = 0;

	for(int i = 0; i < int(r1Points.size()); ++i)
	{
		F += force(lambda.segment(pos, nrLambda(i)), i, cone);
		pos += nrLambda(i);
	}

	return F;
}


int UnilateralContact::nrLambda(int /* point */) const
{
	return static_cast<int>(r1Cone.generators.size());
}


int UnilateralContact::nrLambda() const
{
	int totalLambda = 0;
	for(int i = 0; i < int(r1Points.size()); ++i)
	{
		totalLambda += nrLambda(i);
	}
	return totalLambda;
}


Eigen::Vector3d UnilateralContact::sForce(const Eigen::VectorXd& lambda,
	int point,
	const FrictionCone& cone) const
{
	checkRange(point, r1Points);
	if(static_cast<int>(lambda.rows()) != nrLambda(point))
	{
		std::ostringstream str;
		str << "number of lambda and generator mismatch: expected ("
				<< nrLambda(point) << ") gived (" << lambda.rows() << ")";
		throw std::domain_error(str.str());
	}

	return force(lambda, point, cone);
}


Eigen::Vector3d UnilateralContact::sForce(const Eigen::VectorXd& lambda,
	const FrictionCone& cone) const
{
	int totalLambda = nrLambda();

	if(static_cast<int>(lambda.rows()) != totalLambda)
	{
		std::ostringstream str;
		str << "number of lambda and generator mismatch: expected ("
				<< totalLambda << ") gived (" << lambda.rows() << ")";
		throw std::domain_error(str.str());
	}

	return force(lambda, cone);
}


int UnilateralContact::sNrLambda(int point) const
{
	checkRange(point, r1Points);
	return nrLambda(point);
}


void UnilateralContact::construct(const Eigen::MatrixXd& r1Frame, int nrGen, double mu)
{
	// compute points in b2 coordinate
	r2Points.reserve(r1Points.size());
	for(const Eigen::Vector3d& p: r1Points)
	{
		r2Points.push_back((sva::PTransformd(p)*X_b1_b2.inv()).translation());
	}

	// compute points frame in b2 coordinate
	//Eigen::Matrix3d r2Frame = (X_b1_b2*sva::PTransformd(Eigen::Matrix3d(r1Frame))).rotation();
	Eigen::Matrix3d r2Frame = (sva::PTransformd(Eigen::Matrix3d(r1Frame))*X_b1_b2.inv()).rotation();

	// create the b2 cone
	// We take the oppostie frame because force are opposed
	r2Cone = FrictionCone(-r2Frame, nrGen, mu, -1.);
}



/**
	*													BilateralContact
	*/



BilateralContact::BilateralContact(int r1I, int r2I,
	int r1BId, int r2BId,
	std::vector<Eigen::Vector3d> r1P,
	const std::vector<Eigen::Matrix3d>& r1Frames,
	const sva::PTransformd& Xbb,
	int nrGen, double mu,
	const sva::PTransformd& Xbs, int nSurf):
	contactId(r1I, r2I, r1BId, r2BId, nSurf),
	r1Points(std::move(r1P)),
	r2Points(),
	r1Cones(r1Points.size()),
	r2Cones(r1Points.size()),
	X_b1_b2(Xbb),
	X_b1_s1(Xbs)
{
	construct(r1Frames, nrGen, mu);
}


BilateralContact::BilateralContact(const ContactId& cId,
	std::vector<Eigen::Vector3d> r1P,
	const std::vector<Eigen::Matrix3d>& r1Frames,
	const sva::PTransformd& Xbb,
	int nrGen, double mu,
	const sva::PTransformd& Xbs):
	contactId(cId),
	r1Points(std::move(r1P)),
	r2Points(),
	r1Cones(r1Points.size()),
	r2Cones(r1Points.size()),
	X_b1_b2(Xbb),
	X_b1_s1(Xbs)
{
	construct(r1Frames, nrGen, mu);
}


BilateralContact::BilateralContact(const UnilateralContact& c):
	contactId(c.contactId),
	r1Points(c.r1Points),
	r2Points(c.r2Points),
	r1Cones(c.r1Points.size(), c.r1Cone),
	r2Cones(c.r1Points.size(), c.r2Cone),
	X_b1_b2(c.X_b1_b2),
	X_b1_s1(c.X_b1_s1)
{ }


Eigen::Vector3d BilateralContact::force(const Eigen::VectorXd& lambda,
	int point, const std::vector<FrictionCone>& cones) const
{
	Eigen::Vector3d F(Eigen::Vector3d::Zero());

	for(std::size_t i = 0; i < cones[point].generators.size(); ++i)
	{
		F += cones[point].generators[i]*lambda(i);
	}

	return F;
}


Eigen::Vector3d BilateralContact::force(const Eigen::VectorXd& lambda,
	const std::vector<FrictionCone>& cones) const
{
	Eigen::Vector3d F(Eigen::Vector3d::Zero());
	int pos = 0;

	for(int i = 0; i < int(r1Points.size()); ++i)
	{
		F += force(lambda.segment(pos, nrLambda(i)), i, cones);
		pos += nrLambda(i);
	}

	return F;
}


int BilateralContact::nrLambda(int point) const
{
	return static_cast<int>(r1Cones[point].generators.size());
}


int BilateralContact::nrLambda() const
{
	int totalLambda = 0;
	for(int i = 0; i < int(r1Points.size()); ++i)
	{
		totalLambda += nrLambda(i);
	}
	return totalLambda;
}


Eigen::Vector3d BilateralContact::sForce(const Eigen::VectorXd& lambda,
	int point, const std::vector<FrictionCone>& cones) const
{
	checkRange(point, r1Points);
	if(static_cast<int>(lambda.rows()) != nrLambda(point))
	{
		std::ostringstream str;
		str << "number of lambda and generator mismatch: expected ("
				<< nrLambda(point) << ") gived (" << lambda.rows() << ")";
		throw std::domain_error(str.str());
	}

	return force(lambda, point, cones);
}


Eigen::Vector3d BilateralContact::sForce(const Eigen::VectorXd& lambda,
	const std::vector<FrictionCone>& cones) const
{
	int totalLambda = nrLambda();

	if(static_cast<int>(lambda.rows()) != totalLambda)
	{
		std::ostringstream str;
		str << "number of lambda and generator mismatch: expected ("
				<< totalLambda << ") gived (" << lambda.rows() << ")";
		throw std::domain_error(str.str());
	}

	return force(lambda, cones);
}


int BilateralContact::sNrLambda(int point) const
{
	checkRange(point, r1Points);
	return nrLambda(point);
}


void BilateralContact::construct(const std::vector<Eigen::Matrix3d>& r1Frames,
	int nrGen, double mu)
{
	assert(r1Points.size() == r1Frames.size());

	r2Points.reserve(r1Points.size());
	sva::PTransformd X_b2_b1(X_b1_b2.inv());
	for(std::size_t i = 0; i < r1Points.size(); ++i)
	{
		// compute point i in b2 coordinate
		sva::PTransformd X_b1_p(r1Frames[i], r1Points[i]);
		sva::PTransformd X_b2_p = X_b1_p*X_b2_b1;
		r2Points.push_back(X_b2_p.translation());

		// construct r1 cone
		r1Cones[i] = FrictionCone(r1Frames[i], nrGen, mu);
		// create the b2 cone
		// We take the oppostie frame because force are opposed
		r2Cones[i] = FrictionCone(-X_b2_p.rotation(), nrGen, mu, -1.);
	}
}


} // namespace qp

} // namespace tasks
