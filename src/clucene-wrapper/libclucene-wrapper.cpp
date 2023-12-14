#include "libclucene-wrapper.h"
#include "destor.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

double searcher_load_time = 0, dtor_time = 0;

extern int job;

std::wstring toWstring(const std::string& str) {
	std::wstring temp(str.size(), L'\0');
	for (size_t i = 0; i < str.size(); i++)
	{
		if (0 <= str[i] && str[i] <= 127) {
			temp[i] = wchar_t(str[i]);
		} else {
			temp[i] = L'?';
		}
	}
	return temp;
}

const wchar_t* _STOP_LIST[] = {
    L"a", L"an", L"and", L"are", L"as", L"at", L"be", L"but", L"by",
    L"for", L"if", L"in", L"into", L"is", L"it", L"no", L"not", L"of",
    L"on", L"or", L"such", L"that", L"the", L"their", L"then", L"there",
    L"these", L"they", L"this", L"to", L"was", L"will", L"with",L"auto", 
	L"break", L"case", L"char", L"const", L"continue", L"default", L"do", 
	L"double", L"else", L"enum", L"extern", L"float", L"for", L"go", L"to", 
	L"if", L"int", L"long", L"register", L"return", L"short", L"signed", 
	L"sizeof", L"static", L"struct", L"switch", L"typedef", L"union", 
	L"unsigned", L"void", L"volatile", L"while", L"b", L"c", L"d",
	L"e", L"f", L"g", L"h", L"i", L"j", L"k", L"l", L"m", L"n", L"o", L"p",
	L"q", L"r", L"s", L"t", L"u", L"v", L"w", L"x", L"y", L"z", L"0", L"1", 
	L"2", L"3", L"4", L"6", L"7", L"8", L"9", L"10"
};

HashSet<String> STOP_LIST;

class MyHitCollector : public Collector {
public:
	IndexReaderPtr m_reader;
	std::vector<std::string> m_results;
	std::vector<int32_t> m_docIds;
	std::vector<double> m_scores;
	std::vector<uint32_t> m_termFreqs;
	int32_t m_docBase;
	SyncQueue* m_queue;
	ScorerPtr m_scorer;

	explicit MyHitCollector(SyncQueue* queue) : m_queue(queue) {}
	explicit MyHitCollector() : MyHitCollector(nullptr) {}

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) override {
         this->m_docBase = docBase;
		 this->m_reader = reader;
		 this->m_scorer = nullptr;
	}

	virtual bool acceptsDocsOutOfOrder() override {
		return true;
	}

	virtual void setScorer(const ScorerPtr& scorer) override {
		m_scorer = scorer;
	}

	virtual void collect(const int32_t doc) override {
		if (destor.is_threaded_lookup) {
			DocumentPtr document  = m_reader->document(doc);
			auto filename(document->get(toWstring("name")));
			std::string filenameUtf8 = StringUtils::toUTF8(filename);
			m_results.push_back(filenameUtf8);
			sync_queue_push(m_queue, new std::string(filenameUtf8));
		} else {
			m_docIds.push_back(m_docBase + doc);
			if (m_scorer && destor.tf_idf) {
				if (job == DESTOR_DEDUPLICATED_LOOKUP) {
					m_termFreqs.push_back(m_scorer->termFreq());
				} else {
					m_scores.push_back(m_scorer->score());
				}
			}
		}
	}

	virtual std::vector<std::string> getResultsFromSearcher(IndexSearcherPtr searcher) {
		std::vector<std::string> results;
		
		for (auto doc : m_docIds) {
			DocumentPtr document  = searcher->doc(doc);
			for (const auto& filename : document->getValues(toWstring("name"))) {
				results.push_back(StringUtils::toUTF8(filename));
			}
		}

		return results;
	}
	
	virtual std::map<std::string, double> getResultsFromSearcherWithScores(IndexSearcherPtr searcher) {
		std::map<std::string, double> results;
		
		for (size_t i = 0; i < m_docIds.size(); ++i) {
			auto doc = m_docIds[i];
			double score = m_scores[i];
			DocumentPtr document  = searcher->doc(doc);
			const auto& filename = document->get(toWstring("name"));
			results.emplace(StringUtils::toUTF8(filename), score);
		}

		return results;
	}
	
	virtual std::map<std::string, uint32_t> getResultsFromSearcherWithTermFreqs(IndexSearcherPtr searcher) {
		std::map<std::string, uint32_t> results;
		
		for (size_t i = 0; i < m_docIds.size(); ++i) {
			auto doc = m_docIds[i];
			uint32_t freq = m_termFreqs[i];
			DocumentPtr document  = searcher->doc(doc);
			const auto& filename = document->get(toWstring("name"));
			results.emplace(StringUtils::toUTF8(filename), freq);
		}

		return results;
	}

	virtual std::map<std::string, std::vector<uint64_t>> getResultsFromSearcherWithOffsets(IndexSearcherPtr searcher, std::vector<std::string> keywords) {
		std::map<std::string, std::vector<uint64_t>> results;
		
		for (auto doc : m_docIds) {
			DocumentPtr document  = searcher->doc(doc);
			for (const auto& filename : document->getValues(toWstring("name"))) {
				std::string name = StringUtils::toUTF8(filename);
				std::vector<uint64_t> accumulatedOffsets;
				TermPositionVectorPtr tv = 
					boost::dynamic_pointer_cast<TermPositionVector>(searcher->getIndexReader()->getTermFreqVector(doc, toWstring("data")));
				for (const auto& keyword : keywords) {
					uint32_t idx = tv->indexOf(toWstring(keyword));
					auto offsets = tv->getOffsets(idx);
					for (const auto& offsetObj : offsets) {
						accumulatedOffsets.push_back(offsetObj->getStartOffset());
					}
				}
				results.emplace(name, accumulatedOffsets);
			}
		}

		return results;
	}
};

