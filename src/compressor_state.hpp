#ifndef COMPRESSOR_STATE_HPP
#define COMPRESSOR_STATE_HPP

#include "compressor.hpp"
#include "compressor_thread.hpp"
#include "threading.hpp"

namespace fcmp
{
  /// Convenience typedef.
  typedef std::vector<CompressorThread*> CompressorThreadVector;

  /// Compression state.
  class CompressorState
  {
    private:
      /// Mutex guarding multi-threaded compression.
      std::mutex m_mutex;

      /// Condition value for signalling.
      std::condition_variable m_cond;

      /// Input data.
      const DataBits *m_data;

      /// Base compressor.
      CompressorSptr m_compressor;

      /// Next compressor.
      CompressorSptr m_next_compressor;

      /// Best data so far.
      DataCompressedSptr m_best_data;

      /// Compressor threads.
      std::vector<CompressorThreadSptr> m_threads;

      /// Currently active threads.
      CompressorThreadVector m_threads_active;

      /// Currently dormant threads.
      CompressorThreadVector m_threads_dormant;

    public:
      /// Constructor.
      ///
      /// \param data Data to compress.
      /// \param threads Number of threads to use, 0 for hardware concurrency.
      CompressorState(const DataBits *data, unsigned threads = 0);

    public:
      /// Get best data.
      ///
      /// \return Best data.
      DataCompressedSptr getBestData() const
      {
        return m_best_data;
      }

      /// Accessor.
      ///
      /// \return Mutex.
      std::mutex& getMutex()
      {
        return m_mutex;
      }
      /// Accessor.
      ///
      /// \return Mutex.
      const std::mutex& getMutex() const
      {
        return m_mutex;
      }

      /// Mutate the main compressor.
      ///
      /// \param context Context to mutate with.
      /// \param weight Weight to mutate with.
      /// \return New compressor, may be empty if compressor did not change.
      CompressorSptr mutate(uint8_t context, uint8_t weight) const
      {
        return m_compressor->mutate(context, weight);
      }

    private:
      /// Move a thread from dormant state into active state, then wake it up.
      ///
      /// Mutex must be locked by calling thread.
      ///
      /// \param thr Thread to move.
      /// \param context Context to use for next job.
      /// \param weight Weight to use for next job.
      /// \param size_limit Cancel compression if size limit is reached.
      void awaken(CompressorThread *thr, uint8_t context, uint8_t weight, size_t size_limit);

      /// Rotate to next cycle. If no improvements have happened, do nothing.
      ///
      /// \return True if improvements have happened.
      bool cycle();

    public:
      /// Perform a compress cycle.
      ///
      /// \return True if cycle improved compression, false if not.
      bool compressCycle();

      /// Update the current 'best' compressor and data state if necessary.
      ///
      /// Mutex must be locked by calling thread.
      ///
      /// \param compressor Compressor attempt.
      /// \param data Data attempt.
      void update(CompressorSptr compressor, DataCompressedSptr data);

      /// Move a thread from active state into dormant state.
      ///
      /// \param thr Thread going dormant.
      /// \param sl Scoped lock to free for sleeping.
      void wait(CompressorThread *thr, ScopedLock &sl);

      /// Register a thread into this state as a dormant thread.
      ///
      /// \param thr Thread going dormant.
      /// \param sl Scoped lock to free for sleeping.
      void waitInitial(CompressorThread *thr, ScopedLock &sl);

  };
}

#endif
