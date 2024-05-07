#pragma once

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"

namespace ranges {

    template <typename It, typename Ptr = void>
    class Range {
    public:
        using ValueType = typename std::iterator_traits<It>::value_type;

        Range(It begin, It end, const Ptr* ptr = nullptr)
            : begin_(begin)
            , end_(end)
            , ptr_(ptr) {
        }

        It begin() const {
            return begin_;
        }

        It end() const {
            return end_;
        }

        const Ptr* GetPtr() const {
            return ptr_;
        }

    private:
        It begin_;
        It end_;
        const Ptr* ptr_ = nullptr;
    };

    template <typename C, typename Ptr = void>
    auto AsRange(const C& container) {
        return Range{ container.begin(), container.end() };
    }


    template <typename It>
    class BusRange : public Range<It, transport_catalogue::domain::Bus> {
    public:
        explicit BusRange(It begin, It end, const transport_catalogue::domain::Bus* ptr)
            : Range<It, transport_catalogue::domain::Bus>(begin, end, ptr) {
        }
    };

    inline auto AsBusRangeDirect(transport_catalogue::domain::Bus* bus) {
        return BusRange{ bus->stops_.begin(), bus->stops_.end(), bus };
    }

    inline auto AsBusRangeReversed(transport_catalogue::domain::Bus* bus) {
        return BusRange{ bus->stops_.rbegin(), bus->stops_.rend(), bus };
    }

}  // namespace ranges