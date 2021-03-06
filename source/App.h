/**
  \file maxPerf/App.h

  Sample application showing how to render simple graphics with maximum throughput and 
  minimum latency by stripping away most high level VFX and convenience features for
  development. This approach is good for some display and perception research. For general
  game and rendering applications, look at the G3D starter app and vrStarter which give very
  performance with a lot of high-level game engine features.

 */
#pragma once
#include <G3D/G3D.h>
#include "ConfigFiles.h"
#include "TargetEntity.h"
#include "PlayerEntity.h"
#include "GuiElements.h"
#include "PyLogger.h"
#include "Weapon.h"

class Session;
class G3Dialog;
class WaypointManager;

// An enum that tracks presentation state within a trial. Duration defined in session.h
// ready: ready scene that happens before beginning of a task.
// task: actual task (e.g. instant hit, tracking, projectile, ...)
// feedback: feedback showing whether task performance was successful or not.
enum PresentationState { initial, ready, task, feedback, scoreboard, complete };

class FloatingCombatText : public VisibleEntity {
protected:
	shared_ptr<GFont> m_font;
	String m_text;
	float m_size;
	Color4 m_color;
	Color4 m_outline;
	Point3 m_offset;
	Point3 m_velocity;
	float m_fade;
	float m_timeout;
	RealTime m_created;

public:
	static shared_ptr<FloatingCombatText> create(String text, shared_ptr<GFont> font, float size, Color4 color, Color4 outlineColor, Point3 offset, Point3 velocity, float fade, float timeout_s) {
		return  createShared<FloatingCombatText>(text, font, size, color, outlineColor, offset, velocity, fade, timeout_s);
	}
	FloatingCombatText(String text, shared_ptr<GFont> font, float size, Color4 color, Color4 outlineColor, Point3 offset, Point3 velocity, float fade, float timeout_s) {
		m_text = text;
		m_font = font;
		m_size = size;
		m_color = color;
		m_outline = outlineColor;
		m_offset = offset;
		m_velocity = velocity;
		m_fade = fade;
		m_timeout = timeout_s;
		m_created = System::time();		// Capture the time at which this was created
	}

	bool draw(RenderDevice* rd, const Camera& camera, const Framebuffer& framebuffer) {
		// Abort if the timeout has expired (return false to remove this combat text from the tracked array)
		float time_existing = static_cast<float>(System::time() - m_created);
		if (time_existing > m_timeout) {
			return false;
		}

		// Project entity position into image space
		Rect2D viewport = Rect2D(framebuffer.vector2Bounds());
		Point3 position = camera.project(frame().translation, viewport);

		// Abort if the target is not in front of the camera 
		Vector3 diffVector = frame().translation - camera.frame().translation;
		if (camera.frame().lookRay().direction().dot(diffVector) < 0.0f) {
			return true;
		}
		// Abort if the target is not in the view frustum
		if (position == Point3::inf()) {
			return true;
		}
		position += m_offset;						// Apply (initial) offset in pixels
		position += time_existing * m_velocity;		// Update the position based on velocity

		// Apply the fade
		m_color.a *= m_fade;						
		m_outline.a *= m_fade;

		// Draw the font
		m_font->draw2D(rd, m_text, position.xy(), m_size, m_color, m_outline, GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
		return true;
	}
};

class App : public GApp {
protected:
	static const int						MAX_HISTORY_TIMING_FRAMES = 360;	///< Length of the history queue for m_frameDurationQueue
	shared_ptr<Sound>						m_sceneHitSound;					///< Sound for target exploding

	shared_ptr<GFont>						m_combatFont;						///< Font used for floating combat text
	Array<shared_ptr<FloatingCombatText>>	m_combatTextList;					///< Array of existing combat text

	shared_ptr<Weapon>						m_weapon;							///< Current weapon
	shared_ptr<ArticulatedModel>			m_missDecalModel;					///< Model for the miss decal
	shared_ptr<ArticulatedModel>			m_hitDecalModel;					///< Model for the hit decal
	shared_ptr<VisibleEntity>				m_hitDecal;							///< Pointer to hit decal
	RealTime								m_hitDecalTimeRemainingS = 0.f;		///< Remaining duration to show the decal for
	Array<shared_ptr<VisibleEntity>>		m_currentMissDecals;				///< Pointers to miss decals

