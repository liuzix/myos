//
// Created by Zixiong Liu on 12/18/16.
//

#ifndef MYOS_PAGING_H
#define MYOS_PAGING_H

#include <cstdint>

struct page_table_entry {
    bool present : 1;
    bool writable : 1;
    bool user : 1;
    bool write_through : 1;
    bool cache_disabled : 1;
    bool accessed : 1;
    bool dirty : 1;
    bool huge_page : 1;
    bool global : 1;
    int software : 3;
    uint64_t address : 40;
    int reserved : 12;
} __attribute__((packed));

struct page_table {
    page_table_entry *entries;
    int level;
    page_table (int level, page_table_entry* addr);
    page_table_entry *get_pte(uint8_t *vaddr, bool create);
    page_table_entry *map(uint8_t *vaddr, uint8_t *paddr);
    inline bool is_valid (uint8_t *vaddr) {
      auto p = get_pte(vaddr, false);
      return p && p->present;
    }

    page_table_entry *map_no_cache(uint8_t *vaddr, uint8_t *paddr);
};

extern page_table_entry p4_table[64];

extern  page_table pages;

inline void page_init () {
  pages.entries = p4_table;
  pages.level = 4;
}


#endif //MYOS_PAGING_H
