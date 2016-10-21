#include "encoder.h"

using namespace std;

namespace GPU {

Encoder::Encoder(const Weights& model)
: embeddings_(model.encEmbeddings_),
  forwardRnn_(model.encForwardGRU_),
  backwardRnn_(model.encBackwardGRU_)
{}

void Encoder::GetContext(size_t sentInd, const std::vector<size_t>& words,
    EncoderDecoder::SourceContext& context) {

}

void Encoder::GetContextes(const Sentences& sentences, size_t tab,
    EncoderDecoder::SourceContextes& contextes) {
  embeddedSentencesFwd_.resize(sentences.size());
  embeddedSentencesBck_.resize(sentences.size());

  forwardRnn_.InitializeState(sentences.size());
  backwardRnn_.InitializeState(sentences.size());

  for (size_t sentInd = 0; sentInd < sentences.size(); ++sentInd) {
    const Sentence *sentence = sentences.at(sentInd);
    const std::vector<size_t>& words = sentence->GetWords(tab);
    size_t sentenceLen = words.size();

    EmbeddedSentence & embeddedSentenceFwd = embeddedSentencesFwd_[sentInd];
    embeddedSentenceFwd.resize(sentenceLen);

    EmbeddedSentence & embeddedSentenceBck = embeddedSentencesBck_[sentInd];
    embeddedSentenceBck.resize(sentenceLen);

    // looking embedding for each word in the sentence
    for (size_t i = 0; i < words.size(); ++i) {
      const Word &wordFwd = words[i];
      mblas::Matrix &mFwd = embeddedSentenceFwd[i];
      embeddings_.Lookup(mFwd, wordFwd);

      const Word &wordBck = words[words.size() - i - 1];
      mblas::Matrix &mBck = embeddedSentenceBck[i];
      embeddings_.Lookup(mBck, wordBck);

      //cerr << "mFwd=" << mFwd.GetShape().Debug() << endl;
      //cerr << "mBck=" << mBck.GetShape().Debug() << endl;
    }

    // calc rnn
    EncoderDecoder::SourceContext& context = contextes[sentInd];
    context->Resize(sentenceLen, forwardRnn_.GetStateLength() + backwardRnn_.GetStateLength());

    forwardRnn_.GetContext(sentInd, embeddedSentenceFwd.cbegin(),
        embeddedSentenceFwd.cend(),
               *context, false);

    backwardRnn_.GetContext(sentInd, embeddedSentenceBck.cbegin(),
        embeddedSentenceBck.cend(),
                *context, true);

  }
}

}
