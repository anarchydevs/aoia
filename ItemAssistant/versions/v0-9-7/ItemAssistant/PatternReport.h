#ifndef PATTERNREPORT_H
#define PATTERNREPORT_H


/**
 * \brief
 * This class generates a HTML report for the specified search criterias.
 *
 * Based on the criterias specified in the constructor a HTML report-template 
 * is loaded from a resource, and filled with the results of the search.
 */
class PatternReport
{
public:
    PatternReport(unsigned int dimensionid, unsigned int pbid, unsigned int toonid, bool excludeassembled);
    virtual ~PatternReport();

    std::tstring toString() const;

private:
    unsigned int m_dimensionid;
    unsigned int m_pbid;
    unsigned int m_toonid;
    bool m_excludeassembled;

    std::tstring m_avail;
    std::tstring m_toonname;
    std::tstring m_time;
    std::tstring m_pbname;
    std::tstring m_table;
};


#endif // PATTERNREPORT_H
