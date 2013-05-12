//class definition
class Hunter {
public:
    // constructor functions
    Hunter(void);
    Hunter(double mass, double speed, R3Point position, R3Vector velocity, R3ParticleSource source);

    // property functions/operators
    const double getMass(void) const;
    const double getSpeed(void) const;
    const R3Point& getPosition(void) const;
    const R3Vector& getVelocity(void) const;

    // manipulation functions/operators
    virtual void setVelocity(const R3Vector& newVelocity);
    virtual void updatePosition(double delta_time, R3Point playerPos, double bound);

    // other functions
    void shoot(R3Scene *scene, double current_time, double delta_time, R3Point playerPos);

    // public variables
    double mass;
    double speed;
    R3Point position;
    R3Vector velocity;
    R3ParticleSource source;
    R3Shape icon;
    R3Box bbox;
};

// Inline functions
inline const double Hunter::
getMass(void) const
{
    // return hunter mass
    return mass;
}

inline const double Hunter::
getSpeed(void) const
{
    // return hunter movement speed multiplier
    return speed;
}

inline const R3Point& Hunter::
getPosition(void) const
{
    // return hunter position
    return position;
}

inline const R3Vector& Hunter::
getVelocity(void) const
{
    // return hunter velocity
    return velocity;
}
