#include <cstddef>
#include <stdlib.h>
#include "miniz.h"
#include "zip.h"
#include "blob.h"

namespace fw
{
   namespace zip
   {
      mz_zip_archive write_archive;
      mz_zip_archive read_archive;
      bool begin_archive(const char* zip_file)
      {
	 memset(&write_archive, 0, sizeof(write_archive));
	 if (!mz_zip_writer_init_file(&write_archive, zip_file, 0))
	 {
	    fprintf(stderr, "Failed to initialize zip file: %s\n", zip_file);
	    return false;
	 }
	 return true;
      }
      bool add_file(const char* file_path, const char* zip_path)
      {
	 printf("adding %s -> %s\n", file_path, zip_path);
	 mz_zip_writer_add_file(&write_archive, zip_path, file_path, NULL, 0, MZ_BEST_COMPRESSION);
	 return true;
      }
      bool end_archive()
      {
	 mz_zip_writer_finalize_archive(&write_archive);
	 mz_zip_writer_end(&write_archive);
	 return true;
      }
      bool begin_load(const char* zip_file)
      {
	 memset(&read_archive, 0, sizeof(read_archive));
	 if (!mz_zip_reader_init_file(&read_archive, zip_file, 0))
	 {
	    fprintf(stderr, "Could not open ZIP archive: %s\n", zip_file);
	    return false;
	 }
	 return true;
      }
      int entry_count()
      {
	 return (int)mz_zip_reader_get_num_files(&read_archive);
      }
      // todo: allocholder is a hack, not my favourite.
      struct allocholder { blob::allocation* alloc = nullptr; };
      bool load_entry(int index, blob::bank* in_bank, blob::allocation* out_entry)
      {
	 static blob::bank* current_bank = nullptr;
	 current_bank = in_bank;
	 allocholder holder;
	 auto bank_allocate2 = [](void* holder, size_t items, size_t size)
	 {
	    blob::allocation* alloc = current_bank->allocate(size);
	    if(((allocholder*)holder)->alloc == nullptr)((allocholder*)holder)->alloc = alloc;
	    printf("[ALLOC]  %zu x %zu bytes = %zu bytes @ %p\n", items, size, items * size, (void*)alloc->data);
	    return (void*)alloc->data;
	 };
	 auto bank_free = [](void*, void* address)
	 {
	    printf("[FREE]  %p\n", address);
	    blob::allocation alloc = {{}, (char*)address, 0};
	    current_bank->free(alloc);
	 };
	 // auto bank_free = [](void*, void* address){printf("[no_free]  %p\n", address);};
	 auto bank_reallocate = [](void*, void*, size_t, size_t)
	 {
	    assert(false);
	    return (void*)nullptr;
	 };
	 read_archive.m_pAlloc = bank_allocate2;
	 read_archive.m_pAlloc_opaque = &holder;
	 read_archive.m_pFree = bank_free;
	 read_archive.m_pRealloc = bank_reallocate;
	 mz_zip_archive_file_stat stat;
	 mz_zip_reader_file_stat(&read_archive, index, &stat);
	 printf("name: %s\n", stat.m_filename);
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
	 // auto bank_allocate = [](void*, size_t, size_t) {return (void*)nullptr;};
	 // auto bank_free = [](void*, void* address){printf("[no_free]  %p\n", address);};
	 // auto bank_reallocate = [](void*, void*, size_t, size_t){return (void*)nullptr;};
	 // read_archive.m_pAlloc = bank_allocate;
	 // read_archive.m_pFree = bank_free;
	 // read_archive.m_pRealloc = bank_reallocate;
	 mz_zip_reader_end(&read_archive);
	 return true;
      }
      bool load(const char* zip_filename)
      {
	 // return read_all_files_from_zip(zip_filename);
	 return false;
      }
   }
} // namespace fw
