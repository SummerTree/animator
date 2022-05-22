#include "bullet_scene.h"
#include "bullet_rigidbody.h"
#include "bullet_joint.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>

namespace octoon
{
	struct FilterCallback : public btOverlapFilterCallback
	{
		virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
		{
			bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
			collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			return collides;
		}
	};

	BulletScene::BulletScene(PhysicsSceneDesc desc)
		: broadphase_(std::make_unique<btDbvtBroadphase>())
		, collisionConfiguration_(std::make_unique<btDefaultCollisionConfiguration>())
		, filterCallback_(std::make_unique<FilterCallback>())
		, solver_(std::make_unique<btSequentialImpulseConstraintSolver>())
		, maxSubSteps_(1)
		, fixedTimeStep_(1.0f / 60.f)
	{
		dispatcher_ = std::make_unique<btCollisionDispatcher>(collisionConfiguration_.get());

		dynamicsWorld_ = std::make_unique<btDiscreteDynamicsWorld>(dispatcher_.get(), broadphase_.get(), solver_.get(), collisionConfiguration_.get());
		dynamicsWorld_->setGravity(btVector3(desc.gravity.x, desc.gravity.y, desc.gravity.z));
		dynamicsWorld_->getPairCache()->setOverlapFilterCallback(filterCallback_.get());
	}

	BulletScene::~BulletScene()
	{
		dynamicsWorld_.reset();
		dispatcher_.reset();
	}

	void
	BulletScene::setGravity(const math::float3& gravity) noexcept
	{
		dynamicsWorld_->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
	}

	math::float3
	BulletScene::getGravity() const noexcept
	{
		auto gravity = dynamicsWorld_->getGravity();
		return math::float3(gravity.getX(), gravity.getY(), gravity.getZ());
	}

	void
	BulletScene::addRigidbody(std::shared_ptr<PhysicsRigidbody> rigidbody)
	{
		auto bulletRigidbody = std::dynamic_pointer_cast<BulletRigidbody>(rigidbody)->getRigidbody();
		this->dynamicsWorld_->addRigidBody(bulletRigidbody, 1 << bulletRigidbody->getUserIndex(), bulletRigidbody->getUserIndex2());
	}

	void
	BulletScene::removeRigidbody(std::shared_ptr<PhysicsRigidbody> rigidbody)
	{
		auto bulletRigidbody = std::dynamic_pointer_cast<BulletRigidbody>(rigidbody);
		this->dynamicsWorld_->removeRigidBody(bulletRigidbody->getRigidbody());
	}

	void
	BulletScene::addConstraint(std::shared_ptr<PhysicsJoint> joint)
	{
		auto constraint = std::dynamic_pointer_cast<BulletJoint>(joint);
		this->dynamicsWorld_->addConstraint(constraint->getConstraint(), true);
	}

	void
	BulletScene::removeConstraint(std::shared_ptr<PhysicsJoint> joint)
	{
		auto constraint = std::dynamic_pointer_cast<BulletJoint>(joint);
		this->dynamicsWorld_->removeConstraint(constraint->getConstraint());
	}

	void
	BulletScene::setMaxSubStepCount(int numSubSteps) noexcept
	{
		maxSubSteps_ = numSubSteps;
	}

	int
	BulletScene::getMaxSubStepCount() noexcept
	{
		return maxSubSteps_;
	}

	void
	BulletScene::setFixedTimeStep(float fixedTimeStep) noexcept
	{
		fixedTimeStep_ = fixedTimeStep;
	}

	float
	BulletScene::getFixedTimeStep() noexcept
	{
		return fixedTimeStep_;
	}

	void
	BulletScene::simulate(float time)
	{
		auto collision = this->dynamicsWorld_->getCollisionObjectArray();
		auto collisionNums = this->dynamicsWorld_->getNumCollisionObjects();

		for (int i = 0; i < collisionNums; ++i)
		{
			btRigidBody* body = btRigidBody::upcast(collision[i]);
			if (body->getUserIndex3())
			{
				this->dynamicsWorld_->removeRigidBody(body);
				this->dynamicsWorld_->addRigidBody(body, 1 << body->getUserIndex(), body->getUserIndex2());

				body->setUserIndex3(false);
			}
		}

		this->dynamicsWorld_->stepSimulation(time, maxSubSteps_, fixedTimeStep_);
	}

	void
	BulletScene::reset()
	{
		btDispatcher* dispatcher = dynamicsWorld_->getDispatcher();

		dynamicsWorld_->getBroadphase()->resetPool(dispatcher);
		dynamicsWorld_->getConstraintSolver()->reset();
		dynamicsWorld_->clearForces();
	}

	void
	BulletScene::fetchResults()
	{
		auto rigidbodies = this->dynamicsWorld_->getCollisionObjectArray();
		auto rigidbodiesNums = rigidbodies.size();

		for (int i = 0; i < rigidbodiesNums; ++i)
		{
			btRigidBody* body = btRigidBody::upcast(rigidbodies[i]);
			if (body->isActive())
			{
				PhysicsListener* listener = static_cast<PhysicsListener*>(body->getUserPointer());
				if (listener)
					listener->onFetchResult();
			}
		}
	}
}