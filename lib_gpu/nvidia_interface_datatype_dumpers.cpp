#include "pch.h"
#include "nvidia_interface_datatype_dumpers.h"

namespace lib_gpu {

std::ostream& print_hex(std::ostream &strm, UINT32 num)
{
    auto flags = strm.flags();
    strm << std::hex << std::uppercase << "0x" << std::setw(8) << std::setfill('0') << num;
    strm.flags(flags);
    return strm;
}

std::ostream& print_array(std::ostream &strm, const UINT32* array, size_t length)
{
    strm << "[";
    for (auto i = 0u; i < length; i++) {
        print_hex(strm, array[i]);
        if (i < (length - 1)) {
            strm << ", ";
        }
    }
    strm << "]";
    return strm;
}

inline std::string _indent(unsigned level)
{
    return std::string(level * 2, ' ');
}

struct indented_newline
{
    unsigned level = 0;
};

std::ostream& operator<<(std::ostream &strm, const indented_newline& i)
{
    return strm << std::endl << std::string(i.level * 2, ' ');
}

template <typename F>
void new_scope(std::ostream& strm, indented_newline& iendl, std::string name, F scope_printer)
{
    iendl.level++;
    strm << name << " { " << iendl;;
    scope_printer();
    iendl.level--;
    strm << iendl << "}" << iendl;
}

template <typename F>
void new_scope(std::ostream& strm, indented_newline& iendl, std::string name, unsigned num, F scope_printer)
{

    auto scope_name = std::stringstream{};
    scope_name << name << "[" << num << "]";
    new_scope(strm, iendl, scope_name.str(), scope_printer);
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_DELTA_ENTRY &d)
{
    auto flags = strm.flags();
    strm << std::dec << "DELTA(" << d.val_min << " <= " << d.value << " <= " << d.val_max << ")";
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_PSTATES20_V2 &p)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_GPU_PSTATES20_V2", [&]() {
        strm << "flags: ";
        print_hex(strm, p.flags) << iendl;
        strm << "state_count: " << p.state_count << iendl
            << "clock_count: " << p.clock_count << iendl
            << "voltage_count: " << p.voltage_count << iendl;

        for (auto i = 0u; i < p.state_count; i++) {
            const auto& state = p.states[i];
            new_scope(strm, iendl, "State", i, [&]() {
                strm << "state_num: " << state.state_num << iendl;
                strm << "flags: ";
                print_hex(strm, state.flags) << iendl;

                for (auto j = 0u; j < p.clock_count; j++) {
                    const auto& clock = state.clocks[j];
                    new_scope(strm, iendl, "Clock", j, [&]() {
                        strm << "domain: " << clock.domain << iendl;
                        strm << "type: " << clock.type << iendl;
                        strm << "flags: ";
                        print_hex(strm, clock.flags) << iendl;
                        strm << "freq_delta: " << clock.freq_delta << iendl;
                        strm << "min_or_single_freq: " << clock.min_or_single_freq << iendl;
                        strm << "max_freq: " << clock.max_freq << iendl;
                        strm << "voltage_domain: " << clock.voltage_domain << iendl;
                        strm << "min_volt: " << clock.min_volt << iendl;
                        strm << "max_volt: " << clock.max_volt;
                    });
                }

                for (auto j = 0u; j < p.voltage_count; j++) {
                    const auto& voltage = state.base_voltages[j];
                    new_scope(strm, iendl, "Voltage", j, [&]() {
                        strm << "domain: " << voltage.domain << iendl;
                        strm << "flags: ";
                        print_hex(strm, voltage.flags) << iendl;
                        strm << "voltage: " << voltage.voltage << iendl;
                        strm << "volt_delta: " << voltage.volt_delta;
                    });
                }
            });
        }