using MyHitCollectorPtr = boost::shared_ptr<MyHitCollector>;

AbstractField::TermVector getTermVectorMode() {
	AbstractField::TermVector termVectorMode = AbstractField::TERM_VECTOR_NO;
	switch (destor.offsets_mode)
	{
	default:
	case NO_OFFSETS: termVectorMode = AbstractField::TERM_VECTOR_NO; break;
	case TERM_VECTOR_OFFSETS: termVectorMode = AbstractField::TERM_VECTOR_WITH_OFFSETS; break;
	}
	return termVectorMode;
}


DocumentPtr create_document(const std::wstring& name, const std::wstring& data) {


	FieldPtr nameField = newLucene<Field>(L"name", 
										  name, 
										  AbstractField::STORE_YES,
										  AbstractField::INDEX_NO, 
										  AbstractField::TERM_VECTOR_NO);
	FieldPtr dataField = newLucene<Field>(L"data", 
										  data, 
										  AbstractField::STORE_NO,
										  destor.tf_idf ? AbstractField::INDEX_ANALYZED : 
										  				AbstractField::INDEX_ANALYZED_NO_NORMS, 
										  getTermVectorMode());
	nameField->setOmitTermFreqAndPositions(true);
	if (destor.offsets_mode != GLOBAL_OFFSETS && !destor.tf_idf) {
		dataField->setOmitTermFreqAndPositions(true);
	}
	DocumentPtr document = newLucene<Document>();
	document->add(nameField);
	document->add(dataField);
	return document;
}

DocumentPtr create_fingerprint_document(std::wstring& fp, const std::wstring& data) {
	FieldPtr fpField = newLucene<Field>(L"fp", 
										  fp, 
										  AbstractField::STORE_NO,
										  AbstractField::INDEX_ANALYZED_NO_NORMS, 
										  AbstractField::TERM_VECTOR_NO);
	FieldPtr dataField = newLucene<Field>(L"data", 
										  data, 
										  AbstractField::STORE_NO,
										  AbstractField::INDEX_ANALYZED_NO_NORMS, 
										  getTermVectorMode());
	fpField->setOmitTermFreqAndPositions(true);
	dataField->setOmitTermFreqAndPositions(true);
	DocumentPtr document = newLucene<Document>();
	document->add(fpField);
	document->add(dataField);
	return document;
}

DocumentPtr create_multiple_names_document(const std::vector<std::wstring>& names, const std::wstring& data) {
	DocumentPtr document = newLucene<Document>();
	for (const auto& name : names) {
		FieldPtr nameField = newLucene<Field>(L"name", 
											name, 
											AbstractField::STORE_YES,
											AbstractField::INDEX_NO, 
											AbstractField::TERM_VECTOR_NO);
		nameField->setOmitTermFreqAndPositions(true);
		document->add(nameField);
	}
	FieldPtr dataField = newLucene<Field>(L"data", 
										  data, 
										  AbstractField::STORE_NO,
										  AbstractField::INDEX_ANALYZED_NO_NORMS, 
										  getTermVectorMode());
	dataField->setOmitTermFreqAndPositions(true);
	document->add(dataField);
	return document;
}

