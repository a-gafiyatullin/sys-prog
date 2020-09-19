/*
 * Paging in x86 Long Mode:
 *
 * Table record fields:
 * bit number | bit type
 * ---------------------------------------
 *     0     | P  (set if record is using)
 *  12 - 51  | Physical Address
 *
 * Logical address structure:
 * bit number | bit type
 * ---------------------------------------
 *  0 - 11   | offset
 *  12 - 20  | LVL4 table
 *  21 - 29  | LVL3 table
 *  30 - 38  | LVL2 table
 *  39 - 47  | LVL1 table
 */

#include <fstream>
#include <iostream>
#include <map>
using namespace std;

#define P(address) (address & 0x1)

#define TABLE_INDEX(address, LVL) ((address << (0x10 + (0x9 * LVL))) >> 0x37)
#define OFFSET(address) (address & 0xfff)

constexpr int MAX_TABLE_LVL = 4;

/* return 0 and physical_address as the second argument or -1 if page fault */
int translate_logical_to_physical_address(const uint64_t &logical_address, uint64_t &physical_address,
                                          const uint64_t &first_lvl_table_address, const map<uint64_t, uint64_t> &memory_map);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "usage " + std::string(argv[0]) + " <filename>" << endl;
        return -1;
    }

    ifstream input(argv[1]);

    map<uint64_t, uint64_t> memory_map;
    uint64_t m = 0, q = 0, r = 0;
    input >> m >> q >> r;

    uint64_t paddr = 0, value = 0;
    for (uint64_t i = 0; i < m; i++) {
        input >> paddr >> value;
        memory_map[paddr] = value;
    }

    uint64_t logical_address = 0, physical_address = 0;
    for (uint64_t i = 0; i < q; i++) {
        input >> logical_address;
        if (!translate_logical_to_physical_address(logical_address, physical_address, r, memory_map)) {
            cout << physical_address << endl;
        } else {
            cout << "fault" << endl;
        }
    }

    return 0;
}

int translate_logical_to_physical_address(const uint64_t &logical_address, uint64_t &physical_address,
                                          const uint64_t &first_lvl_table_address, const map<uint64_t, uint64_t> &memory_map) {
    int address_lvl_index = 0;
    uint64_t table_address = first_lvl_table_address;

    while (address_lvl_index < MAX_TABLE_LVL) {
        uint64_t index = TABLE_INDEX(logical_address, address_lvl_index);
        uint64_t table_record_address = table_address + sizeof(uint64_t) * index;

        auto record_iterator = memory_map.find(table_record_address);
        uint64_t table_record = (record_iterator == memory_map.end() ? 0 : record_iterator->second);
        if (!P(table_record)) {
            return -1;
        }
        table_address = table_record - 1;
        address_lvl_index++;
    }
    table_address += OFFSET(logical_address);
    physical_address = table_address;

    return 0;
}
