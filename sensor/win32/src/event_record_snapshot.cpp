#include <etw/event_record_snapshot.h>

#include <cstring>
#include <flat_map>
#include <limits>

namespace aegis::sensor::win32::etw
{
//static_assert(std::numeric_limits<decltype(ExtendedDataSnapshot::type)>::max() >= EVENT_HEADER_EXT_TYPE_MAX);

namespace
{
using ext_t               = EVENT_HEADER_EXTENDED_DATA_ITEM;
using ext_data_migrator_t = void (*)(ext_t const &src, ext_t &dst);

void copy_event_key(ext_t const &src, ext_t &dst)
{
    auto const &data = *reinterpret_cast<EVENT_EXTENDED_ITEM_EVENT_KEY *>(src.DataPtr);
    dst.DataPtr      = data.Key;
}

void copy_process_key(ext_t const &src, ext_t &dst)
{
    auto const &data = *reinterpret_cast<EVENT_EXTENDED_ITEM_PROCESS_START_KEY *>(src.DataPtr);
    dst.DataPtr      = data.ProcessStartKey;
}

void copy_session_id(ext_t const &src, ext_t &dst)
{
    auto const &data = *reinterpret_cast<EVENT_EXTENDED_ITEM_TS_ID *>(src.DataPtr);
    dst.DataPtr      = data.SessionId;
}

auto build_migrator_map()
{
    std::flat_map<USHORT, ext_data_migrator_t> map;
    map[EVENT_HEADER_EXT_TYPE_EVENT_KEY]         = copy_event_key;
    map[EVENT_HEADER_EXT_TYPE_PROCESS_START_KEY] = copy_process_key;
    map[EVENT_HEADER_EXT_TYPE_TS_ID]             = copy_session_id;
    return map;
}

ext_data_migrator_t get_ext_data_migrator(USHORT type)
{
    static auto map = build_migrator_map();
    if (auto it = map.find(type); it != map.end())
        return it->second;

    return +[](ext_t const &, ext_t &dst) {
        // Assume data internal to ETW
        dst.DataSize = 0;
        dst.DataPtr  = 0;
        dst.ExtType  = 0;
    };
}
} // namespace

EventRecordSnapshot make_event_record_snapshot(const EVENT_RECORD &record) noexcept
{
    /*EventRecordSnapshot snapshot{.ev = record};
    snapshot.ev.UserContext = nullptr;
    if (record.UserDataLength > 0)
    {
        snapshot.ev.UserData = new unsigned char[record.UserDataLength];
        std::memcpy(snapshot.ev.UserData, record.UserData, record.UserDataLength);
    }
    if (record.ExtendedDataCount > 0)
    {
        snapshot.ev.ExtendedData = new ext_t[record.ExtendedDataCount];
        for (USHORT i = 0; i < record.ExtendedDataCount; ++i)
        {
            auto const &srcData = record.ExtendedData[i];
            auto       &dstData = snapshot.ev.ExtendedData[i];
            dstData             = srcData;
            get_ext_data_migrator(srcData.ExtType)(srcData, dstData);
        }
    }
    return snapshot;*/
    return {};
}

} // namespace aegis::sensor::win32::etw
