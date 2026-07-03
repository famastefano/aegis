#include <aegis/allocators/pool_allocator.h>

#include <etw/events/extended_data.h>

#include <limits>

namespace aegis::sensor::win32::etw
{
ExtendedData::ExtendedData(EVENT_HEADER_EXTENDED_DATA_ITEM const &item)
{
    if (item.ExtType == EVENT_HEADER_EXT_TYPE_RELATED_ACTIVITYID)
    {
        type_                 = Type::RELATED_ACTIVITY_ID;
        data_.rel_activity_id = *reinterpret_cast<EVENT_EXTENDED_ITEM_RELATED_ACTIVITYID *>(item.DataPtr);
    }
    else if (item.ExtType == EVENT_HEADER_EXT_TYPE_SID)
    {
        type_ = Type::NONE;
    }
    else if (item.ExtType == EVENT_HEADER_EXT_TYPE_TS_ID)
    {
        type_       = Type::TS_ID;
        data_.ts_id = *reinterpret_cast<EVENT_EXTENDED_ITEM_TS_ID *>(item.DataPtr);
    }
    else if (item.ExtType == EVENT_HEADER_EXT_TYPE_INSTANCE_INFO)
    {
        type_       = Type::TS_ID;
        data_.ts_id = *reinterpret_cast<EVENT_EXTENDED_ITEM_TS_ID *>(item.DataPtr);
    }
    else if (item.ExtType == EVENT_HEADER_EXT_TYPE_EVENT_KEY)
    {
        type_           = Type::EVENT_KEY;
        data_.event_key = *reinterpret_cast<EVENT_EXTENDED_ITEM_EVENT_KEY *>(item.DataPtr);
    }
    else if (item.ExtType == EVENT_HEADER_EXT_TYPE_PROCESS_START_KEY)
    {
        type_             = Type::PROCESS_START_KEY;
        data_.process_key = *reinterpret_cast<EVENT_EXTENDED_ITEM_PROCESS_START_KEY *>(item.DataPtr);
    }
    else
    {
        type_ = Type::NONE;
    }
}
} // namespace aegis::sensor::win32::etw