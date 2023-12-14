#ifndef _MY_SIMILARITY_H_
#define _MY_SIMILARITY_H_

#include "Lucene.h"
#include "DefaultSimilarity.h"

namespace Lucene {

class LPPAPI MySimilarity : public DefaultSimilarity {
public:
    MySimilarity() { }
    virtual ~MySimilarity() { }

    LUCENE_CLASS(MySimilarity);

    virtual double tf(double freq);

    virtual double idf(int32_t docFreq, int32_t numDocs);

    virtual double computeNorm(const String& fieldName, const FieldInvertStatePtr& state);

    virtual double lengthNorm(const String& fieldName, int32_t numTokens) ;

    virtual double queryNorm(double sumOfSquaredWeights);

    virtual double sloppyFreq(int32_t distance);

    virtual double coord(int32_t overlap, int32_t maxOverlap);

    uint32_t lastNumTokens = 0;
};

DECLARE_SHARED_PTR(MySimilarity);

};

Lucene::MySimilarityPtr getStaticMySimilarity();

#endif