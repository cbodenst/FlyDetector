#ifndef POINTS_H
#define	POINTS_H

#include "constants.h"
#include "rules.h"
#include "util.h"

#include <cmath>
#include <stddef.h>
#include <string>
#include <vector>

#define REORDER true
#define DATASET "DBSCAN"

class Pointz
{
    
    Cell*    m_cells;
    Cluster* m_clusters;
    Coord*   m_points;
    size_t*  m_initialOrder;
    
    size_t   m_dimensions;
    size_t   m_size;
    size_t   m_totalSize;
    
    
    /**
     * Internal Operations
     */
    void readFile(const std::string& filename);

public:
    /**
     * Constructor
     */
    Pointz(Coord* points, int npoins, int dimensions);
    
    /**
     * Access
     */
    inline size_t cell(size_t index) const
    {
        return this->m_cells[index];
    }
    
    inline ssize_t cluster(const size_t index) const
    {
        return std::abs(this->m_clusters[index]);
    }

	inline Cluster* cluster() const
    {
        return this->m_clusters;
    }
    
    inline bool corePoint(const size_t index) const
    {
        return this->m_clusters[index] < 0;
    }
    
    inline size_t dimensions() const
    {
        return this->m_dimensions;
    }
    
    inline Coord* operator[](size_t index) const
    {
        return this->m_points + index * this->m_dimensions;
    }
    
    inline size_t size() const
    {
        return this->m_size;
    }
    inline size_t totalSize() const
    {
        return this->m_totalSize;
    }
    
    /**
     * Modifiers
     */
    inline void cell(size_t index, size_t number)
    {
        this->m_cells[index] = number;
    }
    
    inline void cluster(const size_t index, ssize_t value, bool core)
    {
        atomicMin(&this->m_clusters[index], core ? -value : value);
    }
    
    inline void overrideCluster(const size_t index, ssize_t value, bool core)
    {
        this->m_clusters[index] = core ? -value : value;
    }
    
    /**
     * Operations
     */

    void   resetClusters(Cluster* clusters);
    void   sortByCell(const CellIndex& index);
    void   sortByOrder(size_t maxDigits, size_t lowerBound, size_t upperBound);
    void   writeClusterToFile(const std::string& filename) const;
    
    /**
     * Destructor
     */
    ~Pointz();
};

#endif	// POINTS_H
