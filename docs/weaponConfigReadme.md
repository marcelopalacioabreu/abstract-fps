# Introduction
The weapon config allows for a modular configuration of the weapon used during an [experiment](experimentConfigReadme.md). Weapon configurations can either be inlined into the [`experimentconfig.Any`](../data-files/SAMPLEexperimentconfig.Any#L16) or included as independent `.Any` file using the `#include("file.Any")` directive in the experiment configuration file to refer to a `[filename].weapon.Any` file (or any other `.Any` file including a weapon). If you plan to regularly change weapons we suggest using the later approach.

In the future support for a weapon editor might be offered as part of the application.

## File Location(s)
The [`data-files/weapon` directory](../data-files/weapon) contains some default weapon configurations as stand-alone `.Any` files. These file can be included using the supported `.Any` `#include` construct as shown below:

```
"weapon" : #include("[weapon file name].Any");
```

## Interaction with Experiment/Session Level Specification
The "parameter inheritance" structure used for experiment/session level parameters does not carry into the weapon configuration. That is to say, weapon configurations must be _completely_ specified wherever they are provided. For this reason we suggest encapsulating weapons in their own `*.weapon.Any` files and including them using a `#include()` directive.

As an example of this concept, you **cannot** (currently) specify a weapon at the experiment level, then change just one field within this weapon configuration on a per-session basis. Instead you need to _redefine_ this weapon for each session. Using `#include()` directives for weapons avoids this issue since the full weapon configuration is always specified.

# Weapon Config Field Descriptions

This file provides information about the weapon to be used in the experiment. Detailed field descriptions are provided below. Each example configuration provides the default values for it's fields.

| Parameter Name        |Units      | Description                                                                                           |
|-----------------------|-----------|-------------------------------------------------------------------------------------------------------|
|`maxAmmo`              |shots      | The maximum number of clicks a user can make in any given trial                                       |
|`firePeriod`           |s/shot     | The minimum fire period allowed in the session (set this to 0 if you want "laser" mode)               |
|`autoFire`             |`bool`     | Whether or not the weapon fires when the left mouse is held, or requires release between fire         |
|`damagePerSecond`      |damage/s   | The damage done by the weapon per second, when `firePeriod` > 0 the damage per round is set by `damagePerSecond`*`firePeriod`, when `firePeriod` is 0 and `autoFire` is `True` damage is computed based on time hitting the target.        |
|`hitScan`              |`bool`     | Whether or not the weapon acts as an instantaneous hitscan (true) vs propagated projectile (false)    |

```
    "maxAmmo" : 10000;              // Large ammo count
    "firePeriod" : 0.5;             // 2 shots per second fire period
    "autoFire": false;              // Single fire (no hold to fire)
    "damagePerSecond": 2.0;         // 1 damage per shot (single shot to destroy)
    "hitScan" : true;               // Use hitscan (not propogated projectile) for hit detection
```

## Sound and View Model
Controls specific to the sound/view model for the weapon are provided below:

| Parameter Name        |Units      | Description                                                                                           |
|-----------------------|-----------|-------------------------------------------------------------------------------------------------------|
|`fireSound`            |file       | The filename/location of the audio clip to use for the weapon firing                                  |
|`fireSoundVol`         |ratio      | The volume to play the `fireSound` clip with                                                          |
|`renderModel`          |`bool`     | Whether or not a weapon model is rendered in the first-person view                                    |
|`modelSpec`            |`ArticulatedModel::Specification` | Any-based specification for the weapon being used                              |

```
    "fireSound" : "sound/42108__marcuslee__Laser_Wrath_6.wav"           // This comes w/ G3D
    "fireSoundVol" : 0.5;       // Play the fire sound at 1/2 volume
    "renderModel" : false;      // Don't render a weapon model
    "modelSpec" : [];           // No default model spec provided (see the example config below for more info)
```

## Projectiles 
Controls specific to the projectiles fired by the weapon are included below:

| Parameter Name        |Units      | Description                                                                                           |
|-----------------------|-----------|-------------------------------------------------------------------------------------------------------|
|`renderBullets`        |`bool`     | Whether or not bullets are rendered from the barrel of the weapon when fired                          |
|`bulletSpeed`          |m/s        | The speed of rendered bullets                                                                         |
|`bulletGravity`        |m/s²       | The gravity to apply to bullets (0 for no droop)                                                      |
|`bulletScale`          |`Vector3(`m`)`| This sets the scaling to apply to the bullet (effectively controls bullet size in meters)        |
|`bulletColor`          |`Color3`   | The (emissive) color/power to apply to the bullet. RGB fields can be > 1                              |
|`bulletOffset`         |`Vector3(`m`)`| The offset from the camera view to where to spawn a bullet                                       |

```
    "renderBullets" : false;                        // Don't draw bullets
    "bulletSpeed" : 100.0;                          // 100m/s bullet speed (not used)
    "bulletGravity" : 0.0;                          // No bullet gravity
    "bulletScale" : Vector3(0.05, 0.05, 2.0);       // 5cm x 5cm x 2m bullet shape
    "bulletColor": Color3(5, 4, 0);                 // Emissive yellow-ish projectile
    "bulletOffset" : Vector3(0, 0, 0);              // No bullet offset from the camera
```

## Decal Control
Controls specific to the miss decals drawn in the scene are included below:

| Parameter Name        |Units      | Description                                                                                           |
|-----------------------|-----------|-------------------------------------------------------------------------------------------------------|
|`renderDecals`         |`bool`     | Whether or not bullet hole decals are put on misses when `autoFire` is `False` and `firePeriod` > 0 (i.e. not in laser mode) |
|`missDecal`            |`String`   | The filename of an image to use for miss decals. Can be set to `""` for no decals.
|`missDecalCount`       |`int`      | The maximum number of miss decals to draw from this weapon (oldest are removed first). Can be set to `0` for no decals.|
|`missDecalScale`       |`float`    | A scale to apply to the miss decals drawn by this weapon, `1.0` means do not scale                     |
|`hitDecal`             |`String`   | The filename of an image to use for hit decals. Can be set to `""` for no decals.                      |
|`hitDecalScale`        |`float`    | A scale to apply to the hit decals drawn by this weapon. `1.0` means do not scale.                     |
|`hitDecalDuration`     |s          | The duration to draw a hit decal for (in seconds).                                                     |
|`hitDecalColorMult`    |`float`    | The value used to multiply colors for the hit decal (higher means brighter). Set >1 for "emissive".    |

```
    "renderDecals" : true,                          // Draw decals on hit/miss
    "missDecal" : "bullet-decal-256x256.png";       // Included in FPSci
    "missDecalCount" : 2;                           // Number of miss decals to draw (at once)
    "missDecalScale" : 1.0;                         // Don't scale the miss decal (1.0x scale)
    "hitDecalScale" : 1.0;                          // Don't scale the hit decal  (1.0x scale)
    "hitDecalDurationS" : 0.1;                      // Draw the decal for 0.1s
    "hitDecalColorMult" = 2.0;                      // Slightly emissive hit decal
```

<!-- ## Muzzle Flash Control
Controls specific to muzzle flash are provided below:

| Parameter Name        |Units      | Description                                                                                           |
|-----------------------|-----------|-------------------------------------------------------------------------------------------------------|
|`renderMuzzleFlash`    |bool       | Whether or not a muzzle flash is rendered for the weapon                                              |
|`muzzleOffset`         |`Vector3`(m)| The offset of the muzzle within the weapon frame                                                     |

```
    "renderMuzzleFlash" : false;                // Don't render a muzzle flash
    "muzzleOffset" : Vector3(0,0,0);            // No muzzle offset
``` -->

## Scope Control
Controls specific to scope behavior are provided below:

| Parameter Name        |Units      | Description                                                                                           |
|-----------------------|-----------|-------------------------------------------------------------------------------------------------------|
|`scopeFoV`             |float      | (Horizontal) field of view for the camera when using a scope (set to `0` for no scope)                |
|`scopeToggle`          |bool       | Whether or not the scope is active when the keymapped button is held (false) or toggled using this button (true)|

```
    "scopeFoV" : 0.0;                           // Don't change FoV w/ scope
    "scopeToggle" : false;                      // Don't use scope "toggling" mode (requiresl hold)
```

# Example Config
The config below provides an example for each of the fields above (along with their default values):

```
"maxAmmo": 10000,           // Maximum ammo for the weapon
"firePeriod": 0.5,          // Fire twice a second max
"autoFire": false,          // Automatic firing if mouse is held
"damagePerSecond": 2.0,     // 2 damage/s * 0.5 s/shot = 1 damage/shot
"hitScan": false,

"fireSound": "sound/42108__marcuslee__Laser_Wrath_6.wav",       // The sound to fire
"fireSoundVol": 0.5f,
"renderModel": true,        // Default is false,
"modelSpec": ArticulatedModel::Specification{			        // Default model
	filename = "model/sniper/sniper.obj";
	preprocess = {
		transformGeometry(all(), Matrix4::yawDegrees(90));
		transformGeometry(all(), Matrix4::scale(1.2,1,0.4));
	};
	scale = 0.25;
},

"renderBullets": true,      // Default is false
"bulletSpeed": 100.0,       // Bullets move at 100m/s
"bulletGravity": 10.0,      // Bullets accelerate down at 10m/s²

"renderDecals": true,       // Draw decals on walls for misses/target for hit
"missDecal": "bullet-decal-256x256.png",           // Default miss decal
"missDecalCount": 2,        // Draw 2 decals (at most) at a time
"missDecalScale": 1.0,      // Don't scale the decal
"hitDecal" : "",            // No default hit decal
"hitDecalScale": 1.0,       // Scale to apply to hit decal
"hitDecalDuration": 0.1,    // Show the hit decal for 0.1s
"hitDecalColorMult": 2.0,   // Multiply the hit decal color values by 2 (pseudo-emissive)

"renderMuzzleFlash": false, // Draw a muzzle flash
"muzzleOffset": Vector3(0.0,-0.8,0.0)        // Add an offset here to correct the barrel location (default is [0,0,0])

"scopeFoV": 0.0,            // No scope
"scopeToggle": false,       // Scope in "on-demand" mode (not toggled)
```
 
# Weapon Modes and Damage
There were several common "modes" that motivated the weapon configuration information provided in the experimentconfig. These are outlined below:

|  | autoFire = True | autoFire = False |
|-----------------|------------------------|--------------------------|
| **firePeriod = 0** | "laser" mode | unlimited rate semi-auto |
| **firePeriod > 0** | limited rate full auto | limited rate semi-auto |

When using a non "laser" mode weapon, the damage done (per fired round) is computed as `damagePerSecond` * `firePeriod` where damage/s * s/fire = damage/fire.

When using a "laser" mode weapon the `damagePerSecond` is applied iteratively (the weapon does no damage instantaneously). Instead for each frame the laser stays over the target the health is reduced based on the time since the last frame.

# ArticulatedModel Specification
If you want to render a 3D model as the weapon from the FPS view you will need to provide a .Any serializable `ArticulatedModel` specification as the `modelSpec` field. The example below gives some info on how to do this.

    "modelSpec": ArticulatedModel::Specification{
        filename = "model/sniper/sniper.obj";
        preprocess = {
            transformGeometry(all(), Matrix4::yawDegrees(90));>
            transformGeometry(all(), Matrix4::scale(1.2,1,0.4));
            transformGeometry(all(), Matrix4::translation(0.5, 0, 0));
        };
        scale = 0.25;
    }

The `modelSpec` above provides a filename for a `.obj` file for the model, as well as several preprocess steps intended to orient, position, and scale the model correctly.

If the `renderModel` flag within a weapon configuration is `true` a `modelSpec` must be provided in the configuration. If a `modelSpec` is not provided an exception will be thrown at runtime stating this.