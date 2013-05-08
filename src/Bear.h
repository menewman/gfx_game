// class definition
class Bear {
public:
    // constructor functions
    Bear(void);
    Bear(double mass, double speed, double height, R3Vector velocity);

    // bear property functions/operators
    const double getMass(void) const;
    const double getSpeed(void) const;
    const double getHeight(void) const;
    const R3Vector& getVelocity(void) const;

    // manipulation functions/operators
    virtual void setVelocity(const R3Vector& newVelocity);

private:
    double mass;
    double speed;
    double height;
    R3Vector velocity;
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
