  1	virtual void			SetRefEHandle( const CBaseHandle &handle );
  2	virtual const			CBaseHandle& GetRefEHandle() const;
  3	virtual ICollideable	*GetCollideable();
  4	virtual IServerNetworkable *GetNetworkable();
  5	virtual CBaseEntity		*GetBaseEntity();
  6	virtual int				GetModelIndex( void ) const;
  7	// something
  8	virtual void			SetModelIndex( int index );
	//virtual string_t		GetModelName( void ) const;
	//virtual void			SetModelIndexOverride( int index, int nValue );
	//virtual bool			TestCollision( const Ray_t& ray, unsigned int mask, trace_t& trace );
	//virtual	bool			TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	//virtual void			ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs );
	//virtual	bool			ShouldCollide( int collisionGroup, int contentsMask ) const;
	//virtual void			SetOwnerEntity( CBaseEntity* pOwner );
	//virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo );
	//virtual int				UpdateTransmitState();
	//virtual void			SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways );
	//virtual const char	*GetTracerType( void );
	
	
	
	
	
 25	virtual void Spawn( int ? );
 
 27	virtual void Precache( void ) {}
 28	virtual void SetModel( const char *szModelName );
	//virtual CStudioHdr *OnNewModel();
	//virtual void PostConstructor( const char *szClassname );
	//virtual void PostClientActive( void );
	//virtual void ParseMapData( CEntityMapData *mapData );
	//virtual bool KeyValue( const char *szKeyName, const char *szValue );
	//virtual bool KeyValue( const char *szKeyName, float flValue );
	//virtual bool KeyValue( const char *szKeyName, const Vector &vecValue );
	//virtual bool GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen );
	//virtual void Activate( void );
	
	
 40	virtual void	SetParent( CBaseEntity* pNewParent, int iAttachment = -1 );
	// virtual void* SomethingAboutEffects(int a2)
	virtual int	ObjectCaps( void );
 43	virtual bool AcceptInput( const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID );
	virtual void GetInputDispatchEffectPosition( const char *sInputString, Vector &pOrigin, QAngle &pAngles );
	virtual	void DrawDebugGeometryOverlays(void);					
	virtual int  DrawDebugTextOverlays(void);
	virtual int	Save( ISave &save );
	virtual int	Restore( IRestore &restore );
	virtual bool ShouldSavePhysics();
	virtual void OnSave( IEntitySaveUtils *pSaveUtils );
	virtual void OnRestore();
	virtual int RequiredEdictIndex( void ) { return -1; } 
	virtual void MoveDone( void ) { if (m_pfnMoveDone) (this->*m_pfnMoveDone)();};
 54	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)();};
	// void something
	// void something
	// void something
	// void something
	// CBaseEntity *GetSomething(?);
	// CBaseEntity *GetSomething(?);
 61	virtual CBaseAnimating*	GetBaseAnimating() { return 0; }
 62	virtual IResponseSystem *GetResponseSystem();
 63	virtual void	DispatchResponse( const char *conceptName );
 64	virtual Class_T Classify ( void );
 65	virtual void	DeathNotice ( CBaseEntity *pVictim ) {}// NPC maker children use this to tell the NPC maker that they have died.
 66	virtual bool	ShouldAttractAutoAim( CBaseEntity *pAimingEnt ) { return ((GetFlags() & FL_AIMTARGET) != 0); }
 67	virtual float	GetAutoAimRadius();
 68	virtual Vector	GetAutoAimCenter() { return WorldSpaceCenter(); }
 69	virtual ITraceFilter*	GetBeamTraceFilter( void );
 70	virtual bool	PassesDamageFilter( const CTakeDamageInfo &info );
 71	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator = NULL );
 72	virtual bool	CanBeHitByMeleeAttack( CBaseEntity *pAttacker ) { return true; }
 73	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
 74	virtual int		TakeHealth( float flHealth, int bitsDamageType );
 75	virtual bool	IsAlive( void );
 76	virtual void	Event_Killed( const CTakeDamageInfo &info );
 77	virtual void	Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info ) { return; }
 78	virtual int				BloodColor( void );
 79	virtual bool			IsTriggered( CBaseEntity *pActivator ) {return true;}