        for (auto i = 0u; i < p.over_volt.voltage_count; i++) {
            const auto& voltage = p.over_volt.voltages[i];
            new_scope(strm, iendl, "Overvolt", i, [&]() {
                strm << "domain: " << voltage.domain << iendl;
                strm << "flags: ";
                print_hex(strm, voltage.flags) << iendl;
                strm << "voltage: " << voltage.voltage << iendl;
                strm << "volt_delta: " << voltage.volt_delta;
            });
        }
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_CLOCK_FREQUENCIES &f)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_CLOCK_FREQUENCIES", [&]() {
        strm << "clock_type: ";
        print_hex(strm, f.clock_type) << iendl;

        for (auto i = 0u; i < 32; i++) {
            new_scope(strm, iendl, "Frequency", i, [&]() {
                strm << "present: ";
                print_hex(strm, f.entries[i].present) << iendl;
                strm << "freq: " << f.entries[i].freq;
            });
        }
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_PERF_TABLE &p)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_GPU_PERF_TABLE", [&]() {
        strm << "plevel_count: " << p.plevel_count << iendl;
        strm << "field1: " << p.field1 << iendl;
        strm << "domain_entries: " << p.domain_entries << iendl;
        strm << "field3: " << p.field3 << iendl;
        strm << "p_state_level: " << p.p_state_level << iendl;
        strm << "field5: " << p.field5 << iendl;

        for (auto i = 0u; i < 10; i++) {
            const auto& entry = p.entries[i];
            new_scope(strm, iendl, "Entry", i, [&]() {
                for (auto j = 0u; j < 32; j++) {
                    const auto& domain = entry.domains[j];
                    new_scope(strm, iendl, "Domain", j, [&]() {
                        strm << "domain: " << domain.domain << iendl;
                        strm << "unknown1: " << domain.unknown1 << iendl;
                        strm << "clock: " << domain.clock << iendl;
                        strm << "defaultClock: " << domain.defaultClock << iendl;
                        strm << "minClock: " << domain.minClock << iendl;
                        strm << "maxClock: " << domain.maxClock << iendl;
                        strm << "unknown2: " << domain.unknown2;
                    });
                }
                strm << "unknown1: " << entry.unknown1 << iendl;
                strm << "possiblySettingFlags: ";
                print_hex(strm, entry.possiblySettingFlags);
            });
        }
        print_array(strm, p.unknownBuf, 450);
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_DYNAMIC_PSTATES &p)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_DYNAMIC_PSTATES", [&]() {
        strm << "flags: ";
        print_hex(strm, p.flags) << iendl;

        for (auto i = 0u; i < 8; i++) {
            new_scope(strm, iendl, "State", i, [&]() {
                strm << "present: ";
                print_hex(strm, p.pstates[i].present) << iendl;
                strm << "value: " << p.pstates[i].value;
            });
        }
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_POWER_POLICIES_INFO &p)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_GPU_POWER_POLICIES_INFO", [&]() {
        strm << "flags: ";
        print_hex(strm, p.flags) << iendl;

        for (auto i = 0u; i < 4; i++) {
            const auto& entry = p.entries[i];
            new_scope(strm, iendl, "Entry", i, [&]() {
                strm << "Unknown 1: ";
                print_array(strm, entry.unknown, 3) << iendl;
                strm << "min_power: " << entry.min_power << iendl;
                strm << "Unknown 2: ";
                print_array(strm, entry.unknown2, 2) << iendl;
                strm << "default_power: " << entry.default_power << iendl;
                strm << "Unknown 3: ";
                print_array(strm, entry.unknown3, 2) << iendl;
                strm << "max_power: " << entry.max_power << iendl;
                strm << "Unknown 4: ";
                print_hex(strm, entry.unknown4);
            });
        }
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_POWER_POLICIES_STATUS &p)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_GPU_POWER_POLICIES_STATUS", [&]() {
        strm << "count: ";
        print_hex(strm, p.count) << iendl;

        for (auto i = 0u; i < 4; i++) {
            const auto& entry = p.entries[i];
            new_scope(strm, iendl, "Entry", i, [&]() {
                strm << "flags: ";
                print_hex(strm, entry.flags) << iendl;
                strm << "unknown: ";
                print_hex(strm, entry.unknown) << iendl;
                strm << "power: ";
                print_hex(strm, entry.power) << iendl;
                strm << "unknown2: ";
                print_hex(strm, entry.unknown2);
            });
        }
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS &v)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS", [&]() {
        strm << "flags: ";
        print_hex(strm, v.flags) << iendl;
        strm << "count: " << v.count << iendl;

        for (auto i = 0u; i < v.count; i++) {
            const auto& entry = v.entries[i];
            new_scope(strm, iendl, "Entry", i, [&]() {
                strm << "voltage_domain" << entry.voltage_domain << iendl;
                strm << "current_voltage" << entry.current_voltage;
            });
        }
    });
    strm.flags(flags);
    return strm;
}

std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_THERMAL_SETTINGS_V2 &t)
{
    auto flags = strm.flags();
    auto iendl = indented_newline{};
    new_scope(strm, iendl, "NVIDIA_GPU_THERMAL_SETTINGS_V2", [&]() {
        strm << "count: " << t.count << iendl;

        for (auto i = 0u; i < t.count; i++) {
            const auto& sensor = t.sensor[i];
            new_scope(strm, iendl, "Sensor", i, [&]() {
                strm << "controller: " << sensor.controller << iendl;
                strm << "default_minimum: " << sensor.default_minimum << iendl;
                strm << "default_max: " << sensor.default_max << iendl;
                strm << "current_temp: " << sensor.current_temp << iendl;
                strm << "target: " << sensor.target;
            });
        }
    });
    strm.flags(flags);
    return strm;
}

}
