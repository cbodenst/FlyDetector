#include "constants.h"
#include "space.h"

#include <algorithm>
#include <limits>
#include <iomanip>
#include <math.h>
#include <numeric>
#include <omp.h>

void mergeCells(CellCounter& omp_in, CellCounter& omp_out)
{
    for(auto pair: omp_in)
    {
        omp_out[pair.first] += pair.second;
    }
}

void vectorMin(std::vector<Coord>& omp_in, std::vector<Coord>& omp_out)
{
    size_t index = -1;
    for (auto& coordinate : omp_out)
    {
        coordinate = std::min(coordinate, omp_in[++index]);
    }
}

void vectorMax(std::vector<Coord>& omp_in, std::vector<Coord>& omp_out)
{
    size_t index = -1;
    for (auto& coordinate : omp_out)
    {
        coordinate = std::max(coordinate, omp_in[++index]);
    }
}

#pragma omp declare reduction(mergeCells: CellCounter: mergeCells(omp_in, omp_out)) initializer(omp_priv(CellCounter()))
#pragma omp declare reduction(vectorMax: std::vector<Coord>: vectorMax(omp_in, omp_out)) initializer(omp_priv(omp_orig))
#pragma omp declare reduction(vectorMin: std::vector<Coord>: vectorMin(omp_in, omp_out)) initializer(omp_priv(omp_orig))

Space::Space(Pointz& points, float epsilon) :
    m_points(points),
    m_total(1),
    m_lastCell(0),
    m_cells(std::vector<size_t>(points.dimensions(),  0)),
    m_maximum(std::vector<Coord>(points.dimensions(), -std::numeric_limits<Coord>::max())),
    m_minimum(std::vector<Coord>(points.dimensions(),  std::numeric_limits<Coord>::max())),
    m_swapDims(std::vector<size_t>(points.dimensions()))
{
    std::iota(this->m_swapDims.begin(), this->m_swapDims.end(), 0);
    this->computeDimensions(epsilon);
    CellCounter cellCounter = this->computeCells(epsilon);
    this->computeIndex(cellCounter);
    this->m_points.sortByCell(this->m_cellIndex);
}

CellCounter Space::computeCells(float epsilon)
{
    CellCounter cellCounter;
    //#pragma omp parallel for reduction(mergeCells: cellCounter)
    for (size_t i = 0; i < this->m_points.size(); ++i)
    {
        size_t cell    = 0;
        size_t cellAcc = 1;

        for (size_t d : this->m_swapDims)
        {
            const Coord minimum = this->m_minimum[d];
            const Coord point   = this->m_points[i][d];

            size_t dim_index = (size_t) floor((point - minimum) / epsilon);
            cell            += dim_index * cellAcc;
            cellAcc         *= this->m_cells[d];
        }
        this->m_points.cell(i, cell);
        cellCounter[cell] += 1;
    } 
    return cellCounter;
}  

void Space::computeDimensions(float epsilon)
{
    const size_t dimensions = this->m_points.dimensions();
    auto& maximum = this->m_maximum;
    auto& minimum = this->m_minimum;
    
    #pragma omp parallel for reduction(vectorMin: minimum) reduction(vectorMax: maximum)
    for (size_t iter = 0; iter < this->m_points.size(); ++iter)
    {
        const auto& point = this->m_points[iter];
        for (size_t d = 0; d < dimensions; ++d)
        {
            const Coord& coordinate = point[d];
            minimum[d] = std::min(minimum[d], coordinate);
            maximum[d] = std::max(maximum[d], coordinate);
        }
    }

    // compute cell count
    for (size_t d = 0; d < this->m_cells.size(); ++d)
    {
        size_t cells     = (size_t) ceil((this->m_maximum[d] - this->m_minimum[d]) / epsilon) + 1;
        this->m_cells[d] = cells;
        this->m_total   *= cells;
    }
    
    this->swapDimensions();
    this->m_lastCell = this->m_total;
}

void Space::computeIndex(CellCounter& cellCounter)
{
    // setup index
    size_t accumulator = 0;
    
    for (auto& cell : cellCounter)
    {
        auto& index  = m_cellIndex[cell.first];
        index.first  = accumulator;
        index.second = cell.second;
        accumulator += cell.second;
    }
    m_cellIndex[this->m_lastCell].first  = m_points.size();
    m_cellIndex[this->m_lastCell].second = 0;
}

void Space::swapDimensions()
{
    const auto& dims = this->m_cells;
    std::sort(this->m_swapDims.begin(), this->m_swapDims.end(), [dims](size_t a, size_t b)
    {
        return dims[a] < dims[b];
    });   
}

/**
 * Operations
 */

std::vector<size_t> Space::getNeighbors(const size_t cellId) const
{    
    const CellIndex& cellIdx = this->m_cellIndex;
    
    std::vector<size_t> neighborCells;
    neighborCells.reserve(std::pow(3, m_points.dimensions()));
    neighborCells.push_back(cellId);

    size_t lowerSpace     = 1;
    size_t currentSpace   = 1;
    size_t numberOfPoints = cellIdx.find(cellId)->second.second;
    
    // here be dragons!
    for (size_t d : this->m_swapDims)
    {
        currentSpace *= this->m_cells[d];
        
        for (size_t i = 0, end = neighborCells.size(); i < end; ++i)
        {
            const size_t current = neighborCells[i];
            // check "left" neighbor - a.k.a the cell in the current dimension that has a lower number
            const long int left = current - lowerSpace;
            auto found    = cellIdx.find(left);
            if (current % currentSpace >= lowerSpace)
            {
                neighborCells.push_back(left);
                numberOfPoints += found != cellIdx.end() ? found->second.second : 0;
            }

            // check "right" neighbor - a.k.a the cell in the current dimension that has a higher number
            const long int right = current + lowerSpace;
            found = cellIdx.find(right);
            if (current % currentSpace < currentSpace - lowerSpace)
            {
                neighborCells.push_back(right);
                numberOfPoints += found != cellIdx.end() ? found->second.second : 0;
            }
        }
        
        lowerSpace = currentSpace;
    }

    std::vector<size_t> neighborPoints;
    neighborPoints.reserve(numberOfPoints);
    
    for (size_t neighborCell : neighborCells)
    {
        const auto found = cellIdx.find(neighborCell);
        if (found == cellIdx.end())
        {
            continue;
        }
        
        const std::pair<size_t, size_t>& locator = found->second;
        neighborPoints.resize(neighborPoints.size() + locator.second);
        auto end = neighborPoints.end();
        std::iota(end - locator.second, end, locator.first);
    }

    return neighborPoints;
}

size_t Space::regionQuery(const size_t pointIndex, const std::vector<size_t>& neighborPoints, const float EPS2, std::vector<size_t>& minPointsArea) const
{
    const Coord* point = this->m_points[pointIndex];
    // this MUST be a positive number so that atomicMin will result in correct result with set corePoint bit
    size_t clusterId   = pointIndex + 1;
    
    for (size_t neighbor: neighborPoints)
    {
        float offset            = 0.0f;
        const Coord* otherPoint = this->m_points[neighbor];

        for (size_t d = 0; d < this->m_points.dimensions(); ++d)
        {
            offset += std::pow(otherPoint[d] - point[d], 2);
        }
        if (offset <= EPS2)
        {
            minPointsArea.push_back(neighbor);  
            size_t neighborCluster = this->m_points.cluster(neighbor);
            if (neighborCluster != NOT_VISITED && this->m_points.corePoint(neighbor))
            {
                clusterId = std::min(clusterId, neighborCluster);
            }
        }
    }

    return clusterId;
}
