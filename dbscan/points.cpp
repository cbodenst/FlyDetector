#include "points.h"
#include "rules.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <unordered_map>

/**
 * Radix-Sort helpers
 */
const unsigned short DIGITS   = 10; /* digit count, radix sort buckets */
const unsigned int   POWERS[] = {
    1, 
    10, 
    100, 
    1000, 
    10000, 
    100000, 
    1000000, 
    10000000, 
    100000000, 
    1000000000
};

inline size_t power(size_t exp)
{
    return POWERS[exp];
}

/**
 * Constructor
 */
Pointz::Pointz(Coord* points, int npoints, int dimension)
{    
	this->m_points = points;
	this->m_size = npoints;
	this->m_dimensions = dimension;
    this->m_cells = new Cell[this->m_size];
	this->m_initialOrder = new size_t[this->m_size];
	std::iota(this->m_initialOrder, this->m_initialOrder + this->m_size, 0);
}


/**
 * Operations
 */
void Pointz::resetClusters(Cluster* clusters)
{
    std::fill(clusters, clusters + this->m_size, NOT_VISITED);
    this->m_clusters = clusters;
}

void Pointz::sortByOrder(size_t maxDigits, size_t lowerBound, size_t upperBound)
{    
    std::vector<std::vector<size_t> > buckets(maxDigits, std::vector<size_t>(DIGITS));
    size_t size            = upperBound - lowerBound;
    this->m_points        += lowerBound * this->m_dimensions;
    this->m_initialOrder  += lowerBound;
    this->m_clusters      += lowerBound;
    Cluster* clusterBuffer = new Cluster[size];
    size_t*  orderBuffer   = new size_t[size];
    Coord*   pointsBuffer  = new Coord[size * this->m_dimensions];
    
    for (size_t j = 0; j < maxDigits; ++j)
    {
        for(size_t f = 0; f < DIGITS; ++f)
        {
            buckets[j][f] = 0;
        }
    }

    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < maxDigits; ++j)
        {
            const size_t base  = power(j);
            const size_t digit = this->m_initialOrder[i] / base % DIGITS;
            #pragma omp atomic
            ++buckets[j][digit];
        }
    }

    #pragma omp parallel for shared(buckets)
    for (size_t j = 0; j < maxDigits; ++j)
    {
        for (size_t f = 1; f < DIGITS; ++f)
        {
            buckets[j][f] += buckets[j][f-1];
        }
    }

    for (size_t j = 0; j < maxDigits; ++j)
    {
        const size_t base = power(j);
        for (size_t i = size - 1; i < NOT_VISITED; --i)
        {
            size_t unit = this->m_initialOrder[i] / base % DIGITS;
            size_t pos  = --buckets[j][unit];

            for (size_t d = 0; d < this->dimensions(); ++d)
            {
                pointsBuffer[pos * this->m_dimensions + d] = this->operator [](i)[d];
            }
            orderBuffer[pos]   = this->m_initialOrder[i];
            clusterBuffer[pos] = this->m_clusters[i];
        }
        std::copy(orderBuffer   , orderBuffer   + size, this->m_initialOrder);
        std::copy(clusterBuffer , clusterBuffer + size, this->m_clusters );
        std::copy(pointsBuffer  , pointsBuffer  + size * this->m_dimensions, this->m_points);
    }
    
    
    this->m_points       -= lowerBound * this->m_dimensions;
    this->m_initialOrder -= lowerBound;
    this->m_clusters     -= lowerBound;
    
    delete[] pointsBuffer;
    delete[] orderBuffer;
    delete[] clusterBuffer;
}

 void Pointz::sortByCell(const CellIndex& index)
 {
     // Initialization
     Cell*   cellBuffer  = new Cell[this->m_size];
     size_t* orderBuffer = new size_t[this->m_size];
     Coord*  pointBuffer = new Coord[this->m_size * this->m_dimensions];
     
     std::unordered_map<size_t, std::atomic<size_t>> counter;
     for (auto pair : index)
     {
         counter[pair.first].store(0);
     }
     
     // Sorting Off-Place
     #pragma omp parallel for
     for (size_t i = 0; i < this->m_size; ++i)
     {
         const auto& locator = index.find(this->m_cells[i]);
         size_t copyTo       = locator->second.first + (counter[locator->first]++);
         for (size_t d = 0; d < this->m_dimensions; ++d)
         {
             pointBuffer[copyTo * this->m_dimensions + d] = this->m_points[i * this->m_dimensions + d];
         }
         cellBuffer[copyTo]  = this->m_cells[i];
         orderBuffer[copyTo] = this->m_initialOrder[i];
     }   
     
     // Copy In-Place
     std::copy(cellBuffer,  cellBuffer  + this->m_size, this->m_cells);
     std::copy(orderBuffer, orderBuffer + this->m_size, this->m_initialOrder);
     std::copy(pointBuffer, pointBuffer + this->m_size * this->m_dimensions, this->m_points);
     
     delete[] cellBuffer;
     delete[] pointBuffer;
}

/**
 * Destructor
 */
Pointz::~Pointz()
{
    delete[] this->m_cells;
    delete[] this->m_initialOrder;
}