DocumentPtr create_names_only_document(const std::vector<std::wstring>& names) {
	DocumentPtr document = newLucene<Document>();
	for (const auto& name : names) {
		FieldPtr nameField = newLucene<Field>(L"name", 
											name, 
											AbstractField::STORE_YES,
											AbstractField::INDEX_NO, 
											AbstractField::TERM_VECTOR_NO);
		nameField->setOmitTermFreqAndPositions(true);
		document->add(nameField);
	}
	return document;
}

IndexWriterPtr get_index_writer(const std::string& index_path) {
	bool create = (access(index_path.c_str(), F_OK) != 0);

	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());

	auto writer =  newLucene<IndexWriter>(FSDirectory::open(toWstring(index_path)), 
										  get_analyzer(), 
										  create, 
										  std::numeric_limits<int>::max());
	writer->setSimilarity(getStaticMySimilarity());
	return writer;
}

std::vector<std::string> search_index(const std::string& query, const std::string& index, SyncQueue* queue) {	
	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());

	QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_30,
												   toWstring("data"),
												   get_analyzer());

	return search_query_in_index(parser->parse(toWstring(query)), index, queue);
}

std::vector<std::string> search_keywords_in_index(const std::vector<std::string>& keywords, 
												  const std::string& index, 
												  SyncQueue* queue) {

	double query_creation_time = 0;
	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());

	BooleanQueryPtr query = newLucene<BooleanQuery>();	

	for (size_t i = 0; i < keywords.size(); i++) {
		const String keyword = toWstring(keywords[i]);
		query->add(newLucene<TermQuery>(newLucene<Term>(toWstring("data"), 
														StringUtils::toLower(keyword))),
				   BooleanClause::SHOULD);
	}

	TIMER_END(1, query_creation_time);
	printf("query creation time: %.3f\n", query_creation_time / 1000000);
	
	return search_query_in_index(query, index, queue);
}



std::vector<std::string> search_query_in_index(const QueryPtr& query, const std::string& index, SyncQueue* queue) {
	MyHitCollectorPtr collector = newLucene<MyHitCollector>(queue);
	double doc_id_lookup_time = 0, doc_fetch_lookup_time = 0;
	std::vector<std::string> results;
	TIMER_DECLARE(1);
	TIMER_DECLARE(2);
	TIMER_DECLARE(3);
	TIMER_DECLARE(4);
	{
		TIMER_BEGIN(3);
		IndexSearcherPtr searcher = newLucene<IndexSearcher>(FSDirectory::open(toWstring(index)));
		TIMER_END(3, searcher_load_time);

		TIMER_BEGIN(1);
		searcher->search(query, nullptr, collector);
		TIMER_END(1, doc_id_lookup_time);

		if (destor.is_threaded_lookup) { 
			results = collector->m_results;
		} else {
			TIMER_BEGIN(2);
			results = collector->getResultsFromSearcher(searcher);
			TIMER_END(2, doc_fetch_lookup_time);
		}
		TIMER_BEGIN(4);
	}
	TIMER_END(4, dtor_time);
	dtor_time /= 1000000;
	printf("searcher load time: %.3f\n", searcher_load_time / 1000000);
	printf("doc lookup time: %.3f\n", doc_id_lookup_time / 1000000);
	printf("doc fetch lookup time: %.3f\n", doc_fetch_lookup_time / 1000000);
	printf("destructors' time: %.3f\n", dtor_time);
	
	return results;
}

std::map<std::string, std::vector<uint64_t>> search_keywords_in_index_with_tv_offsets(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr) {
	
	double query_creation_time = 0;
	TIMER_DECLARE(7);
	TIMER_BEGIN(7);

	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());

	BooleanQueryPtr query = newLucene<BooleanQuery>();	

	for (size_t i = 0; i < keywords.size(); i++) {
		const String keyword = toWstring(keywords[i]);
		query->add(newLucene<TermQuery>(newLucene<Term>(toWstring("data"), 
														StringUtils::toLower(keyword))),
				   BooleanClause::SHOULD);
	}

	TIMER_END(7, query_creation_time);
	printf("query creation time: %.3f\n", query_creation_time / 1000000);
	
	MyHitCollectorPtr collector = newLucene<MyHitCollector>(queue);
	double doc_id_lookup_time = 0, doc_fetch_lookup_time = 0;
	std::map<std::string, std::vector<uint64_t>> results;
	TIMER_DECLARE(1);
	TIMER_DECLARE(2);
	TIMER_DECLARE(3);
	TIMER_DECLARE(4);
	{
		TIMER_BEGIN(3);
		IndexSearcherPtr searcher = newLucene<IndexSearcher>(FSDirectory::open(toWstring(index)));
		TIMER_END(3, searcher_load_time);

		TIMER_BEGIN(1);
		searcher->search(query, nullptr, collector);
		TIMER_END(1, doc_id_lookup_time);

		TIMER_BEGIN(2);
		results = collector->getResultsFromSearcherWithOffsets(searcher, keywords);
		TIMER_END(2, doc_fetch_lookup_time);
		TIMER_BEGIN(4);
	}
	TIMER_END(4, dtor_time);
	dtor_time /= 1000000;
	printf("searcher load time: %.3f\n", searcher_load_time / 1000000);
	printf("doc lookup time: %.3f\n", doc_id_lookup_time / 1000000);
	printf("doc fetch lookup time: %.3f\n", doc_fetch_lookup_time / 1000000);
	printf("destructors' time: %.3f\n", dtor_time);
	
	return results;
}

