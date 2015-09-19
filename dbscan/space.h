#ifndef SPACE_H
#define	SPACE_H

#include "constants.h"
#include "points.h"

#include <stddef.h>
#include <map>
#include <vector>

class Space {
    Pointz&             m_points;
    
    CellIndex           m_cellIndex;
    
    size_t              m_total;
    size_t              m_lastCell;
    
    std::vector<size_t> m_cells;
    std::vector<Coord>  m_maximum; 
    std::vector<Coord>  m_minimum;    
    std::vector<size_t> m_swapDims;
    
    
    /**
     * Initialization
     */
    CellCounter computeCells(float epsilon);
    
    void computeDimensions(float epsilon);
    void computeIndex(CellCounter& counter);
    void swapDimensions();
    
public:
    Space(Pointz& points, float epsilon);
    
    /**
     * Access 
     */    
    inline const CellIndex& cellIndex() const
    {
        return this->m_cellIndex;
    }
    
    inline const std::vector<size_t>& cells() const
    {
        return this->m_cells;
    }
    
    inline size_t cells(size_t dimension) const
    {
        return this->m_cells[dimension];
    }
    
    inline const std::vector<Coord>& max() const
    {
        return this->m_maximum;
    }
    
    inline Coord max(size_t dimension) const
    { 
        return this->m_maximum[dimension];
    
    }
    
    inline const std::vector<Coord>& min() const
    {
        return this->m_minimum;
    }
    
    inline Coord min(size_t dimension) const
    {
        return this->m_minimum[dimension];
    }
    
    inline size_t total() const
    {
        return this->m_total;
    }
    
    /**
     * Operations
     */
    std::vector<size_t> getNeighbors(const size_t cellId) const;
    size_t regionQuery(const size_t pointIndex, const std::vector<size_t>& neighborPoints, const float EPS2, std::vector<size_t>& minPointsArea) const;
};

#endif	// SPACE_H
