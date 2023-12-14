#ifndef _CLUCENE_WRAPPER_H_
#define _CLUCENE_WRAPPER_H_

#include "Lucene.h"
#include "StandardAnalyzer.h"
#include "MyAnalyzer.h"
#include "MySimilarity.h"
#include "IndexWriter.h"
#include "Field.h"
#include "Document.h"
#include "Collector.h"
#include "QueryParser.h"
#include "BooleanQuery.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "TermPositionVector.h"
#include "TermVectorOffsetInfo.h"
#include "StringUtils.h"
#include "FSDirectory.h"
#include "TermQuery.h"
#include "BooleanClause.h"
#include "Term.h"
#include "TermEnum.h"
#include "StopAnalyzer.h"
#include "HashSet.h"
#include "SpanTermQuery.h"
#include "SpanOrQuery.h"
#include "Spans.h"
#include "Scorer.h"
#include "sync_queue.h"

#include <memory>

using namespace Lucene;

/// create documents
DocumentPtr create_document(const std::wstring& name, const std::wstring& data);
DocumentPtr create_multiple_names_document(const std::vector<std::wstring>& names, const std::wstring& data);

/// search in index
std::vector<std::string> search_index(const std::string& query, const std::string& index, SyncQueue* queue = nullptr);
std::vector<std::string> search_query_in_index(const QueryPtr& query, const std::string& index, SyncQueue* queue = nullptr);
std::vector<std::string> search_keywords_in_index(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr);
std::map<std::string, std::vector<uint64_t>> search_keywords_in_index_with_tv_offsets(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr);
std::map<std::string, std::vector<uint64_t>> search_keywords_in_index_with_global_offsets(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr);
std::map<std::string, double> search_keywords_in_index_with_scores(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr);
std::map<std::string, uint32_t> search_keywords_in_index_with_termfreqs(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr);

// debug
void print_terms(const std::string& index);

// utilities and wrappers
AnalyzerPtr get_analyzer();
IndexWriterPtr get_index_writer(const std::string& index_path);
std::wstring toWstring(const std::string&);

IndexWriterPtr protectedAddDocument(IndexWriterPtr writer, DocumentPtr document, const std::string& index_path);

// used only for inline in-doc (which does not work). not really relevant.
DocumentPtr create_fingerprint_document(std::wstring& fp, const std::wstring& data);
DocumentPtr create_names_only_document(const std::vector<std::wstring>& names);

uint32_t getLastNumTokens();

/*
	synopsis:
	writer = get_index_writer(index_directory_path)
	writer->addDocument(create_document(file_name1, data1))
	writer->addDocument(create_document(file_name2, data2))
	writer->close()
*/

#endif // _CLUCENE_WRAPPER_H_