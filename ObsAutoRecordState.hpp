#ifndef OBSAUTORECORDSTATE_H
#define OBSAUTORECORDSTATE_H

enum class ObsAutoRecordState {
    CONNECTED,
    DISCONNECTED,
    PAUSED,
    WARNING
};

inline uint qHash(ObsAutoRecordState key, uint seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}

#endif
