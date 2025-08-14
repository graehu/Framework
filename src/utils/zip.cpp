#include <cstddef>
#include <stdlib.h>
#include "miniz.h"
#include "zip.h"
#include "blob.h"
#include "log/log.h"

namespace fw
{
   namespace zip
   {
      mz_zip_archive write_archive;
      mz_zip_archive read_archive;
      bool init()
      {
	 fw::log::topics::add("zip");
	 return true;
      }
      bool begin_archive(const char* zip_file)
      {
	 log::scope topic("zip");
	 // todo: this is allocating from the heap, probably bad?
	 memset(&write_archive, 0, sizeof(write_archive));
	 if (!mz_zip_writer_init_file(&write_archive, zip_file, 0))
	 {
	    log::error("Failed to initialize zip file: {}", zip_file);
	    return false;
	 }
	 return true;
      }
      bool add_file(const char* file_path, const char* zip_path)
      {
	 log::scope topic("zip");
	 log::debug("adding {} -> {}", file_path, zip_path);
	 mz_zip_writer_add_file(&write_archive, zip_path, file_path, NULL, 0, MZ_BEST_COMPRESSION);
	 return true;
      }
      bool end_archive()
      {
	 // todo: this is allocating from the heap, probably bad?
	 mz_zip_writer_finalize_archive(&write_archive);
	 mz_zip_writer_end(&write_archive);
	 return true;
      }
      bool begin_load(const char* zip_file)
      {
	 log::scope topic("zip");
	 // note: init allocations come from the heap directly.
	 // ----: we never want these to be associated with a bank.
	 // ----: that would require realloc / buffer growth.
	 memset(&read_archive, 0, sizeof(read_archive));
	 if (!mz_zip_reader_init_file(&read_archive, zip_file, 0))
	 {
	    log::error("Could not open ZIP archive: {}", zip_file);
	    return false;
	 }
	 return true;
      }
      int entry_count()
      {
	 return (int)mz_zip_reader_get_num_files(&read_archive);
      }
      // todo: allocholder->alloc is a hack, not my favourite.
      // ----: it's taking advantage of the first allocation being the
      // ----: allocation for the resource, I'm not sure that's consistent behaviour.
      struct allocholder
      {
	 blob::bank* bank = nullptr;
	 blob::allocation* alloc = nullptr;
      };
      bool load_entry(int index, blob::bank* in_bank, blob::allocation* out_entry)
      {
	 log::scope topic("zip");
	 allocholder holder;
	 holder.bank = in_bank;
	 auto bank_allocate = [](void* opaque, size_t items, size_t size)
	 {
	    log::scope topic("zip");
	    allocholder* holder = (allocholder*)opaque;
	    blob::allocation* alloc = holder->bank->allocate(size);
	    if(holder->alloc == nullptr) holder->alloc = alloc;
	    log::debug("[ALLOC]  {} x {} bytes = {} bytes @ {}", items, size, items * size, (void*)alloc->data);
	    return (void*)alloc->data;
	 };
	 auto bank_free = [](void* opaque, void* address)
	 {
	    log::scope topic("zip");
	    allocholder* holder = (allocholder*)opaque;
	    log::debug("[FREE]  {}", address);
	    blob::allocation alloc = {{}, (char*)address, 0};
	    holder->bank->free(alloc);
	 };
	 // note: bank doesn't support reallocate.
	 auto bank_reallocate = [](void*, void*, size_t, size_t) { assert(false); return (void*)nullptr; };
	 read_archive.m_pAlloc = bank_allocate;
	 read_archive.m_pAlloc_opaque = &holder;
	 read_archive.m_pFree = bank_free;
	 read_archive.m_pRealloc = bank_reallocate;
	 mz_zip_archive_file_stat stat;
	 mz_zip_reader_file_stat(&read_archive, index, &stat);
	 log::debug("name: {}", stat.m_filename);
	 out_entry->data = (const char*)mz_zip_reader_extract_to_heap(&read_archive, index, &out_entry->len, 0);
	 if (out_entry->data != nullptr)
	 {
	    // note: I'm writing the header twice in my data, we should not do that!
	    // ----: lies, we can have assets inside assets.
	    assert(((blob::header*)out_entry->data)->hash != 0);
	    out_entry->head = *((blob::header*)out_entry->data);
	    holder.alloc->head = *((blob::header*)out_entry->data);
	    assert(out_entry->head.hash != 0);
	    out_entry->data = (char*)((blob::header*)out_entry->data+1);
	 }
	 return out_entry->data != nullptr;
      }
      bool end_load()
      {
	 // note: these allocations come from the heap directly.
	 // ----: we never want these to be associated with a bank.
	 // ----: that would require realloc / buffer growth.
	 read_archive.m_pAlloc = nullptr;
	 read_archive.m_pFree = nullptr;
	 read_archive.m_pRealloc = nullptr;
	 mz_zip_reader_end(&read_archive);
	 return true;
      }
   }
} // namespace fw