//	FUNCTION CHANGED, NOW RETURNS A POINTER: virtual bool			IsNPC( void ) const { return false; }
 81	virtual CBaseCombatCharacter *MyCombatCharacterPointer( void ) { return NULL; }
 82	virtual INextBot		*MyNextBotPointer( void ) { return NULL; }
 83	virtual float			GetDelay( void ) { return 0; }
 84	virtual bool			IsMoving( void );
 85	virtual char const		*DamageDecal( int bitsDamageType, int gameMaterial );
 86	virtual void			DecalTrace( trace_t *pTrace, char const *decalName );
 87	virtual void			ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName = NULL );
	virtual bool	OnControls( CBaseEntity *pControls ) { return false; }
 89	virtual bool	HasTarget( string_t targetname );
	virtual	bool	IsPlayer( void ) const { return false; }
	virtual bool	IsNetClient( void ) const { return false; } // One of those got removed
	virtual bool	IsTemplate( void ) { return false; }
	virtual bool	IsBaseObject( void ) const { return false; }
	virtual bool	IsBaseTrain( void ) const { return false; }
	virtual bool	IsBaseCombatWeapon( void ) const { return false; }
	virtual CBaseCombatWeapon *MyCombatWeaponPointer( void ) { return NULL; }
	virtual IServerVehicle*			GetServerVehicle() { return NULL; }
	virtual bool	IsViewable( void );					// is this something that would be looked at (model, sprite, etc.)?
 99	virtual void	ChangeTeam( int iTeamNum );			// Assign this entity to a team.
	virtual void OnEntityEvent( EntityEvent_t event, void *pEventData );
	virtual bool	CanStandOn( CBaseEntity *pSurface ) const { return (pSurface && !pSurface->IsStandable()) ? false : true; }
	virtual bool	CanStandOn( edict_t	*ent ) const { return CanStandOn( GetContainingEntity( ent ) ); }
	virtual CBaseEntity		*GetEnemy( void ) { return NULL; }
	virtual CBaseEntity		*GetEnemy( void ) const { return NULL; }
	virtual void			Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void			StartTouch( CBaseEntity *pOther );
	virtual void			Touch( CBaseEntity *pOther ); 
	virtual void			EndTouch( CBaseEntity *pOther );
	virtual void			StartBlocked( CBaseEntity *pOther ) {}
	virtual void			Blocked( CBaseEntity *pOther );
	virtual void			EndBlocked( void ) {}
	virtual void			PhysicsSimulate( void );
	virtual void			UpdateOnRemove( void );
114	virtual void			StopLoopingSounds( void ) {}
	virtual	bool			SUB_AllowedToFade( void );
116	virtual void			Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity );
117	virtual void			NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );
	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );
	virtual int	GetTracerAttachment( void );
	virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual void DoImpactEffect( trace_t &tr, int nDamageType ); // give shooter a chance to do a custom impact.
122	virtual CBaseEntity *Respawn( void ) { return NULL; }
123	virtual bool IsLockedByMaster( void ) { return false; }
124	virtual int		GetMaxHealth()  const	{ return m_iMaxHealth; }

// I haven't looked into the functions below, none of them seem useful so I won't

	virtual void	ModifyOrAppendCriteria( AI_CriteriaSet& set );
	virtual int		GetDamageType() const;
	virtual float	GetDamage() { return 0; }
	virtual void	SetDamage(float flDamage) {}
	virtual Vector	EyePosition( void );			// position of eyes
	virtual const QAngle &EyeAngles( void );		// Direction of eyes in world space
	virtual const QAngle &LocalEyeAngles( void );	// Direction of eyes
	virtual Vector	EarPosition( void );			// position of ears
	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true);		// position to shoot at
	virtual Vector	HeadTarget( const Vector &posSrc );
	virtual void	GetVectors(Vector* forward, Vector* right, Vector* up) const;
	virtual const Vector &GetViewOffset() const;
	virtual void SetViewOffset( const Vector &v );
	virtual Vector	GetSmoothedVelocity( void );
	virtual void	GetVelocity(Vector *vVelocity, AngularImpulse *vAngVelocity = NULL);
	virtual	bool FVisible ( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );
	virtual bool FVisible( const Vector &vecTarget, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );
	virtual bool CanBeSeenBy( CAI_BaseNPC *pNPC ) { return true; } // allows entities to be 'invisible' to NPC senses.
	virtual float			GetAttackDamageScale( CBaseEntity *pVictim );
	virtual float			GetReceivedDamageScale( CBaseEntity *pAttacker );
	virtual void			GetGroundVelocityToApply( Vector &vecGroundVel ) { vecGroundVel = vec3_origin; }
	virtual bool			PhysicsSplash( const Vector &centerPoint, const Vector &normal, float rawSpeed, float scaledSpeed ) { return false; }
	virtual void			Splash() {}
	virtual const Vector&	WorldSpaceCenter( ) const;
	virtual Vector			GetSoundEmissionOrigin() const;
	virtual void ModifyEmitSoundParams( EmitSound_t &params );
	virtual bool IsDeflectable() { return false; }
	virtual void Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir ) {}
	virtual bool	CreateVPhysics();
	virtual bool	ForceVPhysicsCollide( CBaseEntity *pEntity ) { return false; }
	virtual void	VPhysicsDestroyObject( void );
	virtual void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	virtual int		VPhysicsTakeDamage( const CTakeDamageInfo &info );
	virtual void	VPhysicsShadowCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void	VPhysicsShadowUpdate( IPhysicsObject *pPhysics ) {}
	virtual void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void	VPhysicsFriction( IPhysicsObject *pObject, float energy, int surfaceProps, int surfacePropsHit );
	virtual void	UpdatePhysicsShadowToCurrentPosition( float deltaTime );
	virtual int		VPhysicsGetObjectList( IPhysicsObject **pList, int listMax );
	virtual bool	VPhysicsIsFlesh( void );
	virtual	CBasePlayer		*HasPhysicsAttacker( float dt ) { return NULL; }
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;
	virtual void			ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );
	virtual void			PerformCustomPhysics( Vector *pNewPosition, Vector *pNewVelocity, QAngle *pNewAngles, QAngle *pNewAngVelocity );
	virtual	Vector			GetStepOrigin( void ) const;
	virtual	QAngle			GetStepAngles( void ) const;
	virtual bool ShouldDrawWaterImpacts() { return true; }