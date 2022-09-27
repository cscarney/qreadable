#pragma once

#ifdef LIBQREADABLE
#define QREADABLE_EXPORT __attribute__((visibility("default")))
#else
#define QREADABLE_EXPORT
#endif
