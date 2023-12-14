#include "MySimilarity.h"


static Lucene::MySimilarityPtr singleton = nullptr;

Lucene::MySimilarityPtr getStaticMySimilarity() {
    if (!singleton) {
        singleton = Lucene::newLucene<Lucene::MySimilarity>();
    }
    
    return singleton;
}

namespace Lucene {

double MySimilarity::tf(double freq) {
    return DefaultSimilarity::tf(freq);
}

double MySimilarity::idf(int32_t docFreq, int32_t numDocs) {
    return DefaultSimilarity::idf(docFreq, numDocs);
}

double MySimilarity::computeNorm(const String& fieldName, const FieldInvertStatePtr& state) {
    return DefaultSimilarity::computeNorm(fieldName, state);
}

double MySimilarity::lengthNorm(const String& fieldName, int32_t numTokens) {
    if (fieldName == L"data") {
        lastNumTokens = numTokens;
    }

    return DefaultSimilarity::lengthNorm(fieldName, numTokens);
}

double MySimilarity::queryNorm(double sumOfSquaredWeights) {
    return DefaultSimilarity::queryNorm(sumOfSquaredWeights);
}

double MySimilarity::sloppyFreq(int32_t distance) {
    return DefaultSimilarity::sloppyFreq(distance);
}

double MySimilarity::coord(int32_t overlap, int32_t maxOverlap) {
    return DefaultSimilarity::coord(overlap, maxOverlap);
}

};