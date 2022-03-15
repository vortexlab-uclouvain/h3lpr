#ifndef H3LPR_SRC_PROF_HPP_
#define H3LPR_SRC_PROF_HPP_

// c headers
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// cpp headers
#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include "macros.hpp"

namespace H3LPR {

class TimerBlock {
   protected:
    int         count_    = 0;         //!< the number of times this block has been called
    size_t      memsize_  = 0;         //!< the memory size associated with a memory operation
    double      t0_       = -1.0;      //!< temp start time of the block
    double      t1_       = -1.0;      //!< temp stop time of the block
    double      time_acc_ = 0.0;       //!< accumulator to add the time accumulation
    std::string name_     = "noname";  //!< the default name of the block

    TimerBlock* parent_ = nullptr;  //!< the link to the parent blocks

    std::map<std::string, TimerBlock*> children_;  //!< the link to the children blocks (must be ordered to ensure correct MPI behavior)

   public:
    explicit TimerBlock(std::string name);
    ~TimerBlock();

    void Start();
    void Stop();

    std::string name() const { return name_; }
    TimerBlock* parent() const { return parent_; }
    double      time_acc() const;
    TimerBlock* AddChild(std::string child_name) noexcept;

    void SetParent(TimerBlock* parent);
    void Disp(FILE* file, const int level, const double totalTime, const int icol) const;
};

/**
 * @brief MPI time profiler
 *
 * @warning for the moment the chained list done for the profiler MUST be the same on every cpus
 * If you plan your profiler to have some fancy behavior, i.e. all the cpus not going though every timer,
 * you need to init the present + all the possible children before starting the timer of interest.
 * To do that, init and leave every prof call before starting the main one.
 */
class Prof {
   protected:
    std::map<std::string, TimerBlock*> time_map_;

    TimerBlock*       current_;  //!< this is a pointer to the last TimerBlock
    const std::string name_;

   public:
    explicit Prof();
    explicit Prof(const std::string myname);
    ~Prof();

    void Init(std::string name);
    void Start(std::string name);
    void Stop(std::string name);
    void Leave(std::string name);

    void Disp() const;
};

};  // namespace H3LPR

#define m_profInit(prof, name)                              \
    ({                                                      \
        H3LPR::Prof* m_profInit_prof_ = (H3LPR::Prof*)(prof); \
        std::string m_profInit_name_ = (std::string)(name); \
        if ((m_profInit_prof_) != nullptr) {                \
            (m_profInit_prof_)->Init(m_profInit_name_);     \
        }                                                   \
    })

#define m_profLeave(prof, name)                              \
    ({                                                       \
        H3LPR::Prof* m_profLeave_prof_ = (H3LPR::Prof*)(prof); \
        std::string m_profLeave_name_ = (std::string)(name); \
        if ((m_profLeave_prof_) != nullptr) {                \
            (m_profLeave_prof_)->Leave(m_profLeave_name_);   \
        }                                                    \
    })

#define m_profInitLeave(prof, name)                                \
    ({                                                             \
        H3LPR::Prof* m_profInitLeave_prof_ = (H3LPR::Prof*)(prof);   \
        std::string m_profInitLeave_name_ = (std::string)(name);   \
        if ((m_profInitLeave_prof_) != nullptr) {                  \
            (m_profInitLeave_prof_)->Init(m_profInitLeave_name_);  \
            (m_profInitLeave_prof_)->Leave(m_profInitLeave_name_); \
        }                                                          \
    })

#define m_profStart(prof, name)                              \
    ({                                                       \
        H3LPR::Prof* m_profStart_prof_ = (H3LPR::Prof*)(prof); \
        std::string m_profStart_name_ = (std::string)(name); \
        if ((m_profStart_prof_) != nullptr) {                \
            (m_profStart_prof_)->Init(m_profStart_name_);    \
            (m_profStart_prof_)->Start(m_profStart_name_);   \
        }                                                    \
    })

#define m_profStop(prof, name)                              \
    ({                                                      \
        H3LPR::Prof* m_profStop_prof_ = (H3LPR::Prof*)(prof); \
        std::string m_profStop_name_ = (std::string)(name); \
        if ((m_profStop_prof_) != nullptr) {                \
            (m_profStop_prof_)->Stop(m_profStop_name_);     \
            (m_profStop_prof_)->Leave(m_profStop_name_);    \
        }                                                   \
    })

#endif  // SRC_PROF_HPP_
