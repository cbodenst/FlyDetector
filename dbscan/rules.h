#ifndef RULES_H
#define	RULES_H

#include "constants.h"

#include <map>

class Rules
{
    std::map<ssize_t, ssize_t> m_rules;
    
public:
    Rules();
    
    inline const std::map<ssize_t, ssize_t>::const_iterator begin() const
    {
        return this->m_rules.begin();
    }
    
    inline const std::map<ssize_t, ssize_t>::const_iterator end() const
    {
        return this->m_rules.end();
    }
    
    inline size_t size() const
    {
        return this->m_rules.size();
    }
    
    ssize_t rule(const ssize_t index) const;
    bool update(const ssize_t first, const ssize_t second);
};

void merge(Rules& omp_out, Rules& omp_in);
#pragma omp declare reduction(merge: Rules: merge(omp_out, omp_in)) initializer(omp_priv(omp_orig))

#endif	// RULES_H