std::map<std::string, double> search_keywords_in_index_with_scores(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr) {
	
	double query_creation_time = 0;
	TIMER_DECLARE(7);
	TIMER_BEGIN(7);

	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());

	BooleanQueryPtr query = newLucene<BooleanQuery>(false);	

	for (size_t i = 0; i < keywords.size(); i++) {
		const String keyword = toWstring(keywords[i]);
		query->add(newLucene<TermQuery>(newLucene<Term>(toWstring("data"), 
														StringUtils::toLower(keyword))),
				   BooleanClause::SHOULD);
	}

	TIMER_END(7, query_creation_time);
	printf("query creation time: %.3f\n", query_creation_time / 1000000);
	
	MyHitCollectorPtr collector = newLucene<MyHitCollector>(queue);
	double doc_id_lookup_time = 0, doc_fetch_lookup_time = 0;
	std::map<std::string, double> results;
	TIMER_DECLARE(1);
	TIMER_DECLARE(2);
	TIMER_DECLARE(3);
	TIMER_DECLARE(4);
	{
		TIMER_BEGIN(3);
		IndexSearcherPtr searcher = newLucene<IndexSearcher>(FSDirectory::open(toWstring(index)));
		TIMER_END(3, searcher_load_time);

		TIMER_BEGIN(1);
		searcher->search(query, nullptr, collector);
		TIMER_END(1, doc_id_lookup_time);

		TIMER_BEGIN(2);
		results = collector->getResultsFromSearcherWithScores(searcher);
		TIMER_END(2, doc_fetch_lookup_time);

		// std::wcout << searcher->explain(query, collector->m_docIds[0])->toString() << std::endl;

		TIMER_BEGIN(4);
	}
	TIMER_END(4, dtor_time);
	dtor_time /= 1000000;
	printf("searcher load time: %.3f\n", searcher_load_time / 1000000);
	printf("doc lookup time: %.3f\n", doc_id_lookup_time / 1000000);
	printf("doc fetch lookup time: %.3f\n", doc_fetch_lookup_time / 1000000);
	printf("destructors' time: %.3f\n", dtor_time);
	
	return results;
}


std::map<std::string, uint32_t> search_keywords_in_index_with_termfreqs(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr) {
	
	double query_creation_time = 0;
	TIMER_DECLARE(7);
	TIMER_BEGIN(7);

	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());

	BooleanQueryPtr query = newLucene<BooleanQuery>(false);	

	for (size_t i = 0; i < keywords.size(); i++) {
		const String keyword = toWstring(keywords[i]);
		query->add(newLucene<TermQuery>(newLucene<Term>(toWstring("data"), 
														StringUtils::toLower(keyword))),
				   BooleanClause::SHOULD);
	}

	TIMER_END(7, query_creation_time);
	printf("query creation time: %.3f\n", query_creation_time / 1000000);
	
	MyHitCollectorPtr collector = newLucene<MyHitCollector>(queue);
	double doc_id_lookup_time = 0, doc_fetch_lookup_time = 0;
	std::map<std::string, uint32_t> results;
	TIMER_DECLARE(1);
	TIMER_DECLARE(2);
	TIMER_DECLARE(3);
	TIMER_DECLARE(4);
	{
		TIMER_BEGIN(3);
		IndexSearcherPtr searcher = newLucene<IndexSearcher>(FSDirectory::open(toWstring(index)));
		TIMER_END(3, searcher_load_time);

		TIMER_BEGIN(1);
		searcher->search(query, nullptr, collector);
		TIMER_END(1, doc_id_lookup_time);

		TIMER_BEGIN(2);
		results = collector->getResultsFromSearcherWithTermFreqs(searcher);
		TIMER_END(2, doc_fetch_lookup_time);

		// std::wcout << searcher->explain(query, collector->m_docIds[0])->toString() << std::endl;

		TIMER_BEGIN(4);
	}
	TIMER_END(4, dtor_time);
	dtor_time /= 1000000;
	printf("searcher load time: %.3f\n", searcher_load_time / 1000000);
	printf("doc lookup time: %.3f\n", doc_id_lookup_time / 1000000);
	printf("doc fetch lookup time: %.3f\n", doc_fetch_lookup_time / 1000000);
	printf("destructors' time: %.3f\n", dtor_time);
	
	return results;
}


