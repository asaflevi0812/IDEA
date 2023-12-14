
#include "Lucene.h"
#include "StandardAnalyzer.h"
#include "LengthFilter.h"

extern bool short_keywords;

using namespace Lucene;

class MyAnalyzer : public StandardAnalyzer {
public:
    MyAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopWords) : 
        StandardAnalyzer(matchVersion, stopWords) {
        // intentionally left blank
    }

    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        if (short_keywords) {
            return StandardAnalyzer::tokenStream(fieldName, reader);
        }
        return newLucene<LengthFilter>(StandardAnalyzer::tokenStream(fieldName, reader), 3, getMaxTokenLength());
    }

    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
        if (short_keywords) {
            return StandardAnalyzer::reusableTokenStream(fieldName, reader);
        }
        return newLucene<LengthFilter>(StandardAnalyzer::reusableTokenStream(fieldName, reader), 3, getMaxTokenLength());
    }
};