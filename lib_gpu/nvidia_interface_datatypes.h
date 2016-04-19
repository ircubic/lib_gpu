#pragma once
#include "helpers.h"
#include <basetsd.h>
#include <iomanip>
#include <ostream>

namespace lib_gpu {

#define NVIDIA_STRUCT_VERSION(_struct, _version) (_version<<16 | sizeof(_struct))
#define NVIDIA_STRUCT_BEGIN_EX(_name, _version, _flags) struct NVLIB_EXPORTED _name {\
	UINT32 version = NVIDIA_STRUCT_VERSION(_name, _version);\
	UINT32 _flags; 
#define NVIDIA_STRUCT_BEGIN(_name, _version) NVIDIA_STRUCT_BEGIN_EX(_name, _version, flags)
#define NVIDIA_STRUCT_END };

typedef int* NV_HANDLE;
typedef NV_HANDLE NV_PHYSICAL_GPU_HANDLE;
typedef NV_HANDLE NV_LOGICAL_GPU_HANDLE;
typedef NV_HANDLE NV_UNATTACHED_DISPLAY_HANDLE;
typedef NV_HANDLE NV_DISPLAY_HANDLE;

#define NVIDIA_SHORT_STRING_SIZE 64

typedef enum
{
    NVAPI_OK = 0,
    NVAPI_ERROR = -1,
    NVAPI_LIBRARY_NOT_FOUND = -2,
    NVAPI_NO_IMPLEMENTATION = -3,
    NVAPI_API_NOT_INITIALIZED = -4,
    NVAPI_INVALID_ARGUMENT = -5,
    NVAPI_NVIDIA_DEVICE_NOT_FOUND = -6,
    NVAPI_END_ENUMERATION = -7,
    NVAPI_INVALID_HANDLE = -8,
    NVAPI_INCOMPATIBLE_STRUCT_VERSION = -9,
    NVAPI_HANDLE_INVALIDATED = -10,
    NVAPI_OPENGL_CONTEXT_NOT_CURRENT = -11,
    NVAPI_INVALID_POINTER = -14,
    NVAPI_NO_GL_EXPERT = -12,
    NVAPI_INSTRUMENTATION_DISABLED = -13,
    NVAPI_NO_GL_NSIGHT = -15,
    NVAPI_EXPECTED_LOGICAL_GPU_HANDLE = -100,
    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE = -101,
    NVAPI_EXPECTED_DISPLAY_HANDLE = -102,
    NVAPI_INVALID_COMBINATION = -103,
    NVAPI_NOT_SUPPORTED = -104,
    NVAPI_PORTID_NOT_FOUND = -105,
    NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE = -106,
    NVAPI_INVALID_PERF_LEVEL = -107,
    NVAPI_DEVICE_BUSY = -108,
    NVAPI_NV_PERSIST_FILE_NOT_FOUND = -109,
    NVAPI_PERSIST_DATA_NOT_FOUND = -110,
    NVAPI_EXPECTED_TV_DISPLAY = -111,
    NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR = -112,
    NVAPI_NO_ACTIVE_SLI_TOPOLOGY = -113,
    NVAPI_SLI_RENDERING_MODE_NOTALLOWED = -114,
    NVAPI_EXPECTED_DIGITAL_FLAT_PANEL = -115,
    NVAPI_ARGUMENT_EXCEED_MAX_SIZE = -116,
    NVAPI_DEVICE_SWITCHING_NOT_ALLOWED = -117,
    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED = -118,
    NVAPI_UNKNOWN_UNDERSCAN_CONFIG = -119,
    NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO = -120,
    NVAPI_DATA_NOT_FOUND = -121,
    NVAPI_EXPECTED_ANALOG_DISPLAY = -122,
    NVAPI_NO_VIDLINK = -123,
    NVAPI_REQUIRES_REBOOT = -124,
    NVAPI_INVALID_HYBRID_MODE = -125,
    NVAPI_MIXED_TARGET_TYPES = -126,
    NVAPI_SYSWOW64_NOT_SUPPORTED = -127,
    NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,
    NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,
    NVAPI_OUT_OF_MEMORY = -130,
    NVAPI_WAS_STILL_DRAWING = -131,
    NVAPI_FILE_NOT_FOUND = -132,
    NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS = -133,
    NVAPI_INVALID_CALL = -134,
    NVAPI_D3D10_1_LIBRARY_NOT_FOUND = -135,
    NVNVLIB_EXPORTEDTION_NOT_FOUND = -136,
    NVAPI_INVALID_USER_PRIVILEGE = -137,
    NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE = -138,
    NVAPI_EXPECTED_COMPUTE_GPU_HANDLE = -139,
    NVAPI_STEREO_NOT_INITIALIZED = -140,
    NVAPI_STEREO_REGISTRY_ACCESS_FAILED = -141,
    NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142,
    NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED = -143,
    NVAPI_STEREO_NOT_ENABLED = -144,
    NVAPI_STEREO_NOT_TURNED_ON = -145,
    NVAPI_STEREO_INVALID_DEVICE_INTERFACE = -146,
    NVAPI_STEREO_PARAMETER_OUT_OF_RANGE = -147,
    NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148,
    NVAPI_TOPO_NOT_POSSIBLE = -149,
    NVAPI_MODE_CHANGE_FAILED = -150,
    NVAPI_D3D11_LIBRARY_NOT_FOUND = -151,
    NVAPI_INVALID_ADDRESS = -152,
    NVAPI_STRING_TOO_SMALL = -153,
    NVAPI_MATCHING_DEVICE_NOT_FOUND = -154,
    NVAPI_DRIVER_RUNNING = -155,
    NVAPI_DRIVER_NOTRUNNING = -156,
    NVAPI_ERROR_DRIVER_RELOAD_REQUIRED = -157,
    NVAPI_SET_NOT_ALLOWED = -158,
    NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED = -159,
    NVAPI_SETTING_NOT_FOUND = -160,
    NVAPI_SETTING_SIZE_TOO_LARGE = -161,
    NVAPI_TOO_MANY_SETTINGS_IN_PROFILE = -162,
    NVAPI_PROFILE_NOT_FOUND = -163,
    NVAPI_PROFILE_NAME_IN_USE = -164,
    NVAPI_PROFILE_NAME_EMPTY = -165,
    NVAPI_EXECUTABLE_NOT_FOUND = -166,
    NVAPI_EXECUTABLE_ALREADY_IN_USE = -167,
    NVAPI_DATATYPE_MISMATCH = -168,
    NVAPI_PROFILE_REMOVED = -169,
    NVAPI_UNREGISTERED_RESOURCE = -170,
    NVAPI_ID_OUT_OF_RANGE = -171,
    NVAPI_DISPLAYCONFIG_VALIDATION_FAILED = -172,
    NVAPI_DPMST_CHANGED = -173,
    NVAPI_INSUFFICIENT_BUFFER = -174,
    NVAPI_ACCESS_DENIED = -175,
    NVAPI_MOSAIC_NOT_ACTIVE = -176,
    NVAPI_SHARE_RESOURCE_RELOCATED = -177,
    NVAPI_REQUEST_USER_TO_DISABLE_DWM = -178,
    NVAPI_D3D_DEVICE_LOST = -179,
    NVAPI_INVALID_CONFIGURATION = -180,
    NVAPI_STEREO_HANDSHAKE_NOT_DONE = -181,
    NVAPI_EXECUTABLE_PATH_IS_AMBIGUOUS = -182,
    NVAPI_DEFAULT_STEREO_PROFILE_IS_NOT_DEFINED = -183,
    NVAPI_DEFAULT_STEREO_PROFILE_DOES_NOT_EXIST = -184,
    NVAPI_CLUSTER_ALREADY_EXISTS = -185,
    NVAPI_DPMST_DISPLAY_ID_EXPECTED = -186,
    NVAPI_INVALID_DISPLAY_ID = -187,
    NVAPI_STREAM_IS_OUT_OF_SYNC = -188,
    NVAPI_INCOMPATIBLE_AUDIO_DRIVER = -189,
    NVAPI_VALUE_ALREADY_SET = -190,
    NVAPI_TIMEOUT = -191,
    NVAPI_GPU_WORKSTATION_FEATURE_INCOMPLETE = -192,
    NVAPI_STEREO_INIT_ACTIVATION_NOT_DONE = -193,
    NVAPI_SYNC_NOT_ACTIVE = -194,
    NVAPI_SYNC_MASTER_NOT_FOUND = -195,
    NVAPI_INVALID_SYNC_TOPOLOGY = -196,
    NVAPI_ECID_SIGN_ALGO_UNSUPPORTED = -197,
    NVAPI_ECID_KEY_VERIFICATION_FAILED = -198,
} NV_STATUS;

typedef enum
{
    NVIDIA_CLOCK_SYSTEM_GPU = 0,
    NVIDIA_CLOCK_SYSTEM_MEMORY = 4,
    NVIDIA_CLOCK_SYSTEM_SHADER = 7
} NVIDIA_CLOCK_SYSTEM;

/**
* The known subsystems one can get utilization for.
*/
typedef enum
{
    NVIDIA_DYNAMIC_PSTATES_SYSTEM_GPU = 0,
    NVIDIA_DYNAMIC_PSTATES_SYSTEM_FB = 1,
    NVIDIA_DYNAMIC_PSTATES_SYSTEM_VID = 2,
    NVIDIA_DYNAMIC_PSTATES_SYSTEM_BUS = 3
} NVIDIA_DYNAMIC_PSTATES_SYSTEM;

#pragma region Structs

struct NVIDIA_DELTA_ENTRY
{
    INT32 value;
    INT32 val_min;
    INT32 val_max;
};

NVIDIA_STRUCT_BEGIN(NVIDIA_GPU_PSTATES20_V2, 2)
UINT32 state_count;
UINT32 clock_count;
UINT32 voltage_count;
struct
{
    UINT32 state_num;
    UINT32 flags;
    struct
    {
        UINT32 domain;
        /**
         * Whether this clock is single- or dynamic-frequency.
         *
         * 0 indicates single frequency, 1 indicates dynamic.
         *
         * If this clock is single frequency, the `max_freq`, `voltage_domain`,
         * `min_volt` and `max_volt` values are invalid, and
         * `min_or_single_freq` contains the static frequency.
         */
        UINT32 type;
        UINT32 flags;
        NVIDIA_DELTA_ENTRY freq_delta;
        UINT32 min_or_single_freq;
        UINT32 max_freq;
        UINT32 voltage_domain;
        UINT32 min_volt;
        UINT32 max_volt;
    } clocks[8];

