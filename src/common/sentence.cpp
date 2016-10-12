#include <sstream>
#include "sentence.h"
#include "god.h"
#include "utils.h"
#include "common/vocab.h"

Sentence::Sentence(size_t lineNo, const std::string& line)
: lineNo_(lineNo), line_(line)
{
  std::vector<std::string> tabs;
  Split(line, tabs, "\t");
  size_t i = 0;
  for(auto&& tab : tabs) {
    std::vector<std::string> lineTokens;
    Trim(tab);
    Split(tab, lineTokens, " ");
    auto processed = God::Preprocess(i, lineTokens);
    words_.push_back(God::GetSourceVocab(i++)(processed));
  }
}

const Words& Sentence::GetWords(size_t index) const {
  return words_[index];
}

size_t Sentence::GetLine() const {
  return lineNo_;
}

std::string Sentence::Debug() const
{
  std::stringstream strm;

  for (size_t i = 0; i < words_.size(); ++i) {
    const Words &moreWords = words_[i];
    strm << " size=" << moreWords.size() << ": ";
    for (size_t j = 0; j < moreWords.size(); ++j) {
      const Word &word = moreWords[j];
      strm << word << " ";
    }

    strm << " |";
  }

  return strm.str();
}

/////////////////////////////////////////////////////////
Sentences::Sentences()
:maxLength_(0)
{

}

void Sentences::push_back(const Sentence *sentence) {
  const Words &words = sentence->GetWords(0);
  size_t len = words.size();
  if (len > maxLength_) {
    maxLength_ = len;
  }

  coll_.push_back(sentence);
}
