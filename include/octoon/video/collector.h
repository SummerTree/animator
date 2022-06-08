#ifndef OCTOON_COLLECTOR_H_
#define OCTOON_COLLECTOR_H_

#include <set>
#include <functional>
#include <octoon/video/render_object.h>

namespace octoon
{
    class Bundle
    {
    public:
        virtual ~Bundle() = default;
    };

    class Iterator
    {
    public:
        Iterator() = default;
        virtual ~Iterator() = default;

        virtual bool IsValid() const = 0;
        virtual void Next() = 0;

        virtual RttiObject* Item() const = 0;
        virtual void Reset() = 0;

        template <typename T> typename T* ItemAs() const { return dynamic_cast<T*>(Item()); }

        Iterator(Iterator const&) = delete;
        Iterator& operator = (Iterator const&) = delete;
    };

    class Collector final
    {
    public:
        using ExpandFunc = std::function<std::set<RttiObject*>(RttiObject*)>;
        using ChangedFunc = std::function<bool(RttiObject*)>;
        using FinalizeFunc = std::function<void(RttiObject*)>;

        Collector();
        virtual ~Collector();

        void Clear();
        std::unique_ptr<Iterator> CreateIterator() const;
        void Collect(Iterator& iter, ExpandFunc expand_func);
        void Collect(RttiObject* object);
        void Collect(std::shared_ptr<RttiObject> object);

        void Commit();
        bool NeedsUpdate(Bundle const* bundle, ChangedFunc cahnged_func) const;
        std::size_t GetNumItems() const;
        Bundle* CreateBundle() const;
        std::uint32_t GetItemIndex(RttiObject* item) const;
        void Finalize(FinalizeFunc finalize_func);

        Collector(Collector const&) = delete;
        Collector& operator = (Collector const&) = delete;

    private:
        struct CollectorImpl;
        std::unique_ptr<CollectorImpl> impl;
    };
}

#endif