    /**
     * Base voltage for all the available voltage domains.
     *
     * The base voltage is the voltage at which the GPU will rest when it's in
     * a given powerstate. The array contains `voltage_count` entries.
     */
    struct
    {
/**
 * The voltage domain being described.
 *
 * This value is referenced by one or more `clocks` entries in their
 * `voltage_domain` attributes.
 */
        UINT32 domain;
        /**
         * The base voltage delta can be edited if bit 0 of the flags is set.
         */
        UINT32 flags;
        /**
         * The current base voltage value.
         */
        UINT32 voltage;
        NVIDIA_DELTA_ENTRY volt_delta;
    } base_voltages[4];
} states[16];
struct
{
    UINT32 voltage_count;
    struct
    {
        UINT32 domain;
        UINT32 flags;
        UINT32 voltage;
        NVIDIA_DELTA_ENTRY volt_delta;
    } voltages[4];
} over_volt;
NVIDIA_STRUCT_END

typedef enum
{
    NVIDIA_CLOCK_FREQUENCY_TYPE_CURRENT = 0,
    NVIDIA_CLOCK_FREQUENCY_TYPE_BASE = 1,
    NVIDIA_CLOCK_FREQUENCY_TYPE_BOOST = 2,
    NVIDIA_CLOCK_FREQUENCY_TYPE_LAST
} NVIDIA_CLOCK_FREQUENCY_TYPE;

/**
* The value for the clock frequencies for all available clocks.
*
* Indexed by NVIDIA_CLOCK_SYSTEM enum.
*/
NVIDIA_STRUCT_BEGIN_EX(NVIDIA_CLOCK_FREQUENCIES, 2, clock_type)
struct
{
    UINT32 present;
    UINT32 freq;
} entries[32];
NVIDIA_STRUCT_END

NVIDIA_STRUCT_BEGIN_EX(NVIDIA_GPU_PERF_TABLE, 1, plevel_count)
UINT32 field1;
UINT32 domain_entries;
UINT32 field3;
UINT32 p_state_level;
UINT32 field5;
struct
{
    struct
    {
        UINT32 domain;
        UINT32 unknown1;
        UINT32 clock;
        UINT32 defaultClock;
        UINT32 minClock;
        UINT32 maxClock;
        UINT32 unknown2;
    }domains[32];
    UINT32 unknown1;
    UINT32 possiblySettingFlags;
}entries[10];
UINT32 unknownBuf[450];
NVIDIA_STRUCT_END

/**
* The utilization of various subsystems of the graphics card.
*
* Each system has a specific index in the pstates array, defined by the
* NVIDIA_DYNAMIC_PSTATES_SYSTEM enum.
*
*/
NVIDIA_STRUCT_BEGIN(NVIDIA_DYNAMIC_PSTATES, 1)
struct
{
/**
* Bitmap where the first bit being set determines whether this entry's value is valid.
*/
    UINT32 present;
    /**
    * The utilization of the subsystem in percent.
    */
    UINT32 value;
} pstates[8];
NVIDIA_STRUCT_END

NVIDIA_STRUCT_BEGIN(NVIDIA_GPU_POWER_POLICIES_INFO, 1)
struct
{
    UINT32 unknown[3];
    UINT32 min_power;
    UINT32 unknown2[2];
    UINT32 default_power;
    UINT32 unknown3[2];
    UINT32 max_power;
    UINT32 unknown4;
} entries[4];
NVIDIA_STRUCT_END

NVIDIA_STRUCT_BEGIN_EX(NVIDIA_GPU_POWER_POLICIES_STATUS, 1, count)
struct
{
    UINT32 flags;
    UINT32 unknown;
    UINT32 power;
    UINT32 unknown2;
} entries[4];
NVIDIA_STRUCT_END

NVIDIA_STRUCT_BEGIN(NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS, 1)
UINT32 count;
struct
{
    UINT32 voltage_domain;
    UINT32 current_voltage;
} entries[16];
NVIDIA_STRUCT_END

typedef enum
{
    NVIDIA_THERMAL_CONTROLLER_NONE,
    NVIDIA_THERMAL_CONTROLLER_GPU_INTERNAL,
    NVIDIA_THERMAL_CONTROLLER_ADM1032,
    NVIDIA_THERMAL_CONTROLLER_MAX6649,
    NVIDIA_THERMAL_CONTROLLER_MAX1617,
    NVIDIA_THERMAL_CONTROLLER_LM99,
    NVIDIA_THERMAL_CONTROLLER_LM89,
    NVIDIA_THERMAL_CONTROLLER_LM64,
    NVIDIA_THERMAL_CONTROLLER_ADT7473,
    NVIDIA_THERMAL_CONTROLLER_SBMAX6649,
    NVIDIA_THERMAL_CONTROLLER_VBIOSEVT,
    NVIDIA_THERMAL_CONTROLLER_OS,
    NVIDIA_THERMAL_CONTROLLER_UNKNOWN = -1
} NVIDIA_THERMAL_CONTROLLER;

typedef enum
{
    NVIDIA_THERMAL_TARGET_NONE = 0,
    NVIDIA_THERMAL_TARGET_GPU = 1,
    NVIDIA_THERMAL_TARGET_MEMORY = 2,
    NVIDIA_THERMAL_TARGET_POWER_SUPPLY = 4,
    NVIDIA_THERMAL_TARGET_BOARD = 8,
    NVIDIA_THERMAL_TARGET_VCD_BOARD = 9,
    NVIDIA_THERMAL_TARGET_VCD_INLET = 10,
    NVIDIA_THERMAL_TARGET_VCD_OUTLET = 11,
    NVIDIA_THERMAL_TARGET_ALL = 0xF,
    NVIDIA_THERMAL_TARGET_UNKNOWN = -1
} NVIDIA_THERMAL_TARGET;

NVIDIA_STRUCT_BEGIN_EX(NVIDIA_GPU_THERMAL_SETTINGS_V2, 2, count)
struct
{
    NVIDIA_THERMAL_CONTROLLER controller;
    INT32 default_minimum;
    INT32 default_max;
    INT32 current_temp;
    NVIDIA_THERMAL_TARGET target;
} sensor[3];
NVIDIA_STRUCT_END

#pragma endregion

#pragma region Structure printers


#pragma endregion
}