#pragma once
#include <G3D/G3D.h>
#include "ConfigFiles.h"

class Projectile : public Entity {
public:
	shared_ptr<VisibleEntity>       entity;

	Projectile() : m_totalTime(0) {}
	Projectile(const shared_ptr<VisibleEntity>& e, RealTime t = 0) : entity(e), m_totalTime(t) {}
	Projectile(const shared_ptr<VisibleEntity>& e, float velocity, bool collision = true, float gravity = 0.0f, RealTime t = 5.0) :
		entity(e),
		m_totalTime(t),
		m_collision(collision),
		m_velocity(velocity),
		m_gravity(gravity){}
	
	void onSimulation(RealTime dt) {
		// Manage time to display
		m_totalTime -= (float)dt;
		// Update gravitational velocity component
		m_gravVel += m_gravity * (float)dt;
		m_gravVel = fmin(m_gravVel, m_maxVel);
		// Save the last position and update
		m_lastPos = entity->frame().translation;
		entity->setFrame(entity->frame() + entity->frame().lookVector()*m_velocity*(float)dt - Vector3(0,m_gravVel,0)*(float)dt);
	}

	LineSegment getCollisionSegment() {
		return LineSegment::fromTwoPoints(m_lastPos, entity->frame().translation);
	}

	Ray getCollisionRay() {
		const Point3 currPos = entity->frame().translation;
		return Ray::fromOriginAndDirection(currPos, (m_lastPos-currPos).unit());
	}

	Ray getDecalRay() {
		const Point3 currPos = entity->frame().translation;
		return Ray::fromOriginAndDirection(currPos, (currPos - m_lastPos).unit());
	}

	void getLastTwoPoints(Point3& p_old, Point3& p_new) {
		p_old = m_lastPos;
		p_new = entity->frame().translation;
	}

	double remainingTime() { return m_totalTime; }
	void clearRemainingTime() { m_totalTime = 0.0f; }

protected:
	// Timed mode
	double							m_totalTime = 5.0;
	// Propagation mode
	bool							m_collision = false;
	float							m_velocity = 0.0f;
	float							m_gravity = 10.0f;
	float							m_gravVel = 0.0f;
	float							m_maxVel = 100.0f;
	Point3							m_lastPos = Point3::zero();
};


class Weapon : Entity {
public:
	static shared_ptr<Weapon> create(shared_ptr<WeaponConfig> config, shared_ptr<Scene> scene, shared_ptr<Camera> cam, Array<Projectile>* projectiles) {
		return createShared<Weapon>(config, scene, cam, projectiles);
	};

	shared_ptr<TargetEntity> fire(const Array<shared_ptr<TargetEntity>>& targets,
		int& targetIdx,
		float& hitDist, 
		Model::HitInfo& hitInfo, 
		Array<shared_ptr<Entity>>& dontHit);

	void onPose(Array<shared_ptr<Surface> >& surface);

	void loadModels() {
		// Create the view model
		if (m_config->modelSpec.filename != "") {
			m_viewModel = ArticulatedModel::create(m_config->modelSpec, "viewModel");
		}
		else {
			const static Any modelSpec = PARSE_ANY(ArticulatedModel::Specification{
				filename = "model/sniper/sniper.obj";
				preprocess = {
					transformGeometry(all(), Matrix4::yawDegrees(90));
					transformGeometry(all(), Matrix4::scale(1.2,1,0.4));
				};
				scale = 0.25;
				});
			m_viewModel = ArticulatedModel::create(modelSpec, "viewModel");
		}

		// Create the bullet model
		const Vector3& scale = m_config->bulletScale;
		const Color3& color = m_config->bulletColor;
		const static Any bulletSpec = Any::parse(format(
			"ArticulatedModel::Specification{\
				filename = \"ifs/d10.ifs\";\
				preprocess = {\
					transformGeometry(all(), Matrix4::pitchDegrees(90));\
					transformGeometry(all(), Matrix4::scale(%f,%f,%f));\
					setMaterial(all(), UniversalMaterial::Specification {\
						lambertian = Color3(0);\
						emissive = Power3(%f,%f,%f);\
					});\
				};\
			}", 
			scale.x, scale.y, scale.z, color.r, color.g, color.b));
		m_bulletModel = ArticulatedModel::create(bulletSpec, "bulletModel");
	}

	void loadSounds() {
		// Check for play mode specific parameters
		if (notNull(m_fireAudio)) { m_fireAudio->stop(); }
		m_fireSound = Sound::create(System::findDataFile(m_config->fireSound), m_config->isLaser());
	}
	void setConfig(const WeaponConfig& config) { m_config = std::make_shared<WeaponConfig>(config); }
	void setCamera(const shared_ptr<Camera>& cam) { m_camera = cam; }
	void setScene(const shared_ptr<Scene>& scene) { m_scene = scene; }
	void setScoped(bool state = true) { m_scoped = state; }

	void setFiring(bool firing = true) {
		if (firing && !m_firing) {
			m_fireAudio = m_fireSound->play();
		}
		else if (m_firing && !firing) {
			m_fireAudio->stop();
		}
		m_firing = firing;
	}

	void setProjectiles(Array<Projectile>* projectileArray) { m_projectiles = projectileArray; };
	bool scoped() { return m_scoped;  }
	bool firing() { return m_firing; }

protected:
	Weapon(shared_ptr<WeaponConfig> config, shared_ptr<Scene>& scene, shared_ptr<Camera>& cam, Array<Projectile>* projectiles) : 
		m_config(config), m_scene(scene), m_camera(cam), m_projectiles(projectiles) {};

	shared_ptr<ArticulatedModel>    m_viewModel;						///< Model for the weapon
	shared_ptr<ArticulatedModel>    m_bulletModel;						///< Model for the "bullet"
	shared_ptr<Sound>               m_fireSound;						///< Sound for weapon firing
	shared_ptr<AudioChannel>		m_fireAudio;						///< Audio channel for fire sound

	shared_ptr<WeaponConfig>		m_config;							///< Weapon configuration
	int								m_lastBulletId = 0;					///< Bullet ID (auto incremented)
	bool							m_scoped = false;					///< Flag used for scope management
	bool							m_firing = false;					///< Flag used for auto fire management

	shared_ptr<Scene>				m_scene;							///< Scene for weapon
	shared_ptr<Camera>				m_camera;							///< Camera for weapon
	Array<Projectile>*				m_projectiles;						///< Projectile array
};