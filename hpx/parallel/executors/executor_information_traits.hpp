//  Copyright (c) 2007-2015 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file parallel/executors/executor_information_traits.hpp

#if !defined(HPX_PARALLEL_EXECUTOR_INFORMATION_TRAITS_AUG_26_2015_1133AM)
#define HPX_PARALLEL_EXECUTOR_INFORMATION_TRAITS_AUG_26_2015_1133AM

#include <hpx/config.hpp>
#include <hpx/exception.hpp>
#include <hpx/exception_list.hpp>
#include <hpx/async.hpp>
#include <hpx/traits/is_executor.hpp>
#include <hpx/util/decay.hpp>
#include <hpx/util/always_void.hpp>
#include <hpx/util/result_of.hpp>
#include <hpx/util/deferred_call.hpp>
#include <hpx/util/unwrapped.hpp>
#include <hpx/parallel/config/inline_namespace.hpp>
#include <hpx/parallel/executors/executor_traits.hpp>

#include <type_traits>
#include <utility>

namespace hpx { namespace parallel { HPX_INLINE_NAMESPACE(v3)
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        /// \cond NOINTERNAL
        struct os_thread_count_helper
        {
            template <typename Executor>
            static std::size_t call(wrap_int, Executor const& exec)
            {
                return hpx::get_os_thread_count();
            }

            template <typename Executor>
            static auto call(int, Executor const& exec)
            ->  decltype(exec.os_thread_count())
            {
                return exec.os_thread_count();
            }
        };

        template <typename Executor>
        std::size_t call_os_thread_count(Executor const& exec)
        {
            return os_thread_count_helper::call(0, exec);
        }

        ///////////////////////////////////////////////////////////////////////
        struct has_pending_closures_helper
        {
            template <typename Executor>
            static auto call(wrap_int, Executor const& exec) -> bool
            {
                return false;   // assume stateless scheduling
            }

            template <typename Executor>
            static auto call(int, Executor const& exec)
                ->  decltype(exec.has_pending_closures())
            {
                return exec.has_pending_closures();
            }
        };

        template <typename Executor>
        bool call_has_pending_closures(Executor const& exec)
        {
            return has_pending_closures_helper::call(0, exec);
        }

        ///////////////////////////////////////////////////////////////////////
        struct get_pu_mask_helper
        {
            template <typename Executor>
            static threads::mask_cref_type call(wrap_int, Executor const& exec,
                threads::topology& topo, std::size_t thread_num)
            {
            }

            template <typename Executor>
            static auto call(int, Executor const& exec,
                    threads::topology& topo, std::size_t thread_num)
            ->  decltype(exec.get_pu_mask(topo, thread_num))
            {
                return exec.get_pu_mask(topo, thread_num);
            }
        };

        template <typename Executor>
        threads::mask_cref_type call_get_pu_mask(Executor const& exec,
            threads::topology& topo, std::size_t thread_num)
        {
            return get_pu_mask_helper::call(0, exec, topo, thread_num);
        }
        /// \endcond
    }

    ///////////////////////////////////////////////////////////////////////////
    /// The executor_information_traits type is used to various pieces of
    /// information from an executor.
    ///
    template <typename Executor, typename Enable>
    struct executor_information_traits
    {
        /// The type of the executor associated with this instance of
        /// \a executor_traits
        typedef Executor executor_type;

        /// Retrieve the number of (kernel-)threads used by the associated
        /// executor.
        ///
        /// \param exec  [in] The executor object to use for scheduling of the
        ///              function \a f.
        ///
        /// \note This calls exec.os_thread_count() if it exists;
        ///       otherwise it executes hpx::get_os_thread_count().
        ///
        static std::size_t os_thread_count(executor_type const& exec)
        {
            return detail::call_os_thread_count(exec);
        }

        /// Retrieve whether this executor has operations pending or not.
        ///
        /// \param exec  [in] The executor object to use for scheduling of the
        ///              function \a f.
        ///
        /// \note If the executor does not expose this information, this call
        ///       will always return \a false
        ///
        static bool has_pending_closures(executor_type const& exec)
        {
            return detail::call_has_pending_closures(exec);
        }

        /// Retrieve the bitmask describing the processing units the given
        /// thread is allowed to run on
        /// All threads::executors invoke sched.get_pu_mask().
        ///
        /// \param sched  [in] The executor object to use for querying the
        ///               number of pending tasks.
        ///
        static threads::mask_type get_pu_mask(executor_type const& sched,
            threads::topology& topo, std::size_t thread_num)
        {
            return detail::call_get_pu_mask(exec, topo, thread_num);
        }
    };
}}}

#undef HPX_ENABLE_WORKAROUND_FOR_GCC46

#endif
