#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/timer/timer.hpp>

#include "common/god.h"
#include "common/logging.h"
#include "common/search.h"
#include "common/threadpool.h"
#include "common/printer.h"
#include "common/sentence.h"
#include "common/translation_task.h"
#include "common/exception.h"


int main(int argc, char* argv[]) {
  God::Init(argc, argv);
  std::setvbuf(stdout, NULL, _IONBF, 0);
  boost::timer::cpu_timer timer;

  std::string in;
  std::size_t taskCounter = 0;

  size_t cpuThreads = God::Get<size_t>("cpu-threads");
  LOG(info) << "Setting CPU thread count to " << cpuThreads;

  size_t totalThreads = cpuThreads;
#ifdef CUDA
  size_t gpuThreads = God::Get<size_t>("gpu-threads");
  auto devices = God::Get<std::vector<size_t>>("devices");
  LOG(info) << "Setting GPU thread count to " << gpuThreads;
  totalThreads += gpuThreads * devices.size();
#endif

  LOG(info) << "Total number of threads: " << totalThreads;
  UTIL_THROW_IF2(totalThreads == 0, "Total number of threads is 0");

  size_t maxBatchSize = God::Get<size_t>("batch-size");

  if (God::Get<bool>("wipo")) {
    LOG(info) << "Reading input";
    while (std::getline(God::GetInputStream(), in)) {
      Sentence *sentence = new Sentence(taskCounter, in);
      Sentences *sentences = new Sentences();;
      sentences->push_back(sentence);

      Histories result = TranslationTask(sentences, taskCounter);
      Printer(result, taskCounter++, std::cout);
    }
  } else {
    ThreadPool pool(totalThreads);
    LOG(info) << "Reading input";

    std::vector<std::future<Histories>> results;
    Sentences *sentences = new Sentences();;

    while(std::getline(God::GetInputStream(), in)) {
      Sentence *sentence = new Sentence(taskCounter, in);
      sentences->push_back(sentence);

      if (sentences->size() >= maxBatchSize) {
        results.emplace_back(
          pool.enqueue(
            [=]{ return TranslationTask(sentences, taskCounter); }
          )
        );

        sentences = new Sentences();;

        taskCounter++;
      }
    }

    results.emplace_back(
      pool.enqueue(
        [=]{ return TranslationTask(sentences, taskCounter); }
      )
    );

    size_t lineCounter = 0;
    for (auto&& result : results)
      Printer(result.get(), lineCounter++, std::cout);
  }
  LOG(info) << "Total time: " << timer.format();
  God::CleanUp();

  return 0;
}
