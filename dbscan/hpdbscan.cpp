#include "hpdbscan.h"

#include <algorithm>
#include <vector>
#include <unordered_set>
#include <set>
#include <iterator>
#include <omp.h>

/**
 * Constructors
 */
HPDBSCAN::HPDBSCAN(Coord* points, int npoints, int dimensions) :
    m_points(points, npoints, dimensions)
{
}

/**
 * Internal Operations
 */
void HPDBSCAN::applyRules(const Rules& rules)
{
    #pragma omp parallel for
    for (size_t i = 0; i < this->m_points.size(); ++i)
    {
        const bool core    = this->m_points.corePoint(i);
        ssize_t    cluster = this->m_points.cluster(i);
        ssize_t    found   = rules.rule(cluster);
        
        while (found < NOISE)
        {
            cluster = found;
            found   = rules.rule(found);
        }
        this->m_points.overrideCluster(i, cluster, core);
    }
}

Rules HPDBSCAN::localDBSCAN(const Space& space, const float epsilon, const size_t minPoints)
{
    const float      EPS2    = std::pow(epsilon, 2);
    
    const size_t lower = 0;
    const size_t upper = this->m_points.size();
    
    Rules rules;
    // local dbscan
    
    size_t cell = NOT_VISITED;
    std::vector<size_t> neighborPoints;
    
    
    #pragma omp parallel for schedule(dynamic, 500) private(neighborPoints) firstprivate(cell) reduction(merge: rules)
    for (size_t point = lower; point < upper; ++point)
    {
        size_t pointCell = this->m_points.cell(point);
        if (pointCell != cell)
        {
            neighborPoints = space.getNeighbors(pointCell);
            cell = pointCell;
        }
        std::vector<size_t> minPointsArea;
        ssize_t clusterId = NOISE;
        if(neighborPoints.size() >= minPoints)
        {
            clusterId =space.regionQuery(point, neighborPoints, EPS2, minPointsArea);
        }

        if (minPointsArea.size() >= minPoints)
        {
            this->m_points.cluster(point, clusterId, true);

            for (size_t other : minPointsArea)
            {
                ssize_t otherClusterId = this->m_points.cluster(other);
                if (this->m_points.corePoint(other))
                {
                    const std::pair<Cluster, Cluster> minmax = std::minmax(otherClusterId, clusterId);
                    rules.update(minmax.second, minmax.first);
                }
                this->m_points.cluster(other, clusterId, false);
            }
        }
        else if (this->m_points.cluster(point) == NOT_VISITED)
        {
            this->m_points.cluster(point, NOISE, false);
        }
    }
    
    
    return rules;
}

/**
 * Operations
 */
void HPDBSCAN::scan(float epsilon, size_t minPoints, Cluster* results)
{
    if(m_points.size() == 0)
    {
        results = NULL;
        return;
    }
    this->m_points.resetClusters(results);
    Space space(this->m_points, epsilon);    
    Rules rules = this->localDBSCAN(space, epsilon, minPoints);   
    this->applyRules(rules);   
    this->m_points.sortByOrder(ceil(log10(this->m_points.size())), 0, this->m_points.size());
}

/**
 * Output
 */

