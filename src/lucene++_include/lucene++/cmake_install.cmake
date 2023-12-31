# Install script for directory: /home/asaf.levi/LucenePlusPlus/include/lucene++

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lucene++" TYPE FILE FILES
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MMapDirectory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorsTermsWriterPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FreqProxTermsWriterPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldInfos.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldCacheRangeFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NormsWriterPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CompoundFileWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ThreadPool.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/WhitespaceTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexFileNames.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StoredFieldsWriterPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DoubleFieldSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ComplexExplanation.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReadOnlyDirectoryReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Weight.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PerFieldAnalyzerWrapper.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ByteFieldSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MergeScheduler.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldProcessorPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldScoreQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiTermQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TestPoint.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DisjunctionMaxScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FastCharStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermBuffer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FileUtils.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InvertedDocEndConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldCacheSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiFieldQueryParser.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharFolder.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SimpleLRUCache.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryParserTokenManager.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneHeaders.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiTermQueryWrapperFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexDeletionPolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SloppyPhraseScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PositionBasedTermVectorMapper.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NumericRangeFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SmallDouble.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexOutput.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsDocsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FreqProxTermsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InfoStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FilteredTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsFieldsConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BufferedDeletes.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/RAMDirectory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorsReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LengthFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Similarity.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/HitQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentWriteState.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanNotQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CustomScoreProvider.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorEntry.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PhraseQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermPositionVector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MaxPayloadFunction.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InvertedDocEndConsumerPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CachingSpanFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Tokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IntBlockPool.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermDocs.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocValues.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/AllTermDocs.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StandardTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SingleInstanceLockFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermsHashPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FilterManager.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Lucene.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SingleTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NumericUtils.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DirectoryReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharBlockPool.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryParseError.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DisjunctionMaxQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocIdBitSet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermsHashConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/WildcardTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StringReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Filter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Explanation.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LowerCaseTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexCommit.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermInfosWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FilteredDocIdSet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Document.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermsHashConsumerPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Directory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BitSet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultipleTermPositions.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BufferedIndexOutput.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NormsWriterPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharArraySet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocInverter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CompoundFileReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneTypes.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ASCIIFoldingFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/HashSet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiSearcher.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsFieldsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CheckIndex.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermsHashPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentMergeQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Collector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NumericField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TopFieldDocs.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermRangeTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IntFieldSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldProcessorPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StopAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/OpenBitSet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorMapper.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/RAMInputStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SetBasedFieldSelector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermInfosReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanNearQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ScorerDocQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReusableStringReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryWrapperFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldSortedTermVectorMapper.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NearSpansUnordered.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SortedTermVectorMapper.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ChecksumIndexOutput.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Set.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Analyzer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DateField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MapFieldSelector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/HitQueueBase.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanFirstQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CycleCheck.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BufferedIndexInput.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocInverterPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FuzzyTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StopFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/UnicodeUtils.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermRangeQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PorterStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BaseCharFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReqExclScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorOffsetInfo.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ConcurrentMergeScheduler.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocConsumerPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InvertedDocConsumerPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/KeepOnlyLastCommitDeletionPolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FileReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldConsumerPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsTermsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DefaultSimilarity.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InvertedDocConsumerPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ISOLatin1AccentFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldConsumersPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReqOptSumScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/OpenBitSetIterator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanWeight.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DisjunctionSumScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldMaskingSpanQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TokenFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/AveragePayloadFunction.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ConjunctionScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NumberTools.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FSDirectory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiLevelSkipListReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CachingWrapperFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexFileDeleter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FuzzyQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CompressionTools.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermPositions.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BitUtil.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BooleanScorer2.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InputStreamReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TokenStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldCacheSanityChecker.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocumentsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldProcessor.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/KeywordTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldComparatorSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TimeLimitingCollector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ParallelReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ByteBlockPool.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Field.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReaderUtil.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SimpleFSDirectory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldValueHitQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Spans.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Map.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Term.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneSync.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TeeSinkTokenFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DefaultSkipListWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LetterTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ConstantScoreQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/RAMOutputStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TopFieldCollector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PayloadAttribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Fieldable.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/OffsetAttribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PorterStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Sort.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldDocSortedHitQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LogMergePolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BooleanClause.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FlagsAttribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorsTermsWriterPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentTermDocs.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ParallelMultiSearcher.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PrefixQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SortedVIntList.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentMerger.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ByteSliceReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BufferedReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SnapshotDeletionPolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Constants.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldConsumerPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MatchAllDocsQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryParser.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ExactPhraseScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ValueSourceQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/WildcardQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FilteredQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MergeDocIDRemapper.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldInvertState.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/RAMFile.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NumericRangeQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CloseableThreadLocal.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PayloadNearQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Synchronize.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldsReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PayloadFunction.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/AttributeSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CharStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneSignal.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NormalizeCharMap.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ChecksumIndexInput.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentInfoCollection.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsPositionsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TopScoreDocCollector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermFreqVector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryParserToken.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TypeAttribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StringUtils.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Collection.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SimilarityDelegator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexSearcher.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocumentsWriterThreadState.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PositionIncrementAttribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NoLockFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexInput.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiPhraseQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Attribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FreqProxTermsWriterPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LoadFirstFieldSelector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NormsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/HashMap.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LockFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Payload.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/KeywordAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PayloadSpanUtil.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SortField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SimpleAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryParserConstants.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocIdSet.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldInfo.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentTermVector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldSelector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MapOfSets.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ScoreCachingWrappingScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Array.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PrefixFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LowerCaseFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanOrQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentInfos.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Base64.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InvertedDocEndConsumerPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MinPayloadFunction.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentMergeInfo.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/AbstractAllTermDocs.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FileSwitchDirectory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/VariantUtils.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LogDocMergePolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocIdSetIterator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermInfo.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Token.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Collator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Lock.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NativeFSLockFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/OpenBitSetDISI.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FilteredDocIdSetIterator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneAllocator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentTermPositionVector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ValueSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Random.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReverseOrdFieldSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsDocsConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermsHash.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldCacheTermsFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StoredFieldsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Query.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PhraseQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StandardFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermSpans.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TopDocsCollector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FilterIndexReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DateTools.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryParserCharStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentTermPositions.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/targetver.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorEntryFreqSortedComparator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PhraseScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ByteSliceWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StandardAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Searcher.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/InvertedDocConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentInfo.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CachingTokenFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MiscUtils.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NumericTokenStream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/AbstractField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BooleanQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TopDocs.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldDoc.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PhrasePositions.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermsHashConsumerPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermVectorsTermsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ScoreDoc.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/ReadOnlySegmentReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Searchable.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BitVector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SegmentReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneObject.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocInverterPerThread.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/UTF8Stream.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/IndexFileNameFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FSLockFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PositiveScoresOnlyCollector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/CustomScoreQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/WordlistLoader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermAttribute.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MultiLevelSkipListWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/RawPostingList.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanTermQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SimpleFSLockFactory.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldCacheImpl.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MappingCharFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldsWriter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsTermsConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FormatPostingsPositionsConsumer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/OrdFieldSource.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/QueryTermVector.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldConsumersPerField.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/StandardTokenizerImpl.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DocFieldConsumers.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanQueryFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PriorityQueue.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/TermRangeFilter.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldCache.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/DefaultSkipListReader.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LuceneException.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/BooleanScorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/LogByteSizeMergePolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FieldComparator.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/WhitespaceAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SerialMergeScheduler.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/MergePolicy.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PrefixTermEnum.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/SpanFilterResult.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/PayloadTermQuery.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/FreqProxFieldMergeState.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Scorer.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/NearSpansOrdered.h"
    "/home/asaf.levi/LucenePlusPlus/include/lucene++/Reader.h"
    )
endif()