	Array<shared_ptr<VisibleEntity>>		m_explosions;						///< Model for target destroyed decal
	Array<RealTime>							m_explosionRemainingTimes;			///< Time for end of explosion
	int										m_explosionIdx = 0;					///< Explosion index
	const int								m_maxExplosions = 20;				///< Maximum number of simultaneous explosions
		
	const int								m_MatTableSize = 10;				///< Set this to set # of color "levels"
	Array<shared_ptr<UniversalMaterial>>	m_materials;						///< This stores the color materials

	GuiDropDownList*						m_sessDropDown;						///< Dropdown menu for session selection
	GuiDropDownList*						m_userDropDown;						///< Dropdown menu for user selection
	GuiLabel*								m_mouseDPILabel;					///< Label for mouse DPI field
	GuiLabel*								m_cm360Label;						///< Label for cm/360 field

	shared_ptr<PlayerControls>				m_playerControls;
	shared_ptr<RenderControls>				m_renderControls;
	shared_ptr<WeaponControls>				m_weaponControls;

	Table<String, Array<shared_ptr<ArticulatedModel>>> m_explosionModels;

	/** Used for visualizing history of frame times. Temporary, awaiting a G3D built-in that does this directly with a texture. */
	Queue<float>							m_frameDurationQueue;				///< Queue for history of frrame times

	/** Used to detect GUI changes to m_reticleIndex */
	int										m_lastReticleLoaded = -1;			///< Last loaded reticle (used for change detection)
	float									m_debugMenuHeight = 0.0f;			///< Height of the debug menu when in developer mode
    GuiPane*								m_currentUserPane;					///< Current user information pane

	// Drop down selection writebacks
	int										m_ddCurrentUser = 0;				///< Index of current user
	int										m_lastSeenUser = -1;				///< Index of last seen user (used for change determination)
	int										m_ddCurrentSession = 0;				///< Index of current session

	RealTime								m_lastJumpTime = 0.0f;				///< Time of last jump

	int										m_lastUniqueID = 0;					///< Counter for creating unique names for various entities
	String									m_loadedScene = "";
	String									m_defaultScene = "FPSci Simple Hallway";	// Default scene to load

	shared_ptr<PythonLogger>				m_pyLogger = nullptr;

	/** When m_displayLagFrames > 0, 3D frames are delayed in this queue */
	Array<shared_ptr<Framebuffer>>			m_ldrDelayBufferQueue;
	int										m_currentDelayBufferIndex = 0;

    shared_ptr<GuiWindow>					m_userSettingsWindow;
    bool									m_userSettingsMode = true;

	Array<Projectile>						m_projectileArray;					///< Arrray of drawn projectiles

	/** Called from onInit */
	void makeGUI();
	void updateControls();
	void loadModels();
	void loadDecals();
	void updateUser(void);
    void updateUserGUI();

	/** Get the current turn scale (per user and scope setting) */
	Vector2 currentTurnScale();
	/** Set the scoped view (and also adjust the turn scale), use setScopeView(!weapon->scoped()) to toggle scope */
	void setScopeView(bool scoped = true);

	void hitTarget(shared_ptr<TargetEntity>);
	void simulateProjectiles(RealTime dt);
	void drawDecal(const Point3& cameraOffset, const Vector3& normal, bool hit = false);

	void drawHUD(RenderDevice *rd);
	void drawClickIndicator(RenderDevice *rd, String mode);

public:
	/* Moving from proctected so that Experiment classes can use it. */
	shared_ptr<GFont>               outputFont;						///< Font used for output
	shared_ptr<GFont>               hudFont;						///< Font used in HUD
	Array<shared_ptr<GFont>>		floatingCombatText;				///< Floating combat text array
	shared_ptr<Texture>             reticleTexture;					///< Texture used for reticle
	shared_ptr<Texture>             hudTexture;						///< Texture used for HUD
	shared_ptr<GuiTheme>			theme;	
	bool                            emergencyTurbo = false;			///< Lower rendering quality to improve performance

