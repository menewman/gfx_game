// class definition
class Prey {
public:
    // constructor functions
    Prey(void);
    Prey(double mass, double speed, R3Point position, R3Vector velocity, R3Shape shape);

    // property functions/operators
    const double getMass(void) const;
    const double getSpeed(void) const;
    const R3Point& getPosition(void) const;
    const R3Vector& getVelocity(void) const;

    // manipulation functions/operators
    virtual void setPosition(const R3Point& newPosition);
    virtual void setVelocity(const R3Vector& newVelocity);
    virtual void updatePosition(double delta_time, R3Point playerPos, double bound);
    
    // public variables
    double mass;
    double speed;
    R3Point position;
    R3Vector velocity;
    R3Shape shape;
};

// Inline functions
inline const double Prey::
getMass(void) const
{
    // return prey mass
    return mass;
}

inline const double Prey::
getSpeed(void) const
{
    // return prey movement speed multiplier
    return speed;
}

inline const R3Point& Prey::
getPosition(void) const
{
    // return prey position
    return position;
}

inline const R3Vector& Prey::
getVelocity(void) const
{
    // return prety velocity
    return velocity;
}