// class definition
class Bear {
public:
    // constructor functions
    Bear(void);
    Bear(double mass, double speed, double height, R3Vector velocity, R3Point position);

    // bear property functions/operators
    const double getMass(void) const;
    const double getSpeed(void) const;
    const double getHeight(void) const;
    const R3Vector& getVelocity(void) const;
    const R3Point& getPosition(void) const;

    // manipulation functions/operators
    virtual void setVelocity(const R3Vector& newVelocity);
    virtual void setPosition(const R3Point& newPosition);
    virtual void updatePosition(double delta_time);

    // public variables
    double mass;
    double speed;
    double height;
    R3Vector velocity;
    R3Point position;
    double health;
    double stamina;
    R3Box bbox;
};

// Inline functions
inline const double Bear::
getMass(void) const
{
    // return bear mass
    return mass;
}

inline const double Bear::
getSpeed(void) const
{
    // return bear speed
    return speed;
}

inline const double Bear::
getHeight(void) const
{
    // return bear height
    return height;
}

inline const R3Vector& Bear::
getVelocity(void) const
{
    // return bear velocity
    return velocity;
}

inline const R3Point& Bear::
getPosition(void) const
{
    // return bear position
    return position;
}