	App(const GApp::Settings& settings = GApp::Settings());

	/** Parameter configurations */
	UserTable						userTable;						///< Table of per user information (DPI/cm/360) that doesn't change across experiment
	UserStatusTable					userStatusTable;				///< Table of user status (session ordering/completed sessions) that do change across experiments
	ExperimentConfig                experimentConfig;				///< Configuration for the experiment and its sessions
	KeyMapping						keyMap;
	shared_ptr<WaypointManager>		waypointManager;				///< Waypoint mananger pointer
	
	shared_ptr<SessionConfig>		sessConfig = SessionConfig::create();			///< Current session config
	shared_ptr<G3Dialog>			dialog;							///< Dialog box

	Table<String, Array<shared_ptr<ArticulatedModel>>>	targetModels;
	const int											modelScaleCount = 30;

	shared_ptr<Session> sess;										///< Pointer to the experiment

	bool renderFPS = false;				///< Control flag used to draw (or not draw) FPS information to the display	
	int  displayLagFrames = 0;			///< Count of frames of latency to add
	float lastSetFrameRate = 0.0f;		///< Last set frame rate
	const int numReticles = 55;			///< Total count of reticles available to choose from
	float sceneBrightness = 1.0f;		///< Scene brightness scale factor

	/** Call to change the reticle. */
	void setReticle(int r);

	/** Show the player controls */
	void showPlayerControls();
	/** Show the render controls */
	void showRenderControls();
	/** Show the weapon controls */
	void showWeaponControls();
	/** Save scene w/ updated player position */
	void exportScene();

	float debugMenuHeight() {
		return m_debugMenuHeight;
	}

    /** callback for saving user config */
	void userSaveButtonPress(void);

	Array<String> updateSessionDropDown(void);
	String getDropDownSessId(void);
	void markSessComplete(String id);
	void updateSessionPress(void);
	void updateSession(const String& id);
	void updateParameters(int frameDelay, float frameRate);
	void presentQuestion(Question question);

	String getDropDownUserId(void);
	shared_ptr<UserConfig> getCurrUser(void);

    void quitRequest();
	   
	/** opens the user settings window */
    void openUserSettingsWindow();

	/** reads current user settings to update sensitivity in the controller */
    void updateMouseSensitivity();
	
	virtual void onPostProcessHDR3DEffects(RenderDevice *rd) override;
	virtual void onInit() override;
	virtual void onAI() override;
	virtual void onNetwork() override;
	virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
	virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
	virtual void onAfterLoadScene(const Any& any, const String& sceneName) override;
	virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;
	virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface) override;
	virtual bool onEvent(const GEvent& e) override;
	virtual void onUserInput(UserInput* ui) override;
	virtual void onCleanup() override;
    virtual void oneFrame() override;
	
	// hardware setting
	struct ScreenSetting
	{
		float viewingDistance = 0.5f; // in m
		float screenDiagonal = 25.0f * 0.0254f; // in m (diagonal)
		Vector2 resolution = Vector2(1920, 1080);
		float pixelSize = screenDiagonal / sqrt(resolution.x * resolution.x + resolution.y * resolution.y);
		Vector2 screenSize = resolution * pixelSize;
	} m_screenSetting;

	bool							m_buttonUp = true;
	bool							m_frameToggle = false;		///< Simple toggle flag used for frame rate click-to-photon monitoring
};

// The "old" way of animation
/** Parameters related to animation during a trial. */
// Animation flow.
// updateAnimation() is called at the beginning of onGraphics3D. Workflow in updateLocation()
//       1. Check if trial time passed m_trialParam.trialDuration.
//                 If yes, end the current trial and initialize the next trial.
//       2. Check if motionChange is required.
//                 If yes, update m_rotationAxis. This is chosen among the vectors orthogonal to [camera - target] line.
//       3. update target location.
//       4. append to m_TargetArray an object with m_targetLocation.
