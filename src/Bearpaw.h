// class definition
class Prey;
class Hunter;

class Bearpaw {
public:
    // constructor functions
    Bearpaw(void);
    Bearpaw(R3Point position, R3Point playeroffset, R3Vector velocity, R3Shape shape); //double mass, double speed, 

    // property functions/operators
    const double getMass(void) const;
    const double getSpeed(void) const;
    const R3Point& getPosition(void) const;
    const R3Vector& getVelocity(void) const;

    // manipulation functions/operators
    virtual void setVelocity(const R3Vector& newVelocity);
    virtual void updatePosition(double delta_time, R3Point playerPos); //, double bound, R3Scene *scene, vector<Bearpaw>& Bearpaw_list, vector<Hunter>& hunter_list, R3Box bearBBox);
    
    // other functions
    bool collides(R3Scene *scene, R3Node *node);
    //bool collides(R3Scene *scene, vector<Bearpaw>& Bearpaw_list, vector<Hunter>& hunter_list, R3Box bearBBox);
    void draw();
    
    // public variables
    double mass;
    double speed;
    R3Point playeroffset;
    R3Point position;
    R3Vector velocity;
    R3Shape shape;
    R3Box bbox;
	R3Material Bearpawmaterial;
};

// Inline functions
inline const double Bearpaw::
getMass(void) const
{
    // return Bearpaw mass
    return mass;
}

inline const double Bearpaw::
getSpeed(void) const
{
    // return Bearpaw movement speed multiplier
    return speed;
}

inline const R3Point& Bearpaw::
getPosition(void) const
{
    // return Bearpaw position
    return playeroffset;
}

inline const R3Vector& Bearpaw::
getVelocity(void) const
{
    // return Bearpaw velocity
    return velocity;
}