std::map<std::string, std::vector<uint64_t>> search_keywords_in_index_with_global_offsets(const std::vector<std::string>& keywords, const std::string& index, SyncQueue* queue = nullptr) {
	std::map<uint32_t, std::vector<uint64_t>> results_docid;
	std::map<std::string, std::vector<uint64_t>> results;

	BooleanQuery::setMaxClauseCount(std::numeric_limits<int32_t>::max());
	Collection<SpanQueryPtr> clauses = Collection<SpanQueryPtr>::newInstance();

	for (size_t i = 0; i < keywords.size(); i++) {
		const String keyword = toWstring(keywords[i]);
		clauses.add(newLucene<SpanTermQuery>(newLucene<Term>(toWstring("data"), 
														StringUtils::toLower(keyword))));
	}

	SpanOrQueryPtr query = newLucene<SpanOrQuery>(std::move(clauses));

	auto indexReader = IndexReader::open(FSDirectory::open(toWstring(index)));

	SpansPtr spans = query->getSpans(indexReader);

	int64_t current_doc_id = -1;
	std::vector<uint64_t> doc_offsets;

	while (spans->next()) {
		if (spans->doc() != current_doc_id) {
			if (current_doc_id != -1) {
				results_docid.emplace(current_doc_id, std::move(doc_offsets));
				doc_offsets = std::vector<uint64_t>();
			}
			current_doc_id = spans->doc();
		}

		doc_offsets.push_back(spans->start());
	}

	if (current_doc_id != -1) {
		results_docid.emplace(current_doc_id, std::move(doc_offsets));
	}

	for (auto& pair : results_docid) {
		auto document = indexReader->document(pair.first);
		std::string docName = StringUtils::toUTF8(document->get(toWstring("name")));
		results.emplace(docName, std::move(pair.second));
	}

	return results;
}

void print_terms(const std::string& index) {
	IndexSearcherPtr searcher = newLucene<IndexSearcher>(FSDirectory::open(toWstring(index)));
	auto indexReader = searcher->getIndexReader();
	TermEnumPtr terms = indexReader->terms();
	uint64_t sumFreq = 0;
	while (terms->next()) {
		std::wcout << L"term=" << terms->term()->text() << L" freq=" << terms->docFreq() << std::endl;
		sumFreq += terms->docFreq();
	}
	terms->close();
	std::cout << "Sum freq=" << sumFreq << std::endl;
}

AnalyzerPtr get_analyzer() {
	static bool is_init = false;
	if (!is_init) {
		is_init = true;
		STOP_LIST = HashSet<String>::newInstance(_STOP_LIST, _STOP_LIST + SIZEOF_ARRAY(_STOP_LIST));
	}

	return newLucene<MyAnalyzer>(LuceneVersion::LUCENE_30, STOP_LIST);
}

IndexWriterPtr protectedAddDocument(IndexWriterPtr writer, DocumentPtr document, const std::string& index_path) {
	try {
		writer->addDocument(document, get_analyzer());
		return writer;
	} catch (std::bad_alloc&) {
		printf("Encountered bad alloc during document addition!\n");
		writer->close();
		auto newWriter = get_index_writer(index_path); 
		newWriter->addDocument(document, get_analyzer());
		return newWriter;
	}
}

void load_only(std::string path) {
	double total_time = 0;
	double cleanup_time = 0;
	TIMER_DECLARE(1);
	TIMER_DECLARE(2);
	{
		TIMER_BEGIN(1);
		IndexSearcherPtr searcher = newLucene<IndexSearcher>(FSDirectory::open(toWstring(path)));
		
		TIMER_END(1, total_time);
		printf("load time: %.3f\n", total_time / 1000000);
		fflush(stdout);
		
		TIMER_BEGIN(2);
	}
	TIMER_END(2, cleanup_time);
	printf("cleanup time: %.3f\n", cleanup_time / 1000000);
}

uint32_t getLastNumTokens() {
	return getStaticMySimilarity()->lastNumTokens;
}