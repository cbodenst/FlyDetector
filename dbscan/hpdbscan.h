#ifndef HPDBSCAN_H
#define	HPDBSCAN_H

#include "points.h"
#include "rules.h"
#include "space.h"

#include <stddef.h>
#include <string>

class HPDBSCAN
{    
protected:
    Pointz      m_points;
    
    /**
     * Internal Operation\
     */
    void applyRules(const Rules& rules);
    void distributeRules(Rules& rules);
    Rules localDBSCAN(const Space &space, float epsilon, size_t minPoints);
    
public:
    HPDBSCAN(Coord* points, int npoints, int dimension);
    void  scan(float epsilon, size_t minPoints, Cluster* results);

     inline size_t size() const
    {
        return this->m_points.size();
    }

    inline size_t dimensions() const
    {
        return this->m_points.dimensions();
    }
};

#endif	// HPDBSCAN_H
