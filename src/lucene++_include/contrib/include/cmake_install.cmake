# Install script for directory: /home/asaf.levi/LucenePlusPlus/src/contrib/include

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
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/DutchStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/Fragmenter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SnowballFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/QueryTermExtractor.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/BrazilianStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SimpleHTMLFormatter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SpanGradientFormatter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/RussianStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/RussianLetterTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SnowballAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/GreekAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/GreekLowerCaseFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/WeightedSpanTermExtractor.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ElisionFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/TextFragment.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/TokenSources.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ContribInc.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/PersianAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SimpleFragmenter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/CzechAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/PersianNormalizationFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/CJKAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/DefaultEncoder.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/BrazilianStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/WeightedSpanTerm.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/FrenchAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/TokenGroup.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/FrenchStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/HighlighterScorer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/DutchStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/GermanStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/BrazilianAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/MapWeightedSpanTerm.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ArabicNormalizationFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/DutchAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/FrenchStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/MemoryIndex.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/RussianLowerCaseFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ChineseAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ArabicStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ArabicAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SimpleHTMLEncoder.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/CJKTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ArabicLetterTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/QueryTermScorer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/QueryScorer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/GradientFormatter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ArabicNormalizer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ArabicStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/NullFragmenter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/SimpleSpanFragmenter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/LuceneContrib.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/GermanStemmer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/GermanAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/Encoder.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ReverseStringFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ChineseTokenizer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/RussianStemFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/PersianNormalizer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/Formatter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/WeightedTerm.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/RussianAnalyzer.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/ChineseFilter.h"
    "/home/asaf.levi/LucenePlusPlus/src/contrib/include/Highlighter.h"
    )
endif()

