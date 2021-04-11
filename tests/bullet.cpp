/**
 * bullet test
 * @author Tobias Weber
 * @date apr-21
 * @license: see 'LICENSE.GPL' file
 *
 * g++ -std=c++20 -I/usr/include/bullet bullet.cpp -o bullet -lLinearMath -lBulletDynamics -lBulletCollision
 *
 * References:
 *   - https://github.com/bulletphysics/bullet3/blob/master/examples/HelloWorld/HelloWorld.cpp
 */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <cmath>

#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>


struct RigidBody
{
	std::shared_ptr<btPolyhedralConvexShape> shape;
	std::shared_ptr<btDefaultMotionState> state;
	std::shared_ptr<btRigidBody> rigid_body;
};


std::vector<RigidBody> get_objects()
{
	std::vector<RigidBody> objs;

	// create 3 cubes
	for(std::size_t i{0}; i<3; ++i)
	{
		btScalar x{0};
		btScalar y{0.9f*i};
		btScalar z{0};

		RigidBody cube;
		btScalar mass{1.f};
		btVector3 com{0,0,0};
		cube.shape = std::make_shared<btBoxShape>(btVector3{0.5, 0.5, 0.5});
		cube.shape->calculateLocalInertia(mass, com);
		cube.state = std::make_shared<btDefaultMotionState>(
			btTransform{btMatrix3x3::getIdentity(), {x,y,z}});
		cube.rigid_body = std::make_shared<btRigidBody>(
			btRigidBody::btRigidBodyConstructionInfo{mass, cube.state.get(), cube.shape.get(), com});
		objs.emplace_back(std::move(cube));
	}

	// inclined plane
	RigidBody plane;
	plane.state = std::make_shared<btDefaultMotionState>(
		btTransform{{btQuaternion{btVector3{1, 0, 0}, M_PI/8.}}, {0, -3, 0}});
	plane.shape = std::make_shared<btBoxShape>(btVector3{10, 0.1, 10});
	plane.rigid_body = std::make_shared<btRigidBody>(
		btRigidBody::btRigidBodyConstructionInfo{0, plane.state.get(), plane.shape.get(), {0, 0, 0}});
	objs.emplace_back(std::move(plane));

	return objs;
}


void sim(std::shared_ptr<btDynamicsWorld> world, btScalar totaltime, RigidBody& obj)
{
	std::ofstream ofstr("bullet.gpl");
	ofstr.precision(4);

	ofstr
		<< std::setw(8) << std::left << "# time"
		<< std::setw(8) << std::left << "pos_x" << " "
		<< std::setw(8) << std::left << "pos_y" << " "
		<< std::setw(8) << std::left << "pos_z" << " "
		<< std::setw(8) << std::left << "rot_x" << " "
		<< std::setw(8) << std::left << "rot_y" << " "
		<< std::setw(8) << std::left << "rot_z" << " "
		<< std::setw(8) << std::left << "rot_w" << "\n";
	ofstr << "$cube << END\n";

	btScalar time{0};
	btScalar dtime{0.01};

	while(time < totaltime)
	{
		world->stepSimulation(dtime);
		time += dtime;

		btTransform trafo{};
		obj.rigid_body->getMotionState()->getWorldTransform(trafo);
		btVector3 pos = trafo.getOrigin();
		btMatrix3x3 mat = trafo.getBasis();
		btQuaternion rot;
		mat.getRotation(rot);

		ofstr << std::fixed
			<< std::setw(8) << std::left << time
			<< std::setw(8) << std::left << pos.getX() << " "
			<< std::setw(8) << std::left << pos.getY() << " "
			<< std::setw(8) << std::left << pos.getZ() << " "
			<< std::setw(8) << std::left << rot.getX() << " "
			<< std::setw(8) << std::left << rot.getY() << " "
			<< std::setw(8) << std::left << rot.getZ() << " "
			<< std::setw(8) << std::left << rot.getW() << "\n";
	}
	ofstr << "END\n\n";

	ofstr << "set xlabel \"time (s)\"\n";
	ofstr << "set ylabel \"y (m)\"\n";
	ofstr << "set key top right\n";
	ofstr << "plot \\\n";
	ofstr << "\t\"$cube\" using 1:2 with lines linewidth 2 linecolor \"#ff0000\" title \"x\", \\\n";
	ofstr << "\t\"$cube\" using 1:3 with lines linewidth 2 linecolor \"#00ff00\" title \"y\", \\\n";
	ofstr << "\t\"$cube\" using 1:4 with lines linewidth 2 linecolor \"#0000ff\" title \"z\" \\\n";
	ofstr << std::endl;

	std::system("gnuplot -p bullet.gpl");
}


int main()
{
	auto coll = std::make_shared<btDefaultCollisionConfiguration>(btDefaultCollisionConstructionInfo{});
	auto disp = std::make_shared<btCollisionDispatcherMt>(coll.get());
	auto cache = std::make_shared<btDbvtBroadphase>();
	auto solver = std::make_shared<btSequentialImpulseConstraintSolver>();
	//auto world = std::make_shared<btDynamicsWorld>(disp.get(), cache.get(), coll.get());
	auto world = std::make_shared<btDiscreteDynamicsWorld>(disp.get(), cache.get(), solver.get(), coll.get());

	auto objs = get_objects();
	for(auto& obj : objs)
		world->addRigidBody(obj.rigid_body.get());

	world->setGravity({0, -9.81, 0});
	sim(world, 1.5, objs[1]);

	return 0;
}
