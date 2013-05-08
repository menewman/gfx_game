// class definition
class Bear {
public:
    // constructor functions
    Bear(void);
    Bear(double mass, double speed, double height);

    // bear property functions/operators
    const double getMass(void) const;
    const double getSpeed(void) const;
    const double getHeight(void) const;

    // manipulation functions/operators

private:
    double mass;
    double speed;
    double height;
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
